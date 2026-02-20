// Copyright SatisfactoryDiscordbot. All Rights Reserved.

#include "Networking/WSClientConnection.h"

#include "Sockets.h"
#include "SocketSubsystem.h"
#include "Misc/SecureHash.h"   // FSHA1
#include "Misc/Base64.h"       // FBase64

DEFINE_LOG_CATEGORY_STATIC(LogWSClientConnection, Log, All);

// ---------------------------------------------------------------------------
// Construction / Destruction
// ---------------------------------------------------------------------------

FWSClientConnection::FWSClientConnection(
	FSocket*          InSocket,
	ISocketSubsystem* InSocketSubsystem,
	const FString&    InRemoteAddress)
	: Socket(InSocket)
	, SocketSubsystem(InSocketSubsystem)
	, RemoteAddress(InRemoteAddress)
{
}

FWSClientConnection::~FWSClientConnection()
{
	if (Socket)
	{
		Socket->Close();
		SocketSubsystem->DestroySocket(Socket);
		Socket = nullptr;
	}
}

// ---------------------------------------------------------------------------
// Handshake
// ---------------------------------------------------------------------------

bool FWSClientConnection::PerformHandshake()
{
	FString ClientKey;
	if (!ReadHandshakeRequest(ClientKey))
	{
		UE_LOG(LogWSClientConnection, Warning,
			TEXT("[%s] Failed to read WebSocket handshake request"), *RemoteAddress);
		return false;
	}

	const FString AcceptKey = ComputeAcceptKey(ClientKey);
	if (!SendHandshakeResponse(AcceptKey))
	{
		UE_LOG(LogWSClientConnection, Warning,
			TEXT("[%s] Failed to send WebSocket handshake response"), *RemoteAddress);
		return false;
	}

	bConnected.Store(true);
	UE_LOG(LogWSClientConnection, Log,
		TEXT("[%s] WebSocket handshake complete"), *RemoteAddress);
	return true;
}

bool FWSClientConnection::ReadHandshakeRequest(FString& OutKey)
{
	// Read raw bytes until we see the end-of-headers marker (\r\n\r\n).
	// The socket is in its default (blocking) state here; we use Wait with a
	// timeout to avoid hanging forever if the client is misbehaving.

	TArray<uint8> RequestData;
	constexpr int32 MaxRequestBytes = 8192;
	constexpr double TimeoutSecs    = 5.0;

	const double StartTime = FPlatformTime::Seconds();
	uint8 Buf[512];

	bool bFoundEnd = false;
	while (!bFoundEnd && RequestData.Num() < MaxRequestBytes)
	{
		if (FPlatformTime::Seconds() - StartTime > TimeoutSecs)
		{
			UE_LOG(LogWSClientConnection, Warning,
				TEXT("[%s] Handshake timed out after %.0f s"), *RemoteAddress, TimeoutSecs);
			return false;
		}

		// Wait up to 100 ms for readable data.
		if (!Socket->Wait(ESocketWaitConditions::WaitForRead, FTimespan::FromMilliseconds(100)))
		{
			continue;
		}

		int32 BytesRead = 0;
		if (!Socket->Recv(Buf, sizeof(Buf), BytesRead, ESocketReceiveFlags::None) || BytesRead <= 0)
		{
			return false;
		}

		RequestData.Append(Buf, BytesRead);

		// Look for \r\n\r\n starting from the new bytes (but include one overlap).
		const int32 SearchFrom = FMath::Max(0, RequestData.Num() - BytesRead - 3);
		for (int32 i = SearchFrom; i <= RequestData.Num() - 4; ++i)
		{
			if (RequestData[i]   == '\r' && RequestData[i+1] == '\n' &&
			    RequestData[i+2] == '\r' && RequestData[i+3] == '\n')
			{
				bFoundEnd = true;
				break;
			}
		}
	}

	if (!bFoundEnd)
	{
		UE_LOG(LogWSClientConnection, Warning,
			TEXT("[%s] Did not find end of HTTP headers"), *RemoteAddress);
		return false;
	}

	// Parse the Sec-WebSocket-Key header from the raw request bytes.
	// The bytes are ASCII/UTF-8 so we can treat them as a plain string.
	RequestData.Add('\0'); // null-terminate for conversion
	const FString Request = FString(UTF8_TO_TCHAR(
		reinterpret_cast<const char*>(RequestData.GetData())));

	TArray<FString> Lines;
	Request.ParseIntoArrayLines(Lines);

	static const FString KeyPrefix = TEXT("Sec-WebSocket-Key:");
	for (const FString& Line : Lines)
	{
		if (Line.StartsWith(KeyPrefix, ESearchCase::IgnoreCase))
		{
			OutKey = Line.Mid(KeyPrefix.Len()).TrimStartAndEnd();
			if (!OutKey.IsEmpty())
			{
				return true;
			}
		}
	}

	UE_LOG(LogWSClientConnection, Warning,
		TEXT("[%s] Sec-WebSocket-Key header not found"), *RemoteAddress);
	return false;
}

