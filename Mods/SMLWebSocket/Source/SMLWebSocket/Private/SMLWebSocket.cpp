// Copyright Epic Games, Inc. All Rights Reserved.

#include "SMLWebSocket.h"

#include "SMLWebSocketModule.h"

#include "Async/Async.h"
#include "HAL/PlatformProcess.h"
#include "HAL/Runnable.h"
#include "HAL/RunnableThread.h"
#include "Misc/Base64.h"
#include "Misc/SecureHash.h"
#include "Networking.h"
#include "Sockets.h"
#include "SocketSubsystem.h"

// OpenSSL headers (available via the "OpenSSL" UE module dependency)
#include "openssl/ssl.h"
#include "openssl/err.h"
#include "openssl/x509v3.h"

DEFINE_LOG_CATEGORY_STATIC(LogSMLWebSocket, Log, All);

// ---------------------------------------------------------------------------
// Constants
// ---------------------------------------------------------------------------

// RFC 6455 magic GUID appended to the client key before SHA-1 hashing.
static const ANSICHAR* WS_ACCEPT_MAGIC = "258EAFA5-E914-47DA-95CA-C5AB0DC85B11";

// Maximum accepted payload size per frame (64 MiB) – a sanity guard.
static constexpr uint64 WS_MAX_FRAME_PAYLOAD = 64 * 1024 * 1024;

// WebSocket opcodes (RFC 6455 §11.8)
namespace EWSOpcode
{
	enum Type : uint8
	{
		Continuation = 0x00,
		Text         = 0x01,
		Binary       = 0x02,
		Close        = 0x08,
		Ping         = 0x09,
		Pong         = 0x0A,
	};
}

// ---------------------------------------------------------------------------
// OpenSSL helper: collect pending error strings
// ---------------------------------------------------------------------------

static FString GetSslErrors()
{
	FString Out;
	unsigned long Err;
	while ((Err = ERR_get_error()) != 0)
	{
		char Buf[256];
		ERR_error_string_n(Err, Buf, sizeof(Buf));
		if (!Out.IsEmpty()) Out += TEXT("; ");
		Out += UTF8_TO_TCHAR(Buf);
	}
	return Out.IsEmpty() ? TEXT("(no SSL error detail)") : Out;
}

// ---------------------------------------------------------------------------
// Custom OpenSSL BIO backed by FSocket
//
// OpenSSL's BIO (Basic I/O) abstraction lets us plug in any I/O source.  By
// creating a BIO that delegates to FSocket we avoid needing the raw platform
// socket handle (SOCKET / fd) and keep UE's socket ownership rules intact.
// ---------------------------------------------------------------------------

static int SMLSocketBioWrite(BIO* Bio, const char* Buf, int Num)
{
	FSocket* Sock = static_cast<FSocket*>(BIO_get_data(Bio));
	BIO_clear_retry_flags(Bio);
	if (!Sock || !Buf || Num <= 0) return -1;

	int32 BytesSent = 0;
	if (!Sock->Send(reinterpret_cast<const uint8*>(Buf), Num, BytesSent))
	{
		return -1;
	}
	return BytesSent > 0 ? BytesSent : -1;
}

static int SMLSocketBioRead(BIO* Bio, char* Buf, int Num)
{
	FSocket* Sock = static_cast<FSocket*>(BIO_get_data(Bio));
	BIO_clear_retry_flags(Bio);
	if (!Sock || !Buf || Num <= 0) return -1;

	int32 BytesRead = 0;
	if (!Sock->Recv(reinterpret_cast<uint8*>(Buf), Num, BytesRead))
	{
		return -1; // socket error or closed
	}
	if (BytesRead == 0)
	{
		// No data yet – tell OpenSSL to retry rather than treating it as EOF.
		BIO_set_retry_read(Bio);
		return -1;
	}
	return BytesRead;
}

static long SMLSocketBioCtrl(BIO* /*Bio*/, int Cmd, long /*Num*/, void* /*Ptr*/)
{
	// Only BIO_CTRL_FLUSH is meaningful; everything else can return 0.
	return Cmd == BIO_CTRL_FLUSH ? 1L : 0L;
}

static int SMLSocketBioCreate(BIO* Bio)
{
	BIO_set_init(Bio, 1);
	return 1;
}

static int SMLSocketBioDestroy(BIO* Bio)
{
	BIO_set_data(Bio, nullptr);
	BIO_set_init(Bio, 0);
	return 1;
}

/** Returns the singleton BIO_METHOD for FSocket-backed BIOs (created once). */
static BIO_METHOD* GetSMLSocketBioMethod()
{
	// Deliberately leaked on shutdown; this matches how OpenSSL itself handles
	// built-in BIO methods.
	static BIO_METHOD* Method = nullptr;
	if (!Method)
	{
		Method = BIO_meth_new(BIO_TYPE_SOURCE_SINK | BIO_get_new_index(), "SMLSocket");
		check(Method);
		BIO_meth_set_write(Method, SMLSocketBioWrite);
		BIO_meth_set_read(Method, SMLSocketBioRead);
		BIO_meth_set_ctrl(Method, SMLSocketBioCtrl);
		BIO_meth_set_create(Method, SMLSocketBioCreate);
		BIO_meth_set_destroy(Method, SMLSocketBioDestroy);
	}
	return Method;
}

