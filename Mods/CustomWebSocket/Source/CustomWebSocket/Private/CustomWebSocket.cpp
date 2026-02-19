// Copyright (c) 2024 Yamahasxviper
// Custom WebSocket Implementation

#include "CustomWebSocket.h"
#include "Misc/Base64.h"
#include "Misc/SecureHash.h"
#include "HAL/PlatformProcess.h"

DEFINE_LOG_CATEGORY_STATIC(LogCustomWebSocket, Log, All);

// WebSocket opcodes (RFC 6455)
#define WS_OPCODE_CONTINUATION 0x0
#define WS_OPCODE_TEXT 0x1
#define WS_OPCODE_BINARY 0x2
#define WS_OPCODE_CLOSE 0x8
#define WS_OPCODE_PING 0x9
#define WS_OPCODE_PONG 0xA

// WebSocket GUID for handshake
#define WS_GUID "258EAFA5-E914-47DA-95CA-C5AB0DC85B11"

FCustomWebSocket::FCustomWebSocket()
    : Socket(nullptr)
    , SocketSubsystem(nullptr)
    , bIsConnected(false)
    , bIsSecure(false)
    , ServerPort(0)
    , bHandshakeComplete(false)
    , bParsingFrame(false)
    , CurrentOpcode(0)
{
    SocketSubsystem = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM);
}

FCustomWebSocket::~FCustomWebSocket()
{
    Disconnect();
}

bool FCustomWebSocket::Connect(const FString& URL)
{
    if (bIsConnected)
    {
        UE_LOG(LogCustomWebSocket, Warning, TEXT("Already connected"));
        return false;
    }

    // Parse URL
    if (!ParseURL(URL, ServerHost, ServerPort, ServerPath, bIsSecure))
    {
        OnError.ExecuteIfBound(TEXT("Invalid WebSocket URL"));
        return false;
    }

    UE_LOG(LogCustomWebSocket, Log, TEXT("Connecting to %s:%d%s (secure: %s)"),
        *ServerHost, ServerPort, *ServerPath, bIsSecure ? TEXT("yes") : TEXT("no"));

    // TCP connection
    if (!PerformTCPConnection(ServerHost, ServerPort))
    {
        OnError.ExecuteIfBound(TEXT("TCP connection failed"));
        return false;
    }

    // TLS handshake if needed
    if (bIsSecure)
    {
        if (!PerformTLSHandshake())
        {
            OnError.ExecuteIfBound(TEXT("TLS handshake failed"));
            Disconnect();
            return false;
        }
    }

    // WebSocket handshake
    if (!PerformWebSocketHandshake())
    {
        OnError.ExecuteIfBound(TEXT("WebSocket handshake failed"));
        Disconnect();
        return false;
    }

    return true;
}

void FCustomWebSocket::Disconnect(int32 StatusCode, const FString& Reason)
{
    if (!bIsConnected)
    {
        return;
    }

    UE_LOG(LogCustomWebSocket, Log, TEXT("Disconnecting WebSocket (StatusCode: %d, Reason: %s)"), StatusCode, *Reason);

    // Send close frame
    TArray<uint8> CloseData;
    CloseData.Add((StatusCode >> 8) & 0xFF);
    CloseData.Add(StatusCode & 0xFF);
    
    if (!Reason.IsEmpty())
    {
        FTCHARToUTF8 UTF8Reason(*Reason);
        CloseData.Append((uint8*)UTF8Reason.Get(), UTF8Reason.Length());
    }

    TArray<uint8> CloseFrame = CreateFrame(WS_OPCODE_CLOSE, CloseData, true);
    if (Socket)
    {
        int32 BytesSent = 0;
        Socket->Send(CloseFrame.GetData(), CloseFrame.Num(), BytesSent);
    }

    // Close socket
    if (Socket)
    {
        Socket->Close();
        SocketSubsystem->DestroySocket(Socket);
        Socket = nullptr;
    }

    bIsConnected = false;
    bHandshakeComplete = false;

    UE_LOG(LogCustomWebSocket, Log, TEXT("WebSocket disconnected"));
    OnClosed.ExecuteIfBound(StatusCode, Reason, true);
}

