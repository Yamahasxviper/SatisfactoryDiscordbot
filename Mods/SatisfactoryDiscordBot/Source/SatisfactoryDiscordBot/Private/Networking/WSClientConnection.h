// Copyright SatisfactoryDiscordbot. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "HAL/CriticalSection.h"
#include "Containers/Queue.h"

class FSocket;
class ISocketSubsystem;

// Forward-declare OpenSSL types so the header does not pull in OpenSSL headers.
struct ssl_st;
struct ssl_ctx_st;

/** A single received WebSocket message (text, binary, or close notification). */
struct FWSMessage
{
	/** True if this is a text (UTF-8) frame; false means binary. */
	bool bIsText{false};

	/** True if this entry represents a connection-close event, not a data frame. */
	bool bIsClosed{false};

	/** WebSocket status code when bIsClosed is true (1000 = normal, 1006 = abnormal). */
	int32 CloseStatusCode{1000};

	/** Human-readable close reason when bIsClosed is true. */
	FString CloseReason;

	/** Decoded text payload (only valid when bIsText && !bIsClosed). */
	FString TextData;

	/** Raw binary payload (only valid when !bIsText && !bIsClosed). */
	TArray<uint8> BinaryData;
};

/**
 * Represents a single connected WebSocket client.
 *
 * Threading model:
 *   - PerformHandshake() and ReadPendingData() are called from the server background thread.
 *   - SendText(), SendBinary(), SendClose() may be called from any thread (protected by SendMutex).
 *   - TryGetNextMessage() / HasPendingMessages() / IsConnected() may be called from any thread.
 */
class FWSClientConnection : public TSharedFromThis<FWSClientConnection>
{
public:
	FWSClientConnection(FSocket* InSocket, ISocketSubsystem* InSocketSubsystem, const FString& InRemoteAddress,
		ssl_ctx_st* InSslContext = nullptr);
	~FWSClientConnection();

	// -------------------------------------------------------------------------
	// Server-thread API
	// -------------------------------------------------------------------------

	/** Performs the HTTP → WebSocket upgrade handshake. Returns true on success. */
	bool PerformHandshake();

	/** Non-blocking read of any available data; parses complete frames. */
	void ReadPendingData();

	// -------------------------------------------------------------------------
	// Thread-safe write API (may be called from game thread)
	// -------------------------------------------------------------------------

	/** Send a UTF-8 text frame. Returns false if not connected. */
	bool SendText(const FString& Message);

	/** Send a binary frame. Returns false if not connected. */
	bool SendBinary(const TArray<uint8>& Data);

	/**
	 * Send a WebSocket close frame and mark the connection as closed.
	 * @param StatusCode  RFC 6455 status code (default 1000 = normal closure).
	 * @param Reason      Optional human-readable reason string.
	 */
	bool SendClose(uint16 StatusCode = 1000, const FString& Reason = TEXT("Normal closure"));

	// -------------------------------------------------------------------------
	// Thread-safe state / message querying
	// -------------------------------------------------------------------------

	/** Returns true if there are unprocessed messages in the queue. */
	bool HasPendingMessages() const;

	/** Dequeues the next message. Returns false if the queue is empty. */
	bool TryGetNextMessage(FWSMessage& OutMessage);

	/** Returns true while the underlying TCP connection is alive. */
	bool IsConnected() const;

	/** Human-readable remote IP:port string. */
	FString GetRemoteAddress() const { return RemoteAddress; }

private:
	// -------------------------------------------------------------------------
	// Handshake helpers
	// -------------------------------------------------------------------------

	/** Reads the HTTP Upgrade request from the socket. Fills OutKey with the Sec-WebSocket-Key value. */
	bool ReadHandshakeRequest(FString& OutKey);

	/** Sends the HTTP 101 response with the computed Sec-WebSocket-Accept header. */
	bool SendHandshakeResponse(const FString& AcceptKey);

	/** RFC 6455 §4.2.2: Base64( SHA-1( key + magic ) ). */
	static FString ComputeAcceptKey(const FString& WebSocketKey);

	// -------------------------------------------------------------------------
	// TLS helpers
	// -------------------------------------------------------------------------

	/**
	 * Performs the TLS handshake using the provided SSL context.
	 * Must be called on the server thread before PerformHandshake().
	 * Returns true on success; on failure SslInstance is left null.
	 */
	bool InitTLS(ssl_ctx_st* InSslContext);

	/**
	 * Abstracted byte-level receive: uses SSL_read when TLS is active,
	 * falls back to Socket->Recv otherwise.
	 */
	bool RecvData(uint8* Buf, int32 MaxLen, int32& OutBytesRead);

	// -------------------------------------------------------------------------
	// Frame parsing
	// -------------------------------------------------------------------------

	/**
	 * Attempts to parse one complete WebSocket frame from ReceiveBuffer.
	 * Returns true if a frame was consumed (caller should loop).
	 */
	bool TryParseFrame();

	/** Processes a fully-parsed frame (handles control frames inline). */
	void ProcessFrame(uint8 Opcode, bool bFinalFrame, const TArray<uint8>& Payload);

	// -------------------------------------------------------------------------
	// Low-level write helpers (must be called with SendMutex held, except during handshake)
	// -------------------------------------------------------------------------

	/** Builds and sends a single WebSocket frame. */
	bool SendFrame(uint8 Opcode, const uint8* Payload, int32 PayloadLen);

	/** Sends all bytes; loops until everything is written or an error occurs. */
	bool SendRawBlocking(const uint8* Data, int32 DataLen);

	// -------------------------------------------------------------------------
	// Members
	// -------------------------------------------------------------------------

	FSocket*          Socket;
	ISocketSubsystem* SocketSubsystem;
	FString           RemoteAddress;

	/** SSL context supplied at construction time; nullptr when TLS is disabled. */
	ssl_ctx_st* PendingSslContext{nullptr};

	/** Live SSL object for this connection; nullptr when TLS is not active. */
	ssl_st* SslInstance{nullptr};

	/** Set to false when the TCP connection is lost or when a close frame is exchanged. */
	TAtomic<bool> bConnected{false};

	/** Incoming byte accumulation buffer – owned exclusively by the server thread. */
	TArray<uint8> ReceiveBuffer;

	/** Fragmented-message assembly – server thread only. */
	uint8        FragmentOpcode{0x0};
	TArray<uint8> FragmentBuffer;
	bool          bInFragment{false};

	/**
	 * Received messages queued for consumption on the game thread.
	 * MPSC queue: multiple potential producers (pong etc.), single consumer (game thread ticker).
	 */
	TQueue<FWSMessage, EQueueMode::Mpsc> IncomingMessageQueue;

	/** Protects all socket writes so game thread and server thread do not interleave frames. */
	FCriticalSection SendMutex;
};