// ---------------------------------------------------------------------------

/** Compute the Sec-WebSocket-Accept header value for the given nonce key. */
static FString ComputeWebSocketAccept(const FString& Key)
{
	// Accept = Base64( SHA1( Key + Magic ) )
	FString Combined = Key + UTF8_TO_TCHAR(WS_ACCEPT_MAGIC);
	FTCHARToUTF8 Converter(*Combined);

	uint8 Hash[20];
	FSHA1::HashBuffer(Converter.Get(), Converter.Length(), Hash);

	TArray<uint8> HashArray(Hash, 20);
	FString Result;
	FBase64::Encode(HashArray, Result);
	return Result;
}

/** Parse a ws:// or wss:// URL into (Host, Port, Path, IsSecure). Returns false on failure. */
static bool ParseWebSocketUrl(const FString& Url,
	FString& OutHost, int32& OutPort, FString& OutPath, bool& OutIsSecure)
{
	FString Rest;

	if (Url.StartsWith(TEXT("wss://"), ESearchCase::IgnoreCase))
	{
		OutIsSecure = true;
		Rest = Url.Mid(6); // strip "wss://"
	}
	else if (Url.StartsWith(TEXT("ws://"), ESearchCase::IgnoreCase))
	{
		OutIsSecure = false;
		Rest = Url.Mid(5); // strip "ws://"
	}
	else
	{
		UE_LOG(LogSMLWebSocket, Error,
			TEXT("URL scheme must be 'ws://' or 'wss://' (got: %s)."), *Url);
		return false;
	}

	// Separate path (and query string)
	int32 SlashIdx;
	if (Rest.FindChar(TEXT('/'), SlashIdx))
	{
		OutPath = Rest.Mid(SlashIdx);
		Rest    = Rest.Left(SlashIdx);
	}
	else
	{
		// Check for query string without path
		int32 QueryIdx;
		if (Rest.FindChar(TEXT('?'), QueryIdx))
		{
			OutPath = TEXT("/") + Rest.Mid(QueryIdx);
			Rest    = Rest.Left(QueryIdx);
		}
		else
		{
			OutPath = TEXT("/");
		}
	}

	// Separate port
	int32 ColonIdx;
	if (Rest.FindLastChar(TEXT(':'), ColonIdx))
	{
		OutHost = Rest.Left(ColonIdx);
		OutPort = FCString::Atoi(*Rest.Mid(ColonIdx + 1));
		if (OutPort <= 0 || OutPort > 65535)
		{
			UE_LOG(LogSMLWebSocket, Error, TEXT("Invalid port in URL: %s"), *Url);
			return false;
		}
	}
	else
	{
		OutHost = Rest;
		OutPort = OutIsSecure ? 443 : 80; // default port per scheme
	}

	if (OutHost.IsEmpty())
	{
		UE_LOG(LogSMLWebSocket, Error, TEXT("Empty hostname in URL: %s"), *Url);
		return false;
	}

	return true;
}

// ---------------------------------------------------------------------------
// FSMLWebSocketWorker  –  background thread
// ---------------------------------------------------------------------------

/**
 * Background FRunnable that:
 *   1. Creates and connects the TCP socket.
 *   2. Performs the RFC 6455 WebSocket upgrade handshake.
 *   3. Reads WebSocket frames in a loop and dispatches events to the game thread.
 *
 * The owning USMLWebSocket outlives this worker because BeginDestroy() calls
 * TearDown() which kills this thread before releasing any memory.
 */
class FSMLWebSocketWorker final : public FRunnable
{
public:
	FSMLWebSocketWorker(USMLWebSocket* InOwner,
		FString InHost, int32 InPort, FString InPath,
		TMap<FString, FString> InHeaders, bool InIsSecure)
		: Owner(InOwner)
		, Host(MoveTemp(InHost))
		, Port(InPort)
		, Path(MoveTemp(InPath))
		, ExtraHeaders(MoveTemp(InHeaders))
		, bIsSecure(InIsSecure)
	{}

	virtual bool Init() override { return true; }
	virtual void Stop() override { /* bStopping is set by the owner */ }
	virtual void Exit() override {}