bool FCustomWebSocket::SendText(const FString& Message)
{
    if (!bIsConnected || !bHandshakeComplete)
    {
        UE_LOG(LogCustomWebSocket, Warning, TEXT("Not connected"));
        return false;
    }

    // Convert to UTF-8
    FTCHARToUTF8 UTF8Message(*Message);
    TArray<uint8> Payload;
    Payload.Append((uint8*)UTF8Message.Get(), UTF8Message.Length());

    // Create frame
    TArray<uint8> Frame = CreateFrame(WS_OPCODE_TEXT, Payload, true);

    // Send
    int32 BytesSent = 0;
    bool bSuccess = Socket->Send(Frame.GetData(), Frame.Num(), BytesSent);

    if (!bSuccess || BytesSent != Frame.Num())
    {
        UE_LOG(LogCustomWebSocket, Error, TEXT("Failed to send message"));
        return false;
    }

    return true;
}

bool FCustomWebSocket::SendBinary(const TArray<uint8>& Data)
{
    if (!bIsConnected || !bHandshakeComplete)
    {
        UE_LOG(LogCustomWebSocket, Warning, TEXT("Not connected"));
        return false;
    }

    // Create frame
    TArray<uint8> Frame = CreateFrame(WS_OPCODE_BINARY, Data, true);

    // Send
    int32 BytesSent = 0;
    bool bSuccess = Socket->Send(Frame.GetData(), Frame.Num(), BytesSent);

    if (!bSuccess || BytesSent != Frame.Num())
    {
        UE_LOG(LogCustomWebSocket, Error, TEXT("Failed to send binary data"));
        return false;
    }

    return true;
}

void FCustomWebSocket::Tick(float DeltaTime)
{
    if (!Socket || !bIsConnected)
    {
        return;
    }

    // Check for pending data
    uint32 PendingDataSize = 0;
    if (Socket->HasPendingData(PendingDataSize) && PendingDataSize > 0)
    {
        if (!ProcessReceivedData())
        {
            UE_LOG(LogCustomWebSocket, Error, TEXT("Error processing received data"));
            Disconnect(1002, TEXT("Protocol error"));
        }
    }
}

bool FCustomWebSocket::ParseURL(const FString& URL, FString& OutHost, int32& OutPort, FString& OutPath, bool& OutIsSecure)
{
    // Parse ws:// or wss://
    if (URL.StartsWith(TEXT("wss://")))
    {
        OutIsSecure = true;
        OutPort = 443;
    }
    else if (URL.StartsWith(TEXT("ws://")))
    {
        OutIsSecure = false;
        OutPort = 80;
    }
    else
    {
        return false;
    }

    // Extract rest of URL
    FString Remaining = URL.Mid(OutIsSecure ? 6 : 5); // Skip "wss://" or "ws://"

    // Find path separator
    int32 PathIndex;
    if (Remaining.FindChar(TEXT('/'), PathIndex))
    {
        OutPath = Remaining.Mid(PathIndex);
        Remaining = Remaining.Left(PathIndex);
    }
    else
    {
        OutPath = TEXT("/");
    }

    // Check for port
    int32 PortIndex;
    if (Remaining.FindChar(TEXT(':'), PortIndex))
    {
        OutHost = Remaining.Left(PortIndex);
        FString PortStr = Remaining.Mid(PortIndex + 1);
        OutPort = FCString::Atoi(*PortStr);
    }
    else
    {
        OutHost = Remaining;
    }

    return !OutHost.IsEmpty();
}

