// Copyright SatisfactoryDiscordbot. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "HAL/Runnable.h"
#include "Containers/Queue.h"

class FWSClientConnection;
class FSocket;
class ISocketSubsystem;

// Forward declaration – avoids propagating OpenSSL headers.
struct ssl_ctx_st;

/**
 * Background thread that accepts incoming TCP connections and polls each active
 * WebSocket connection for new data.
 *
 * The main loop runs at approximately 1 ms granularity so it is responsive
 * without burning a CPU core.
 *
 * New connections are placed in a lock-free queue so the game-thread ticker
 * (UCustomWebSocketServer) can pick them up safely.
 *
 * When an ssl_ctx_st* context is supplied via the constructor, each accepted
 * connection is wrapped in TLS before the WebSocket handshake so the server
 * operates as a wss:// endpoint.
 */
class FWSServerThread : public FRunnable
{
public:
	/**
	 * @param InPort        TCP port to listen on.
	 * @param InSslContext  Optional OpenSSL SSL_CTX for WSS.  Pass nullptr for
	 *                      plain ws://.  The caller retains ownership; the
	 *                      context must remain valid until this thread exits.
	 */
	explicit FWSServerThread(int32 InPort, ssl_ctx_st* InSslContext = nullptr);
	virtual ~FWSServerThread() override;

	// FRunnable interface
	virtual bool   Init() override;
	virtual uint32 Run()  override;
	virtual void   Stop() override;

	/**
	 * Called from the game thread.
	 * Dequeues one newly accepted (and handshaked) client connection.
	 * Returns an invalid TSharedPtr when there are no new connections.
	 */
	TSharedPtr<FWSClientConnection> TryGetNewConnection();

	/** Returns true if the listen socket is open and the thread is running. */
	bool IsRunning() const;

private:
	int32             Port;
	FSocket*          ListenSocket{nullptr};
	ISocketSubsystem* SocketSubsystem{nullptr};

	/** When non-null, newly accepted connections are wrapped in TLS. */
	ssl_ctx_st*       SslContext{nullptr};

	TAtomic<bool> bShouldRun{false};

	/** Connections owned by the server thread (accept + read). */
	TArray<TSharedPtr<FWSClientConnection>> ActiveConnections;

	/**
	 * MPSC queue bridging accepted connections from the server thread to the
	 * game thread. The server thread enqueues; the game thread dequeues.
	 */
	TQueue<TSharedPtr<FWSClientConnection>, EQueueMode::Mpsc> NewConnectionQueue;
};