	virtual uint32 Run() override
	{
		int32 ReconnectCount = 0; // number of reconnect attempts made so far

		while (!Owner->bStopping)
		{
			// ----------------------------------------------------------------
			// Reconnect delay (skipped on the very first attempt)
			// ----------------------------------------------------------------
			if (ReconnectCount > 0)
			{
				// Exponential back-off: delay = initial * 2^(attempt-1), capped at 60 s.
				const float Delay = FMath::Min(
					Owner->ReconnectInitialDelaySeconds * FMath::Pow(2.f, float(ReconnectCount - 1)),
					60.f);

				UE_LOG(LogSMLWebSocket, Log,
					TEXT("SMLWebSocket: reconnect attempt %d in %.1f seconds..."),
					ReconnectCount, Delay);

				DispatchToGameThread(
					[WeakOwner = TWeakObjectPtr<USMLWebSocket>(Owner), ReconnectCount]()
				{
					if (USMLWebSocket* O = WeakOwner.Get())
					{
						O->OnReconnecting.Broadcast(ReconnectCount);
					}
				});

				// Sleep in 100 ms slices so bStopping is honoured quickly.
				float Elapsed = 0.f;
				while (Elapsed < Delay && !Owner->bStopping)
				{
					FPlatformProcess::Sleep(0.1f);
					Elapsed += 0.1f;
				}

				if (Owner->bStopping) break;
			}

			// ----------------------------------------------------------------
			// Single connection attempt
			// ----------------------------------------------------------------
			RunOnce();

			if (Owner->bStopping) break;
			if (!Owner->bAutoReconnect) break;
			if (Owner->MaxReconnectAttempts > 0 && ReconnectCount >= Owner->MaxReconnectAttempts) break;

			++ReconnectCount;
		}

		return 0;
	}

private:
	/**
	 * Perform one complete connect → handshake → read-loop cycle.
	 * Errors are dispatched to the game thread via DispatchError().
	 * On return, the socket is always cleaned up.
	 */
	void RunOnce()
	{
		ISocketSubsystem* SS = ISocketSubsystem::Get(NAME_None);
		if (!SS)
		{
			DispatchError(TEXT("ISocketSubsystem not available"));
			return;
		}

		// -------------------------------------------------------------------
		// 1. DNS resolution
		// -------------------------------------------------------------------
		FAddressInfoResult AddrResult = SS->GetAddressInfo(
			*Host, nullptr, EAddressInfoFlags::Default, NAME_None);

		if (AddrResult.ReturnCode != SE_NO_ERROR || AddrResult.Results.IsEmpty())
		{
			DispatchError(FString::Printf(
				TEXT("DNS resolution failed for '%s': error %d"),
				*Host, (int32)AddrResult.ReturnCode));
			return;
		}

		TSharedRef<FInternetAddr> Addr = AddrResult.Results[0].Address;
		Addr->SetPort(Port);

		// -------------------------------------------------------------------
		// 2. Create & connect TCP socket
		// -------------------------------------------------------------------
		FSocket* Sock = SS->CreateSocket(NAME_Stream,
			TEXT("SMLWebSocket"), Addr->GetProtocolType());
		if (!Sock)
		{
			DispatchError(TEXT("Failed to create TCP socket"));
			return;
		}

		if (!Sock->Connect(*Addr))
		{
			SS->DestroySocket(Sock);
			DispatchError(FString::Printf(
				TEXT("TCP connect to %s:%d failed"), *Host, Port));
			return;
		}

		// Hand the live socket to the owner so SendFrame() can use it.
		{
			FScopeLock Lock(&Owner->SendLock);
			Owner->Socket = Sock;
		}

		// -------------------------------------------------------------------
		// 3. TLS handshake (wss:// only)
		// -------------------------------------------------------------------
		if (bIsSecure && !SetupTls(Sock))
		{
			// SetupTls already dispatched an error event.
			FScopeLock Lock(&Owner->SendLock);
			Sock->Close();
			SS->DestroySocket(Sock);
			Owner->Socket = nullptr;
			return;
		}

		// -------------------------------------------------------------------
		// 4. WebSocket upgrade handshake
		// -------------------------------------------------------------------
		if (!PerformHandshake(Sock))
		{
			TearDownTls();
			FScopeLock Lock(&Owner->SendLock);
			Sock->Close();
			SS->DestroySocket(Sock);
			Owner->Socket = nullptr;
			return;
		}

		// Handshake succeeded – notify the game thread.
		Owner->bConnected = true;
		DispatchToGameThread([WeakOwner = TWeakObjectPtr<USMLWebSocket>(Owner)]()
		{
			if (USMLWebSocket* O = WeakOwner.Get())
			{
				O->OnConnected.Broadcast();
			}
		});

		// -------------------------------------------------------------------
		// 5. Frame read loop
		// -------------------------------------------------------------------
		ReadLoop(Sock);

		// -------------------------------------------------------------------
		// 6. Cleanup
		// -------------------------------------------------------------------
		Owner->bConnected = false;
		TearDownTls();
		{
			FScopeLock Lock(&Owner->SendLock);
			if (Owner->Socket == Sock)
			{
				Owner->Socket = nullptr;
			}
		}
		Sock->Close();
		SS->DestroySocket(Sock);
	}

	// -----------------------------------------------------------------------
	// TLS setup / teardown
	// -----------------------------------------------------------------------