FString FWSClientConnection::ComputeAcceptKey(const FString& WebSocketKey)
{
	// RFC 6455 §4.2.2: Sec-WebSocket-Accept = Base64( SHA-1( key + magic ) )
	static const FString Magic = TEXT("258EAFA5-E914-47DA-95CA-C5AB0DC85B11");
	const FString Combined = WebSocketKey + Magic;

	// Convert to UTF-8 bytes for hashing.
	FTCHARToUTF8 Converter(*Combined);

	uint8 HashBytes[20];
	FSHA1::HashBuffer(Converter.Get(), static_cast<uint64>(Converter.Length()), HashBytes);

	TArray<uint8> HashArray;
	HashArray.Append(HashBytes, 20);
	return FBase64::Encode(HashArray);
}

bool FWSClientConnection::SendHandshakeResponse(const FString& AcceptKey)
{
	const FString Response = FString::Printf(
		TEXT("HTTP/1.1 101 Switching Protocols\r\n"
		     "Upgrade: websocket\r\n"
		     "Connection: Upgrade\r\n"
		     "Sec-WebSocket-Accept: %s\r\n"
		     "\r\n"),
		*AcceptKey);

	FTCHARToUTF8 Converter(*Response);
	return SendRawBlocking(
		reinterpret_cast<const uint8*>(Converter.Get()),
		Converter.Length());
}

// ---------------------------------------------------------------------------
// Reading (server-thread only)
// ---------------------------------------------------------------------------

void FWSClientConnection::ReadPendingData()
{
	if (!bConnected.Load())
	{
		return;
	}

	// Non-blocking: check for available data using HasPendingData (ioctl FIONREAD).
	uint8 Buf[4096];
	uint32 PendingBytes = 0;

	while (Socket->HasPendingData(PendingBytes) && PendingBytes > 0)
	{
		const int32 ReadSize = static_cast<int32>(
			FMath::Min(PendingBytes, static_cast<uint32>(sizeof(Buf))));

		int32 BytesRead = 0;
		if (!Socket->Recv(Buf, ReadSize, BytesRead, ESocketReceiveFlags::None)
		    || BytesRead <= 0)
		{
			// TCP connection dropped.
			bConnected.Store(false);

			FWSMessage CloseMsg;
			CloseMsg.bIsClosed      = true;
			CloseMsg.CloseStatusCode = 1006; // Abnormal closure
			CloseMsg.CloseReason    = TEXT("Connection lost");
			IncomingMessageQueue.Enqueue(MoveTemp(CloseMsg));
			return;
		}

		ReceiveBuffer.Append(Buf, BytesRead);
	}

	// Try to parse as many complete frames as possible.
	while (TryParseFrame()) {}
}

// ---------------------------------------------------------------------------
// Frame parsing (server-thread only)
// ---------------------------------------------------------------------------

