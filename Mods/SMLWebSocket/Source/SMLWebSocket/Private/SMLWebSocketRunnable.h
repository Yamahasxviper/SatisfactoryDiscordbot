// Copyright Coffee Stain Studios. All Rights Reserved.

#pragma once

#include "HAL/Runnable.h"
#include "HAL/RunnableThread.h"
#include "Containers/Queue.h"
#include "Templates/SharedPointer.h"
#include "Templates/Atomic.h"

// Forward-declare OpenSSL types so consumers don't need to include openssl headers.
struct ssl_ctx_st;
struct ssl_st;
struct bio_st;
class FSocket;
class USMLWebSocketClient;

// ── Internal message types ────────────────────────────────────────────────────

struct FSMLWebSocketOutboundMessage
{
	bool bIsBinary{false};
	TArray<uint8> Payload; // UTF-8 bytes for text, raw bytes for binary
};

struct FSMLWebSocketCloseRequest
{
	int32 Code{1000};
	FString Reason;
};

// ── State machine ─────────────────────────────────────────────────────────────

enum class ESMLWebSocketRunnableState : uint8
{
	Idle,
	ResolvingHost,
	Connecting,
	SslHandshake,
	SendingHttpUpgrade,
	ReadingHttpUpgradeResponse,
	Connected,
	Closing,
	Closed
};

/**
 * Background thread that manages the raw TCP (+ optional SSL) socket and the
 * WebSocket protocol (RFC 6455) for USMLWebSocketClient.
 *
 * All public game-thread callbacks are dispatched via AsyncTask so that
 * delegates always fire on the game thread.
 */
class FSMLWebSocketRunnable final : public FRunnable, public TSharedFromThis<FSMLWebSocketRunnable>
{
public:
	FSMLWebSocketRunnable(USMLWebSocketClient* InOwner,
	                      const FString& InUrl,
	                      const TArray<FString>& InProtocols,
	                      const TMap<FString, FString>& InExtraHeaders);

	virtual ~FSMLWebSocketRunnable() override;

	// ── FRunnable ─────────────────────────────────────────────────────────────

	virtual bool Init() override;
	virtual uint32 Run() override;
	virtual void Stop() override;
	virtual void Exit() override;

	// ── Thread-safe API (called from the game thread) ─────────────────────────

	/** Queue a text message to be sent. */
	void EnqueueText(const FString& Text);

	/** Queue a binary message to be sent. */
	void EnqueueBinary(const TArray<uint8>& Data);

	/** Request a graceful WebSocket close. */
	void EnqueueClose(int32 Code, const FString& Reason);

	/** True once the WebSocket handshake has been completed. */
	bool IsConnected() const;

private:
	// ── Connection setup ──────────────────────────────────────────────────────

	bool ResolveAndConnect(const FString& Host, int32 Port);
	bool PerformSslHandshake(const FString& Host);
	bool SendHttpUpgradeRequest(const FString& Host, int32 Port, const FString& Path, const FString& Key);
	bool ReadHttpUpgradeResponse(const FString& ExpectedAcceptKey);

	// ── OpenSSL helpers ───────────────────────────────────────────────────────

	bool InitSslContext();
	void DestroySsl();

	/** Read decrypted bytes through SSL (handles re-keying / WANT_READ internally). */
	int32 SslRead(uint8* Buffer, int32 BufferSize);

	/** Encrypt and write bytes through SSL. */
	bool SslWrite(const uint8* Data, int32 DataSize);

	/** Drain the SSL write-BIO into the TCP socket. */
	bool FlushSslWriteBio();

	/** Feed available TCP bytes into the SSL read-BIO. */
	bool FeedSslReadBio();

	// ── Raw socket helpers ────────────────────────────────────────────────────

	/** Send all bytes to the TCP socket (blocking). */
	bool RawSend(const uint8* Data, int32 DataSize);

	/** Receive exactly BytesRequired bytes from TCP (blocking, timeout aware). */
	bool RawRecvExact(uint8* Buffer, int32 BytesRequired);

	/** Receive at most BufferSize bytes; returns the number actually read (<= 0 on error). */
	int32 RawRecvAvailable(uint8* Buffer, int32 BufferSize);

	// ── High-level send/recv (routes through SSL when bUseSsl is true) ────────

	bool NetSend(const uint8* Data, int32 DataSize);
	int32 NetRecv(uint8* Buffer, int32 BufferSize);
	bool NetRecvExact(uint8* Buffer, int32 BytesRequired);

	// ── WebSocket framing (RFC 6455) ──────────────────────────────────────────

	/** Build and send a WebSocket frame (client-to-server, always masked). */
	bool SendWsFrame(uint8 Opcode, const uint8* Data, int32 DataSize, bool bFinal = true);

	/** Read and process the next incoming WebSocket frame. Returns false on fatal error. */
	bool ProcessIncomingFrame();

	/** Send a Pong frame in response to a Ping. */
	void SendPong(const TArray<uint8>& Payload);

	/** Drain the outbound message queue and send all pending frames. */
	void FlushOutboundQueue();

	// ── WebSocket handshake key helpers ───────────────────────────────────────

	/** Generate a random 16-byte key and Base64-encode it (Sec-WebSocket-Key). */
	static FString GenerateWebSocketKey();

	/** Compute the expected Sec-WebSocket-Accept value for a given key (RFC 6455 §4.2.2). */
	static FString ComputeAcceptKey(const FString& ClientKey);

	// ── Game-thread notifications ─────────────────────────────────────────────

	void NotifyConnected();
	void NotifyMessage(const FString& Message);
	void NotifyBinaryMessage(const TArray<uint8>& Data, bool bIsFinal);
	void NotifyClosed(int32 Code, const FString& Reason);
	void NotifyError(const FString& Error);

	// ── Fields ────────────────────────────────────────────────────────────────

	TWeakObjectPtr<USMLWebSocketClient> Owner;

	// URL components parsed during Init()
	FString ParsedHost;
	FString ParsedPath;
	int32   ParsedPort{80};
	bool    bUseSsl{false};

	TArray<FString>        Protocols;
	TMap<FString, FString> ExtraHeaders;

	// Unreal TCP socket (blocking mode)
	FSocket* Socket{nullptr};

	// OpenSSL objects (only valid when bUseSsl == true)
	ssl_ctx_st* SslCtx{nullptr};
	ssl_st*     SslInstance{nullptr};
	bio_st*     ReadBio{nullptr};  // network → SSL
	bio_st*     WriteBio{nullptr}; // SSL → network

	// Shared state between game thread and I/O thread
	TAtomic<ESMLWebSocketRunnableState> State{ESMLWebSocketRunnableState::Idle};
	FThreadSafeBool bStopRequested{false};
	FThreadSafeBool bConnected{false};

	// Outbound queues (game thread → I/O thread)
	TQueue<FSMLWebSocketOutboundMessage, EQueueMode::Mpsc> OutboundMessages;
	TQueue<FSMLWebSocketCloseRequest,    EQueueMode::Mpsc> CloseRequests;

	// Reassembly buffer for fragmented WebSocket messages
	TArray<uint8> FragmentBuffer;
	bool          bFragmentIsBinary{false};
};