	/**
	 * Wrap an already-connected FSocket in TLS.
	 * Called from RunOnce() AFTER TCP connect, BEFORE the WebSocket handshake.
	 * On success, Owner->Ssl and Owner->SslCtx are set.
	 * On failure, an error event is dispatched and false is returned.
	 */
	bool SetupTls(FSocket* Sock)
	{
		// ---- Create SSL context ----
		SSL_CTX* Ctx = SSL_CTX_new(TLS_client_method());
		if (!Ctx)
		{
			DispatchError(FString::Printf(
				TEXT("TLS: SSL_CTX_new failed: %s"), *GetSslErrors()));
			return false;
		}

		// Require TLS 1.2 or higher (1.0 / 1.1 are deprecated)
		SSL_CTX_set_min_proto_version(Ctx, TLS1_2_VERSION);

		// Certificate verification
		if (Owner->bVerifyTlsCertificate)
		{
			SSL_CTX_set_verify(Ctx, SSL_VERIFY_PEER, nullptr);
			// Use the OS default trust store (works on Linux; Windows requires
			// WinCrypt which the game's OpenSSL build may handle automatically)
			SSL_CTX_set_default_verify_paths(Ctx);
		}
		else
		{
			UE_LOG(LogSMLWebSocket, Warning,
				TEXT("TLS: bVerifyTlsCertificate is false – "
				     "server certificate will NOT be verified (insecure)"));
			SSL_CTX_set_verify(Ctx, SSL_VERIFY_NONE, nullptr);
		}

		// ---- Create SSL object ----
		SSL* NewSsl = SSL_new(Ctx);
		if (!NewSsl)
		{
			SSL_CTX_free(Ctx);
			DispatchError(FString::Printf(
				TEXT("TLS: SSL_new failed: %s"), *GetSslErrors()));
			return false;
		}

		// ---- SNI: include the server hostname in the TLS ClientHello ----
		const FTCHARToUTF8 HostUtf8(*Host);
		SSL_set_tlsext_host_name(NewSsl, HostUtf8.Get());

		// ---- Hostname verification (checks CN / SAN fields in the cert) ----
		if (Owner->bVerifyTlsCertificate)
		{
			SSL_set_hostflags(NewSsl, X509_CHECK_FLAG_NO_PARTIAL_WILDCARDS);
			SSL_set1_host(NewSsl, HostUtf8.Get());
		}

		// ---- Attach a BIO that reads/writes through FSocket ----
		BIO* Bio = BIO_new(GetSMLSocketBioMethod());
		if (!Bio)
		{
			SSL_free(NewSsl);
			SSL_CTX_free(Ctx);
			DispatchError(TEXT("TLS: BIO_new failed"));
			return false;
		}
		BIO_set_data(Bio, Sock); // store FSocket* so the BIO callbacks can use it
		SSL_set_bio(NewSsl, Bio, Bio); // SSL takes ownership of Bio

		// ---- Perform TLS handshake (blocks until done) ----
		const int Result = SSL_connect(NewSsl);
		if (Result != 1)
		{
			const int ErrCode = SSL_get_error(NewSsl, Result);
			DispatchError(FString::Printf(
				TEXT("TLS handshake with %s failed (SSL_error=%d): %s"),
				*Host, ErrCode, *GetSslErrors()));
			SSL_free(NewSsl); // also frees Bio
			SSL_CTX_free(Ctx);
			return false;
		}

		UE_LOG(LogSMLWebSocket, Log,
			TEXT("TLS handshake with %s succeeded (cipher: %s)"),
			*Host, UTF8_TO_TCHAR(SSL_get_cipher(NewSsl)));

		// Store for use in SendFrame() and RecvExact().
		// SendLock serialises access from the main thread.
		{
			FScopeLock Lock(&Owner->SendLock);
			Owner->Ssl    = NewSsl;
			Owner->SslCtx = Ctx;
		}
		return true;
	}

	/**
	 * Free the SSL/SSL_CTX objects stored on the owner.
	 * Safe to call multiple times.  MUST be called from the worker thread
	 * BEFORE the socket is destroyed (the BIO holds a raw FSocket pointer).
	 */
	void TearDownTls()
	{
		ssl_st*     OldSsl = nullptr;
		ssl_ctx_st* OldCtx = nullptr;
		{
			FScopeLock Lock(&Owner->SendLock);
			OldSsl        = Owner->Ssl;
			OldCtx        = Owner->SslCtx;
			Owner->Ssl    = nullptr;
			Owner->SslCtx = nullptr;
		}
		if (OldSsl) SSL_free(OldSsl);  // also frees the BIO
		if (OldCtx) SSL_CTX_free(OldCtx);
	}

	// -----------------------------------------------------------------------
	// TLS-aware I/O helpers
	//   WorkerSend / WorkerRecv abstract over plain-TCP vs TLS so the
	//   PerformHandshake and RecvExact functions don't need to branch.
	// -----------------------------------------------------------------------

	/**
	 * Send all Num bytes through the socket (or SSL layer).
	 * Returns false on any error.
	 */
	bool WorkerSend(FSocket* Sock, const uint8* Data, int32 Num)
	{
		if (bIsSecure)
		{
			// SSL_write is serialised by the caller holding SendLock implicitly
			// because this function is only called from PerformHandshake which
			// runs before the read-loop (i.e., before any concurrent SendFrame calls).
			ssl_st* Ssl = Owner->Ssl;
			if (!Ssl) return false;
			return SSL_write(Ssl, Data, Num) == Num;
		}
		int32 BytesSent = 0;
		return Sock->Send(Data, Num, BytesSent) && BytesSent == Num;
	}

	/**
	 * Receive up to MaxNum bytes.  Returns the number of bytes read, or ≤0 on error.
	 * For TLS: wraps SSL_read; for plain TCP: wraps FSocket::Recv.
	 */
	int32 WorkerRecv(FSocket* Sock, uint8* Buf, int32 MaxNum)
	{
		if (bIsSecure)
		{
			ssl_st* Ssl = Owner->Ssl;
			if (!Ssl) return -1;
			const int N = SSL_read(Ssl, Buf, MaxNum);
			if (N <= 0)
			{
				const int Err = SSL_get_error(Ssl, N);
				// SSL_ERROR_WANT_READ/WRITE mean "no data yet – retry"
				if (Err == SSL_ERROR_WANT_READ || Err == SSL_ERROR_WANT_WRITE)
				{
					return 0;
				}
				return -1; // genuine error or connection closed
			}
			return N;
		}
		int32 BytesRead = 0;
		if (!Sock->Recv(Buf, MaxNum, BytesRead)) return -1;
		return BytesRead;
	}