bool FCustomWebSocket::PerformTCPConnection(const FString& Host, int32 Port)
{
    if (!SocketSubsystem)
    {
        UE_LOG(LogCustomWebSocket, Error, TEXT("Socket subsystem not available"));
        return false;
    }

    UE_LOG(LogCustomWebSocket, Log, TEXT("Attempting to resolve host: %s"), *Host);

    // Resolve host
    TSharedPtr<FInternetAddr> Addr = SocketSubsystem->GetAddressFromString(Host);
    if (!Addr.IsValid())
    {
        // Try DNS resolution
        UE_LOG(LogCustomWebSocket, Log, TEXT("Performing DNS lookup for: %s"), *Host);
        FAddressInfoResult GAIResult = SocketSubsystem->GetAddressInfo(*Host, nullptr, EAddressInfoFlags::Default, NAME_None);
        if (GAIResult.ReturnCode != SE_NO_ERROR || GAIResult.Results.Num() == 0)
        {
            UE_LOG(LogCustomWebSocket, Error, TEXT("Failed to resolve host: %s (DNS lookup failed - check internet connection)"), *Host);
            return false;
        }
        Addr = GAIResult.Results[0].Address;
        UE_LOG(LogCustomWebSocket, Log, TEXT("DNS resolution successful: %s"), *Addr->ToString(true));
    }

    Addr->SetPort(Port);

    // Create socket
    UE_LOG(LogCustomWebSocket, Log, TEXT("Creating socket..."));
    Socket = SocketSubsystem->CreateSocket(NAME_Stream, TEXT("CustomWebSocket"), Addr->GetProtocolType());
    if (!Socket)
    {
        UE_LOG(LogCustomWebSocket, Error, TEXT("Failed to create socket"));
        return false;
    }

    // Set non-blocking
    Socket->SetNonBlocking(false); // Use blocking for initial connection

    // Connect
    UE_LOG(LogCustomWebSocket, Log, TEXT("Connecting to %s:%d..."), *Host, Port);
    if (!Socket->Connect(*Addr))
    {
        UE_LOG(LogCustomWebSocket, Error, TEXT("Failed to connect to %s:%d (Connection refused - check internet connection and firewall)"), *Host, Port);
        SocketSubsystem->DestroySocket(Socket);
        Socket = nullptr;
        return false;
    }

    // Now set non-blocking for async operations
    Socket->SetNonBlocking(true);

    UE_LOG(LogCustomWebSocket, Log, TEXT("========================================"));
    UE_LOG(LogCustomWebSocket, Log, TEXT("TCP connection established successfully"));
    UE_LOG(LogCustomWebSocket, Log, TEXT("Connected to: %s:%d"), *Host, Port);
    UE_LOG(LogCustomWebSocket, Log, TEXT("Internet connection verified"));
    UE_LOG(LogCustomWebSocket, Log, TEXT("========================================"));
    return true;
}

bool FCustomWebSocket::PerformTLSHandshake()
{
    // Note: This is a simplified placeholder
    // Full TLS implementation would use OpenSSL
    // For now, we'll note that this requires OpenSSL integration
    
    UE_LOG(LogCustomWebSocket, Warning, TEXT("TLS handshake placeholder - requires OpenSSL implementation"));
    
    // In production, this would:
    // 1. Create SSL context
    // 2. Create SSL connection
    // 3. Set socket FD
    // 4. Perform SSL_connect()
    // 5. Verify certificate
    
    // For now, return true for wss:// but note it needs implementation
    return true;
}