bool FWSClientConnection::TryParseFrame()
{
	// Minimum frame header is 2 bytes.
	if (ReceiveBuffer.Num() < 2)
	{
		return false;
	}

	const uint8 Byte0 = ReceiveBuffer[0];
	const uint8 Byte1 = ReceiveBuffer[1];

	const bool  bFinalFrame = (Byte0 & 0x80) != 0;
	const uint8 Opcode      = Byte0 & 0x0F;
	const bool  bMasked     = (Byte1 & 0x80) != 0;
	uint64      PayloadLen  = Byte1 & 0x7F;

	int32 HeaderSize = 2;

	// Extended payload length – 16-bit form.
	if (PayloadLen == 126)
	{
		if (ReceiveBuffer.Num() < 4) return false;
		PayloadLen  = (static_cast<uint64>(ReceiveBuffer[2]) << 8)
		            |  static_cast<uint64>(ReceiveBuffer[3]);
		HeaderSize += 2;
	}
	// Extended payload length – 64-bit form.
	else if (PayloadLen == 127)
	{
		if (ReceiveBuffer.Num() < 10) return false;
		PayloadLen = 0;
		for (int32 i = 0; i < 8; ++i)
		{
			PayloadLen = (PayloadLen << 8) | ReceiveBuffer[2 + i];
		}
		HeaderSize += 8;
	}

	// Masking key (clients MUST mask; RFC 6455 §5.3).
	uint8 MaskingKey[4] = {};
	if (bMasked)
	{
		if (ReceiveBuffer.Num() < HeaderSize + 4) return false;
		for (int32 i = 0; i < 4; ++i)
		{
			MaskingKey[i] = ReceiveBuffer[HeaderSize + i];
		}
		HeaderSize += 4;
	}

	// Reject unreasonably large frames (10 MiB).
	constexpr uint64 MaxPayload = 10u * 1024u * 1024u;
	if (PayloadLen > MaxPayload)
	{
		UE_LOG(LogWSClientConnection, Warning,
			TEXT("[%s] Oversized WebSocket frame (%llu bytes); closing"), *RemoteAddress, PayloadLen);
		bConnected.Store(false);
		return false;
	}

	const int32 TotalFrame = HeaderSize + static_cast<int32>(PayloadLen);
	if (ReceiveBuffer.Num() < TotalFrame)
	{
		return false; // Wait for more data.
	}

	// Extract and unmask payload.
	TArray<uint8> Payload;
	if (PayloadLen > 0)
	{
		Payload.SetNumUninitialized(static_cast<int32>(PayloadLen));
		FMemory::Memcpy(
			Payload.GetData(),
			ReceiveBuffer.GetData() + HeaderSize,
			static_cast<int32>(PayloadLen));

		if (bMasked)
		{
			for (int32 i = 0; i < Payload.Num(); ++i)
			{
				Payload[i] ^= MaskingKey[i & 3];
			}
		}
	}

	// Remove the consumed bytes from the front of the buffer.
	ReceiveBuffer.RemoveAt(0, TotalFrame, /*bAllowShrinking=*/false);

	ProcessFrame(Opcode, bFinalFrame, Payload);
	return true;
}

void FWSClientConnection::ProcessFrame(
	uint8              Opcode,
	bool               bFinalFrame,
	const TArray<uint8>& Payload)
{
	switch (Opcode)
	{
		// --- Continuation frame ---
		case 0x0:
		{
			if (!bInFragment)
			{
				UE_LOG(LogWSClientConnection, Warning,
					TEXT("[%s] Unexpected continuation frame; ignoring"), *RemoteAddress);
				return;
			}
			FragmentBuffer.Append(Payload);
			if (bFinalFrame)
			{
				bInFragment = false;
				// Deliver the assembled message.
				FWSMessage Msg;
				if (FragmentOpcode == 0x1)
				{
					Msg.bIsText = true;
					TArray<uint8> Terminated = FragmentBuffer;
					Terminated.Add('\0');
					Msg.TextData = FString(UTF8_TO_TCHAR(
						reinterpret_cast<const char*>(Terminated.GetData())));
				}
				else
				{
					Msg.bIsText    = false;
					Msg.BinaryData = FragmentBuffer;
				}
				IncomingMessageQueue.Enqueue(MoveTemp(Msg));
				FragmentBuffer.Empty();
				FragmentOpcode = 0;
			}
			break;
		}

		// --- Text frame ---
		case 0x1:
		{
			if (bFinalFrame)
			{
				FWSMessage Msg;
				Msg.bIsText = true;
				TArray<uint8> Terminated = Payload;
				Terminated.Add('\0');
				Msg.TextData = FString(UTF8_TO_TCHAR(
					reinterpret_cast<const char*>(Terminated.GetData())));
				IncomingMessageQueue.Enqueue(MoveTemp(Msg));
			}
			else
			{
				bInFragment    = true;
				FragmentOpcode = 0x1;
				FragmentBuffer = Payload;
			}
			break;
		}

		// --- Binary frame ---
		case 0x2:
		{
			if (bFinalFrame)
			{
				FWSMessage Msg;
				Msg.bIsText    = false;
				Msg.BinaryData = Payload;
				IncomingMessageQueue.Enqueue(MoveTemp(Msg));
			}
			else
			{
				bInFragment    = true;
				FragmentOpcode = 0x2;
				FragmentBuffer = Payload;
			}
			break;
		}

		// --- Close frame ---
		case 0x8:
		{
			uint16  StatusCode = 1000;
			FString Reason;

			if (Payload.Num() >= 2)
			{
				StatusCode = (static_cast<uint16>(Payload[0]) << 8)
				           |  static_cast<uint16>(Payload[1]);
				if (Payload.Num() > 2)
				{
					TArray<uint8> ReasonBytes(Payload.GetData() + 2, Payload.Num() - 2);
					ReasonBytes.Add('\0');
					Reason = FString(UTF8_TO_TCHAR(
						reinterpret_cast<const char*>(ReasonBytes.GetData())));
				}
			}

			// Echo the close frame then mark the connection closed.
			{
				FScopeLock Lock(&SendMutex);
				SendFrame(0x8, Payload.GetData(), Payload.Num());
			}
			bConnected.Store(false);

			FWSMessage CloseMsg;
			CloseMsg.bIsClosed       = true;
			CloseMsg.CloseStatusCode = StatusCode;
			CloseMsg.CloseReason     = Reason;
			IncomingMessageQueue.Enqueue(MoveTemp(CloseMsg));
			break;
		}

		// --- Ping frame ---
		case 0x9:
		{
			// RFC 6455 §5.5.3: respond with Pong carrying the same payload.
			FScopeLock Lock(&SendMutex);
			SendFrame(0xA, Payload.GetData(), Payload.Num());
			break;
		}

		// --- Pong frame ---
		case 0xA:
			// Unsolicited pong; nothing to do.
			break;

		default:
			UE_LOG(LogWSClientConnection, Warning,
				TEXT("[%s] Unknown WebSocket opcode 0x%X; ignoring"), *RemoteAddress, Opcode);
			break;
	}
}