	// -----------------------------------------------------------------------
	// Handshake
	// -----------------------------------------------------------------------

	bool PerformHandshake(FSocket* Sock)
	{
		// Generate a random 16-byte nonce and base64-encode it.
		TArray<uint8> NonceBytes;
		NonceBytes.SetNumUninitialized(16);
		for (uint8& B : NonceBytes)
		{
			B = static_cast<uint8>(FMath::RandRange(0, 255));
		}
		FString Nonce;
		FBase64::Encode(NonceBytes, Nonce);

		// Build the HTTP upgrade request.
		FString Request = FString::Printf(
			TEXT("GET %s HTTP/1.1\r\n"
			     "Host: %s:%d\r\n"
			     "Upgrade: websocket\r\n"
			     "Connection: Upgrade\r\n"
			     "Sec-WebSocket-Key: %s\r\n"
			     "Sec-WebSocket-Version: 13\r\n"),
			*Path, *Host, Port, *Nonce);

		for (const TPair<FString, FString>& Header : ExtraHeaders)
		{
			Request += FString::Printf(TEXT("%s: %s\r\n"), *Header.Key, *Header.Value);
		}
		Request += TEXT("\r\n");

		// Send the HTTP request (via TLS if secure).
		FTCHARToUTF8 RequestUTF8(*Request);
		if (!WorkerSend(Sock,
			reinterpret_cast<const uint8*>(RequestUTF8.Get()),
			RequestUTF8.Length()))
		{
			DispatchError(TEXT("Failed to send WebSocket handshake request"));
			return false;
		}

		// Read the HTTP response (up to 4 KB; a proper 101 response is short).
		// Allocate 1 extra byte for null-termination when converting to FString.
		TArray<uint8> ResponseBuf;
		ResponseBuf.SetNumZeroed(4097); // 4096 data + 1 null terminator
		int32 TotalRead = 0;
		bool bFound = false;

		while (TotalRead < 4096 && !Owner->bStopping)
		{
			const int32 Read = WorkerRecv(Sock,
				ResponseBuf.GetData() + TotalRead, 4096 - TotalRead);

			if (Read < 0)
			{
				DispatchError(TEXT("Connection closed while reading WebSocket handshake response"));
				return false;
			}
			if (Read == 0)
			{
				FPlatformProcess::Sleep(0.001f); // no data yet – yield and retry
				continue;
			}

			TotalRead += Read;
			// Look for the blank line that ends HTTP headers ("\r\n\r\n").
			for (int32 i = 0; i + 3 < TotalRead; ++i)
			{
				if (ResponseBuf[i]   == '\r' && ResponseBuf[i+1] == '\n' &&
				    ResponseBuf[i+2] == '\r' && ResponseBuf[i+3] == '\n')
				{
					bFound = true;
					break;
				}
			}
			if (bFound) break;
		}

		if (!bFound)
		{
			DispatchError(TEXT("Incomplete or missing HTTP response during WebSocket handshake"));
			return false;
		}

		// Convert to a string for easy parsing.
		ResponseBuf[TotalRead] = 0;
		FString Response = FString(UTF8_TO_TCHAR(
			reinterpret_cast<const ANSICHAR*>(ResponseBuf.GetData())));

		// Check status line.
		if (!Response.Contains(TEXT("101")))
		{
			DispatchError(FString::Printf(
				TEXT("Server did not return 101 Switching Protocols. Response: %s"),
				*Response.Left(256)));
			return false;
		}

		// Verify Sec-WebSocket-Accept.
		FString ExpectedAccept = ComputeWebSocketAccept(Nonce);
		if (!Response.Contains(ExpectedAccept))
		{
			DispatchError(TEXT("Sec-WebSocket-Accept header mismatch – invalid server"));
			return false;
		}

		return true;
	}

	// -----------------------------------------------------------------------
	// Frame read loop
	// -----------------------------------------------------------------------