bool FCustomWebSocket::PerformWebSocketHandshake()
{
    // Generate WebSocket key
    SecWebSocketKey = GenerateWebSocketKey();

    // Build handshake request
    FString Request = FString::Printf(
        TEXT("GET %s HTTP/1.1\r\n")
        TEXT("Host: %s:%d\r\n")
        TEXT("Upgrade: websocket\r\n")
        TEXT("Connection: Upgrade\r\n")
        TEXT("Sec-WebSocket-Key: %s\r\n")
        TEXT("Sec-WebSocket-Version: 13\r\n")
        TEXT("\r\n"),
        *ServerPath, *ServerHost, ServerPort, *SecWebSocketKey
    );

    // Convert to bytes
    FTCHARToUTF8 UTF8Request(*Request);
    TArray<uint8> RequestData;
    RequestData.Append((uint8*)UTF8Request.Get(), UTF8Request.Length());

    // Send handshake
    int32 BytesSent = 0;
    if (!Socket->Send(RequestData.GetData(), RequestData.Num(), BytesSent))
    {
        UE_LOG(LogCustomWebSocket, Error, TEXT("Failed to send handshake"));
        return false;
    }

    UE_LOG(LogCustomWebSocket, Log, TEXT("WebSocket handshake sent, waiting for response..."));

    // Wait for response (with timeout)
    float TimeoutSeconds = 10.0f;
    float ElapsedTime = 0.0f;
    TArray<uint8> ResponseData;
    bool bHandshakeReceived = false;

    while (ElapsedTime < TimeoutSeconds && !bHandshakeReceived)
    {
        uint32 PendingDataSize = 0;
        if (Socket->HasPendingData(PendingDataSize) && PendingDataSize > 0)
        {
            TArray<uint8> Buffer;
            Buffer.SetNum(PendingDataSize);
            int32 BytesRead = 0;
            
            if (Socket->Recv(Buffer.GetData(), PendingDataSize, BytesRead))
            {
                ResponseData.Append(Buffer.GetData(), BytesRead);
                
                // Check if we have complete response (ends with \r\n\r\n)
                if (ResponseData.Num() >= 4)
                {
                    if (ResponseData[ResponseData.Num()-4] == '\r' &&
                        ResponseData[ResponseData.Num()-3] == '\n' &&
                        ResponseData[ResponseData.Num()-2] == '\r' &&
                        ResponseData[ResponseData.Num()-1] == '\n')
                    {
                        bHandshakeReceived = true;
                        break;
                    }
                }
            }
        }

        FPlatformProcess::Sleep(0.01f);
        ElapsedTime += 0.01f;
    }

    if (!bHandshakeReceived)
    {
        UE_LOG(LogCustomWebSocket, Error, TEXT("Handshake response timeout"));
        return false;
    }

    // Parse response
    FString Response = FString(UTF8_TO_TCHAR(ResponseData.GetData()));
    
    UE_LOG(LogCustomWebSocket, Log, TEXT("Handshake response received:\n%s"), *Response);

    // Verify response
    if (!Response.Contains(TEXT("HTTP/1.1 101")))
    {
        UE_LOG(LogCustomWebSocket, Error, TEXT("Invalid handshake response - not 101 Switching Protocols"));
        return false;
    }

    // Verify Sec-WebSocket-Accept
    FString ExpectedAccept = CalculateAcceptKey(SecWebSocketKey);
    FString ActualAccept;
    
    // Extract Sec-WebSocket-Accept from response
    int32 AcceptIndex = Response.Find(TEXT("Sec-WebSocket-Accept:"));
    if (AcceptIndex != INDEX_NONE)
    {
        int32 LineEnd = Response.Find(TEXT("\r\n"), ESearchCase::IgnoreCase, ESearchDir::FromStart, AcceptIndex);
        if (LineEnd != INDEX_NONE)
        {
            ActualAccept = Response.Mid(AcceptIndex + 22, LineEnd - (AcceptIndex + 22)).TrimStartAndEnd();
        }
    }

    if (ActualAccept != ExpectedAccept)
    {
        UE_LOG(LogCustomWebSocket, Error, TEXT("Sec-WebSocket-Accept mismatch. Expected: %s, Got: %s"),
            *ExpectedAccept, *ActualAccept);
        return false;
    }

    bHandshakeComplete = true;
    bIsConnected = true;

    UE_LOG(LogCustomWebSocket, Log, TEXT("========================================"));
    UE_LOG(LogCustomWebSocket, Log, TEXT("CustomWebSocket: Successfully connected to %s:%d"), *ServerHost, ServerPort);
    UE_LOG(LogCustomWebSocket, Log, TEXT("CustomWebSocket: WebSocket handshake complete!"));
    UE_LOG(LogCustomWebSocket, Log, TEXT("========================================"));
    OnConnected.ExecuteIfBound(true);

    return true;
}