// ---------------------------------------------------------------------------
// Low-level write helpers
// ---------------------------------------------------------------------------

bool FWSClientConnection::SendFrame(uint8 Opcode, const uint8* Payload, int32 PayloadLen)
{
	// RFC 6455 §5.1: server frames are NEVER masked.
	TArray<uint8> Frame;
	Frame.Reserve(PayloadLen + 10);

	// Byte 0: FIN=1, RSV=0, Opcode.
	Frame.Add(static_cast<uint8>(0x80 | (Opcode & 0x0F)));

	// Byte 1 (+ extended length): MASK=0.
	if (PayloadLen < 126)
	{
		Frame.Add(static_cast<uint8>(PayloadLen));
	}
	else if (PayloadLen < 65536)
	{
		Frame.Add(126);
		Frame.Add(static_cast<uint8>((PayloadLen >> 8) & 0xFF));
		Frame.Add(static_cast<uint8>( PayloadLen       & 0xFF));
	}
	else
	{
		Frame.Add(127);
		const uint64 PayloadLen64 = static_cast<uint64>(PayloadLen);
		for (int32 Shift = 56; Shift >= 0; Shift -= 8)
		{
			Frame.Add(static_cast<uint8>((PayloadLen64 >> Shift) & 0xFF));
		}
	}

	if (Payload && PayloadLen > 0)
	{
		Frame.Append(Payload, PayloadLen);
	}

	return SendRawBlocking(Frame.GetData(), Frame.Num());
}

bool FWSClientConnection::SendRawBlocking(const uint8* Data, int32 DataLen)
{
	int32 TotalSent = 0;
	while (TotalSent < DataLen)
	{
		int32 BytesSent = 0;
		if (!Socket->Send(Data + TotalSent, DataLen - TotalSent, BytesSent)
		    || BytesSent <= 0)
		{
			return false;
		}
		TotalSent += BytesSent;
	}
	return true;
}

// ---------------------------------------------------------------------------
// Thread-safe public write API
// ---------------------------------------------------------------------------

bool FWSClientConnection::SendText(const FString& Message)
{
	if (!bConnected.Load()) return false;

	FTCHARToUTF8 Converter(*Message);
	FScopeLock Lock(&SendMutex);
	return SendFrame(
		0x1,
		reinterpret_cast<const uint8*>(Converter.Get()),
		Converter.Length());
}

bool FWSClientConnection::SendBinary(const TArray<uint8>& Data)
{
	if (!bConnected.Load()) return false;

	FScopeLock Lock(&SendMutex);
	return SendFrame(0x2, Data.GetData(), Data.Num());
}

bool FWSClientConnection::SendClose(uint16 StatusCode, const FString& Reason)
{
	if (!bConnected.Load()) return false;

	TArray<uint8> Payload;
	Payload.Add(static_cast<uint8>((StatusCode >> 8) & 0xFF));
	Payload.Add(static_cast<uint8>( StatusCode       & 0xFF));

	if (!Reason.IsEmpty())
	{
		FTCHARToUTF8 Converter(*Reason);
		Payload.Append(
			reinterpret_cast<const uint8*>(Converter.Get()),
			Converter.Length());
	}

	FScopeLock Lock(&SendMutex);
	const bool bOk = SendFrame(0x8, Payload.GetData(), Payload.Num());
	bConnected.Store(false);
	return bOk;
}

// ---------------------------------------------------------------------------
// Accessors
// ---------------------------------------------------------------------------

bool FWSClientConnection::IsConnected() const
{
	return bConnected.Load();
}

bool FWSClientConnection::HasPendingMessages() const
{
	return !IncomingMessageQueue.IsEmpty();
}

bool FWSClientConnection::TryGetNextMessage(FWSMessage& OutMessage)
{
	return IncomingMessageQueue.Dequeue(OutMessage);
}