	void ReadLoop(FSocket* Sock)
	{
		// Accumulate fragmented message payloads here.
		TArray<uint8> FragmentPayload;
		uint8         FragmentOpcode = 0;

		while (!Owner->bStopping)
		{
			// ---- Read frame header (2 bytes) ----
			uint8 Header[2];
			if (!RecvExact(Sock, Header, 2))
			{
				if (!Owner->bStopping)
				{
					DispatchError(TEXT("Connection closed unexpectedly while reading frame header"));
				}
				return;
			}

			const bool  bFin    = (Header[0] & 0x80) != 0;
			const uint8 Opcode  = (Header[0] & 0x0F);
			const bool  bMasked = (Header[1] & 0x80) != 0;
			uint64      PayloadLen = (Header[1] & 0x7F);

			// ---- Extended payload length ----
			if (PayloadLen == 126)
			{
				uint8 Ext[2];
				if (!RecvExact(Sock, Ext, 2)) return;
				PayloadLen = (static_cast<uint64>(Ext[0]) << 8) | Ext[1];
			}
			else if (PayloadLen == 127)
			{
				uint8 Ext[8];
				if (!RecvExact(Sock, Ext, 8)) return;
				PayloadLen = 0;
				for (int32 i = 0; i < 8; ++i)
				{
					PayloadLen = (PayloadLen << 8) | Ext[i];
				}
			}

			if (PayloadLen > WS_MAX_FRAME_PAYLOAD)
			{
				DispatchError(FString::Printf(
					TEXT("Frame payload too large (%llu bytes; limit %llu)"),
					PayloadLen, WS_MAX_FRAME_PAYLOAD));
				return;
			}

			// ---- Masking key (server → client frames are normally unmasked) ----
			uint8 MaskKey[4] = {0, 0, 0, 0};
			if (bMasked)
			{
				if (!RecvExact(Sock, MaskKey, 4)) return;
			}

			// ---- Payload ----
			TArray<uint8> Payload;
			if (PayloadLen > 0)
			{
				Payload.SetNumUninitialized(static_cast<int32>(PayloadLen));
				if (!RecvExact(Sock, Payload.GetData(), static_cast<int32>(PayloadLen)))
				{
					return;
				}
				if (bMasked)
				{
					for (int32 i = 0; i < Payload.Num(); ++i)
					{
						Payload[i] ^= MaskKey[i & 3];
					}
				}
			}

			// ---- Dispatch by opcode ----
			switch (Opcode)
			{
			case EWSOpcode::Close:
				{
					int32  CloseCode   = 1000;
					FString CloseReason;
					if (Payload.Num() >= 2)
					{
						CloseCode = (static_cast<int32>(Payload[0]) << 8) | Payload[1];
						if (Payload.Num() > 2)
						{
							// Reason string is UTF-8; null-terminate before converting.
							const int32 ReasonStart = 2;
							const int32 ReasonLen   = Payload.Num() - ReasonStart;
							TArray<uint8> ReasonBytes(Payload.GetData() + ReasonStart, ReasonLen);
							ReasonBytes.Add(0);
							CloseReason = FString(
								UTF8_TO_TCHAR(reinterpret_cast<const ANSICHAR*>(ReasonBytes.GetData())));
						}
					}

					// Echo a close frame back (per RFC 6455 §5.5.1).
					uint8 ClosePayload[2] = {
						static_cast<uint8>(CloseCode >> 8),
						static_cast<uint8>(CloseCode & 0xFF)
					};
					Owner->SendFrame(EWSOpcode::Close, ClosePayload, 2);

					// Notify game thread.
					DispatchToGameThread(
						[WeakOwner = TWeakObjectPtr<USMLWebSocket>(Owner), CloseCode, CloseReason]()
					{
						if (USMLWebSocket* O = WeakOwner.Get())
						{
							O->OnClosed.Broadcast(CloseCode, CloseReason);
						}
					});
					return; // Exit read loop.
				}

			case EWSOpcode::Ping:
				// RFC 6455 §5.5.3 – respond with a pong containing the same payload.
				Owner->SendFrame(EWSOpcode::Pong, Payload.GetData(), Payload.Num());
				break;

			case EWSOpcode::Pong:
				// Nothing to do with unsolicited pongs.
				break;

			case EWSOpcode::Text:
			case EWSOpcode::Binary:
				// Start of a new message (potentially fragmented).
				FragmentOpcode   = Opcode;
				FragmentPayload  = MoveTemp(Payload);
				if (bFin)
				{
					DispatchMessage(FragmentPayload, FragmentOpcode == EWSOpcode::Binary);
					FragmentPayload.Reset();
					FragmentOpcode = 0;
				}
				break;

			case EWSOpcode::Continuation:
				// Continuation of a fragmented message.
				FragmentPayload.Append(Payload);
				if (bFin)
				{
					DispatchMessage(FragmentPayload, FragmentOpcode == EWSOpcode::Binary);
					FragmentPayload.Reset();
					FragmentOpcode = 0;
				}
				break;

			default:
				UE_LOG(LogSMLWebSocket, Warning,
					TEXT("Received unknown WebSocket opcode 0x%02X – ignoring"), Opcode);
				break;
			}
		}

		// bStopping was set – fire a synthetic close notification.
		DispatchToGameThread([WeakOwner = TWeakObjectPtr<USMLWebSocket>(Owner)]()
		{
			if (USMLWebSocket* O = WeakOwner.Get())
			{
				O->OnClosed.Broadcast(1001, TEXT("Going away"));
			}
		});
	}

	// -----------------------------------------------------------------------
	// Low-level helpers
	// -----------------------------------------------------------------------

	/**
	 * Receive exactly NumBytes bytes from the socket (or SSL layer), retrying as needed.
	 * Returns false if the connection is closed, errors, or bStopping is set.
	 */
	bool RecvExact(FSocket* Sock, uint8* Buffer, int32 NumBytes)
	{
		int32 BytesRead = 0;
		while (BytesRead < NumBytes)
		{
			if (Owner->bStopping) return false;

			const int32 Read = WorkerRecv(Sock, Buffer + BytesRead, NumBytes - BytesRead);
			if (Read < 0)
			{
				return false; // socket closed or error
			}
			if (Read == 0)
			{
				FPlatformProcess::Sleep(0.001f); // no data yet – yield briefly
				continue;
			}
			BytesRead += Read;
		}
		return true;
	}