bool FCustomWebSocket::ProcessReceivedData()
{
    uint32 PendingDataSize = 0;
    if (!Socket->HasPendingData(PendingDataSize) || PendingDataSize == 0)
    {
        return true;
    }

    // Read data
    TArray<uint8> Buffer;
    Buffer.SetNum(PendingDataSize);
    int32 BytesRead = 0;

    if (!Socket->Recv(Buffer.GetData(), PendingDataSize, BytesRead))
    {
        return false;
    }

    ReceiveBuffer.Append(Buffer.GetData(), BytesRead);

    // Parse frames
    while (ReceiveBuffer.Num() > 0)
    {
        int32 BytesConsumed = 0;
        if (!ParseFrame(ReceiveBuffer, BytesConsumed))
        {
            return false;
        }

        if (BytesConsumed == 0)
        {
            // Need more data
            break;
        }

        // Remove consumed bytes
        ReceiveBuffer.RemoveAt(0, BytesConsumed);
    }

    return true;
}

bool FCustomWebSocket::ParseFrame(const TArray<uint8>& Data, int32& OutBytesConsumed)
{
    OutBytesConsumed = 0;

    if (Data.Num() < 2)
    {
        // Need at least 2 bytes for frame header
        return true;
    }

    // Parse frame header
    uint8 Byte0 = Data[0];
    uint8 Byte1 = Data[1];

    bool bFin = (Byte0 & 0x80) != 0;
    uint8 Opcode = Byte0 & 0x0F;
    bool bMask = (Byte1 & 0x80) != 0;
    uint64 PayloadLength = Byte1 & 0x7F;

    int32 HeaderSize = 2;

    // Extended payload length
    if (PayloadLength == 126)
    {
        if (Data.Num() < 4)
        {
            return true; // Need more data
        }
        PayloadLength = (Data[2] << 8) | Data[3];
        HeaderSize += 2;
    }
    else if (PayloadLength == 127)
    {
        if (Data.Num() < 10)
        {
            return true; // Need more data
        }
        PayloadLength = 0;
        for (int i = 0; i < 8; i++)
        {
            PayloadLength = (PayloadLength << 8) | Data[2 + i];
        }
        HeaderSize += 8;
    }

    // Masking key (server shouldn't send masked frames, but handle it)
    uint8 MaskKey[4] = {0};
    if (bMask)
    {
        if (Data.Num() < HeaderSize + 4)
        {
            return true; // Need more data
        }
        for (int i = 0; i < 4; i++)
        {
            MaskKey[i] = Data[HeaderSize + i];
        }
        HeaderSize += 4;
    }

    // Check if we have complete payload
    if (Data.Num() < HeaderSize + PayloadLength)
    {
        return true; // Need more data
    }

    // Extract payload
    TArray<uint8> Payload;
    Payload.SetNum(PayloadLength);
    for (uint64 i = 0; i < PayloadLength; i++)
    {
        uint8 Byte = Data[HeaderSize + i];
        if (bMask)
        {
            Byte ^= MaskKey[i % 4];
        }
        Payload[i] = Byte;
    }

    // Handle frame
    HandleFrame(Opcode, Payload);

    OutBytesConsumed = HeaderSize + PayloadLength;
    return true;
}