	/** Dispatch a complete WebSocket message to the game thread. */
	void DispatchMessage(const TArray<uint8>& Payload, bool bIsBinary)
	{
		DispatchToGameThread(
			[WeakOwner = TWeakObjectPtr<USMLWebSocket>(Owner), Payload, bIsBinary]()
		{
			USMLWebSocket* O = WeakOwner.Get();
			if (!O) return;

			const int32 Num = Payload.Num();
			O->OnRawMessageReceived.Broadcast(Payload, Num, bIsBinary);

			if (!bIsBinary)
			{
				// Interpret the payload as UTF-8 text.
				// Add a null terminator for safe conversion.
				TArray<uint8> NullTerminated = Payload;
				NullTerminated.Add(0);
				FString Text = FString(
					UTF8_TO_TCHAR(reinterpret_cast<const ANSICHAR*>(NullTerminated.GetData())));
				O->OnMessageReceived.Broadcast(Text);
			}
		});
	}

	/** Dispatch a connection-error event and clean up. */
	void DispatchError(const FString& Error)
	{
		UE_LOG(LogSMLWebSocket, Error, TEXT("SMLWebSocket error: %s"), *Error);
		DispatchToGameThread(
			[WeakOwner = TWeakObjectPtr<USMLWebSocket>(Owner), Error]()
		{
			if (USMLWebSocket* O = WeakOwner.Get())
			{
				O->OnConnectionError.Broadcast(Error);
			}
		});
	}

	/** Schedule a lambda to run on the game thread. */
	template <typename TFunc>
	static void DispatchToGameThread(TFunc&& Func)
	{
		AsyncTask(ENamedThreads::GameThread, Forward<TFunc>(Func));
	}

	// -----------------------------------------------------------------------
	// Member data
	// -----------------------------------------------------------------------

	USMLWebSocket*         Owner;
	FString                Host;
	int32                  Port;
	FString                Path;
	TMap<FString, FString> ExtraHeaders;
	bool                   bIsSecure; // true for wss://, false for ws://
};

// ===========================================================================
// USMLWebSocket  –  implementation
// ===========================================================================

USMLWebSocket::USMLWebSocket()
	: bAutoReconnect(true)
	, ReconnectInitialDelaySeconds(2.f)
	, MaxReconnectAttempts(0)
	, bVerifyTlsCertificate(true)
	, Socket(nullptr)
	, Ssl(nullptr)
	, SslCtx(nullptr)
	, Worker(nullptr)
	, WorkerThread(nullptr)
	, bStopping(false)
	, bConnected(false)
{}

USMLWebSocket::~USMLWebSocket()
{
	TearDown();
}

void USMLWebSocket::BeginDestroy()
{
	TearDown();
	Super::BeginDestroy();
}

// ---------------------------------------------------------------------------

void USMLWebSocket::Connect(const FString& Url)
{
	ConnectWithHeaders(Url, {});
}

void USMLWebSocket::ConnectWithHeaders(const FString& Url,
	const TMap<FString, FString>& Headers)
{
	if (bConnected || WorkerThread)
	{
		UE_LOG(LogSMLWebSocket, Warning,
			TEXT("Connect() called while already connected or connecting. Call Close() first."));
		return;
	}

	FString Host, Path;
	int32   Port     = 80;
	bool    bIsSecure = false;
	if (!ParseWebSocketUrl(Url, Host, Port, Path, bIsSecure))
	{
		OnConnectionError.Broadcast(
			FString::Printf(TEXT("Invalid WebSocket URL: %s"), *Url));
		return;
	}

	bStopping  = false;
	bConnected = false;

	Worker = new FSMLWebSocketWorker(this, MoveTemp(Host), Port,
		MoveTemp(Path), Headers, bIsSecure);
	WorkerThread = FRunnableThread::Create(Worker, TEXT("SMLWebSocket"), 0,
		TPri_Normal);

	if (!WorkerThread)
	{
		delete Worker;
		Worker = nullptr;
		OnConnectionError.Broadcast(TEXT("Failed to create worker thread"));
	}
}

// ---------------------------------------------------------------------------

void USMLWebSocket::Send(const FString& Data)
{
	if (!bConnected)
	{
		UE_LOG(LogSMLWebSocket, Warning, TEXT("Send() called when not connected"));
		return;
	}

	FTCHARToUTF8 UTF8(*Data);
	SendFrame(EWSOpcode::Text,
		reinterpret_cast<const uint8*>(UTF8.Get()),
		static_cast<uint64>(UTF8.Length()));
}

void USMLWebSocket::SendBinary(const TArray<uint8>& Data)
{
	if (!bConnected)
	{
		UE_LOG(LogSMLWebSocket, Warning, TEXT("SendBinary() called when not connected"));
		return;
	}

	SendFrame(EWSOpcode::Binary, Data.GetData(), static_cast<uint64>(Data.Num()));
}

// ---------------------------------------------------------------------------

void USMLWebSocket::Close(int32 Code, const FString& Reason)
{
	if (!bConnected)
	{
		return;
	}

	// Build close frame payload: 2-byte status code + UTF-8 reason (max 123 bytes).
	FTCHARToUTF8 ReasonUTF8(*Reason);
	const int32  ReasonLen = FMath::Min(ReasonUTF8.Length(), 123);

	TArray<uint8> ClosePayload;
	ClosePayload.SetNumUninitialized(2 + ReasonLen);
	ClosePayload[0] = static_cast<uint8>((Code >> 8) & 0xFF);
	ClosePayload[1] = static_cast<uint8>(Code & 0xFF);
	FMemory::Memcpy(ClosePayload.GetData() + 2, ReasonUTF8.Get(), ReasonLen);

	SendFrame(EWSOpcode::Close, ClosePayload.GetData(),
		static_cast<uint64>(ClosePayload.Num()));

	// Signal the read loop to stop.
	bStopping  = true;
	bConnected = false;

	FScopeLock Lock(&SendLock);
	if (Socket)
	{
		Socket->Close(); // Interrupts any blocking Recv() in the worker thread.
	}
}

// ---------------------------------------------------------------------------

bool USMLWebSocket::IsConnected() const
{
	return bConnected;
}

// ---------------------------------------------------------------------------

bool USMLWebSocket::SendFrame(uint8 Opcode, const uint8* Payload, uint64 PayloadLength)
{
	FScopeLock Lock(&SendLock);

	if (!Socket)
	{
		return false;
	}

	// ---- Build frame header ----
	// Client frames MUST be masked (RFC 6455 §5.3).
	TArray<uint8> Frame;

	// Byte 0: FIN=1, RSV=0, Opcode
	Frame.Add(0x80 | (Opcode & 0x0F));

	// Byte 1: MASK=1, Payload length field
	if (PayloadLength < 126)
	{
		Frame.Add(0x80 | static_cast<uint8>(PayloadLength));
	}
	else if (PayloadLength <= 0xFFFF)
	{
		Frame.Add(0x80 | 126);
		Frame.Add(static_cast<uint8>(PayloadLength >> 8));
		Frame.Add(static_cast<uint8>(PayloadLength & 0xFF));
	}
	else
	{
		Frame.Add(0x80 | 127);
		for (int32 i = 7; i >= 0; --i)
		{
			Frame.Add(static_cast<uint8>((PayloadLength >> (i * 8)) & 0xFF));
		}
	}

	// 4-byte masking key (must be unpredictable per RFC 6455 §5.3)
	uint8 MaskKey[4];
	for (uint8& B : MaskKey)
	{
		B = static_cast<uint8>(FMath::RandRange(0, 255));
	}
	Frame.Add(MaskKey[0]);
	Frame.Add(MaskKey[1]);
	Frame.Add(MaskKey[2]);
	Frame.Add(MaskKey[3]);

	// Masked payload
	for (uint64 i = 0; i < PayloadLength; ++i)
	{
		Frame.Add(Payload[i] ^ MaskKey[i & 3]);
	}

	// ---- Send through TLS or plain TCP ----
	if (Ssl)
	{
		// SSL_write sends all bytes or returns <=0 on error (unlike TCP send).
		const int Written = SSL_write(Ssl, Frame.GetData(), Frame.Num());
		if (Written != Frame.Num())
		{
			UE_LOG(LogSMLWebSocket, Error,
				TEXT("SSL_write failed (wrote %d of %d bytes)"),
				Written, Frame.Num());
			return false;
		}
		return true;
	}

	int32 BytesSent = 0;
	const bool bOk = Socket->Send(Frame.GetData(), Frame.Num(), BytesSent);
	if (!bOk || BytesSent != Frame.Num())
	{
		UE_LOG(LogSMLWebSocket, Error,
			TEXT("Failed to send WebSocket frame (sent %d of %d bytes)"),
			BytesSent, Frame.Num());
		return false;
	}
	return true;
}

// ---------------------------------------------------------------------------

void USMLWebSocket::TearDown()
{
	// Signal the worker to stop and interrupt any blocking socket call.
	bStopping  = true;
	bConnected = false;

	{
		FScopeLock Lock(&SendLock);
		if (Socket)
		{
			Socket->Close();
		}
	}

	if (WorkerThread)
	{
		WorkerThread->Kill(true); // Wait for the thread to finish.
		delete WorkerThread;
		WorkerThread = nullptr;
	}

	if (Worker)
	{
		delete Worker;
		Worker = nullptr;
	}

	// Free SSL objects AFTER the worker thread has exited so there is no
	// risk of SSL_read/SSL_write racing with SSL_free.
	// The worker's TearDownTls() will have already nulled these if it ran
	// the normal exit path; these guards handle the Kill() case.
	{
		FScopeLock Lock(&SendLock);
		if (Ssl)    { SSL_free(Ssl);    Ssl    = nullptr; }
		if (SslCtx) { SSL_CTX_free(SslCtx); SslCtx = nullptr; }
	}

	// Destroy the socket after the worker thread has exited.
	if (Socket)
	{
		ISocketSubsystem* SS = ISocketSubsystem::Get(NAME_None);
		if (SS)
		{
			SS->DestroySocket(Socket);
		}
		Socket = nullptr;
	}
}

// ===========================================================================
// USMLWebSocketBlueprintLibrary
// ===========================================================================

USMLWebSocket* USMLWebSocketBlueprintLibrary::CreateWebSocket(UObject* WorldContextObject)
{
	return NewObject<USMLWebSocket>(WorldContextObject);
}