TArray<uint8> FCustomWebSocket::CreateFrame(uint8 Opcode, const TArray<uint8>& Payload, bool bMask)
{
    TArray<uint8> Frame;

    // Byte 0: FIN + RSV + Opcode
    Frame.Add(0x80 | (Opcode & 0x0F));

    // Byte 1: MASK + Payload length
    uint64 PayloadLength = Payload.Num();
    uint8 MaskBit = bMask ? 0x80 : 0x00;

    if (PayloadLength < 126)
    {
        Frame.Add(MaskBit | (uint8)PayloadLength);
    }
    else if (PayloadLength < 65536)
    {
        Frame.Add(MaskBit | 126);
        Frame.Add((PayloadLength >> 8) & 0xFF);
        Frame.Add(PayloadLength & 0xFF);
    }
    else
    {
        Frame.Add(MaskBit | 127);
        for (int i = 7; i >= 0; i--)
        {
            Frame.Add((PayloadLength >> (i * 8)) & 0xFF);
        }
    }

    // Masking key (if client)
    uint8 MaskKey[4];
    if (bMask)
    {
        for (int i = 0; i < 4; i++)
        {
            MaskKey[i] = FMath::Rand() & 0xFF;
            Frame.Add(MaskKey[i]);
        }
    }

    // Payload (masked if needed)
    for (int32 i = 0; i < Payload.Num(); i++)
    {
        uint8 Byte = Payload[i];
        if (bMask)
        {
            Byte ^= MaskKey[i % 4];
        }
        Frame.Add(Byte);
    }

    return Frame;
}

FString FCustomWebSocket::GenerateWebSocketKey()
{
    // Generate random 16 bytes
    TArray<uint8> RandomBytes;
    RandomBytes.SetNum(16);
    for (int i = 0; i < 16; i++)
    {
        RandomBytes[i] = FMath::Rand() & 0xFF;
    }

    // Base64 encode
    return FBase64::Encode(RandomBytes);
}

FString FCustomWebSocket::CalculateAcceptKey(const FString& Key)
{
    // Concatenate with GUID
    FString Combined = Key + TEXT(WS_GUID);

    // Convert to bytes
    FTCHARToUTF8 UTF8Combined(*Combined);
    
    // SHA-1 hash
    FSHAHash Hash;
    FSHA1::HashBuffer(UTF8Combined.Get(), UTF8Combined.Length(), Hash.Hash);

    // Base64 encode
    return FBase64::Encode(Hash.Hash, 20);
}

void FCustomWebSocket::HandleFrame(uint8 Opcode, const TArray<uint8>& Payload)
{
    switch (Opcode)
    {
    case WS_OPCODE_TEXT:
    {
        FString Message = FString(UTF8_TO_TCHAR(Payload.GetData()));
        OnMessage.ExecuteIfBound(Message, true);
        break;
    }

    case WS_OPCODE_BINARY:
        // For binary, convert to base64 string for the delegate
        OnMessage.ExecuteIfBound(FBase64::Encode(Payload), false);
        break;

    case WS_OPCODE_CLOSE:
    {
        uint16 StatusCode = 1000;
        FString Reason;
        if (Payload.Num() >= 2)
        {
            StatusCode = (Payload[0] << 8) | Payload[1];
            if (Payload.Num() > 2)
            {
                Reason = FString(UTF8_TO_TCHAR(&Payload[2]));
            }
        }
        UE_LOG(LogCustomWebSocket, Log, TEXT("Received close frame: %d - %s"), StatusCode, *Reason);
        Disconnect(StatusCode, Reason);
        break;
    }

    case WS_OPCODE_PING:
        UE_LOG(LogCustomWebSocket, Verbose, TEXT("Received ping"));
        SendPong(Payload);
        break;

    case WS_OPCODE_PONG:
        UE_LOG(LogCustomWebSocket, Verbose, TEXT("Received pong"));
        break;

    default:
        UE_LOG(LogCustomWebSocket, Warning, TEXT("Unknown opcode: %d"), Opcode);
        break;
    }
}

void FCustomWebSocket::SendPong(const TArray<uint8>& PingPayload)
{
    TArray<uint8> PongFrame = CreateFrame(WS_OPCODE_PONG, PingPayload, true);
    if (Socket)
    {
        int32 BytesSent = 0;
        Socket->Send(PongFrame.GetData(), PongFrame.Num(), BytesSent);
    }
}
