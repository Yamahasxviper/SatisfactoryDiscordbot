// Copyright SatisfactoryDiscordbot. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "HAL/Runnable.h"
#include "Containers/Queue.h"

class FWSClientConnection;
class FSocket;
class ISocketSubsystem;

/**
 * Background thread that accepts incoming TCP connections and polls each active
 * WebSocket connection for new data.
 *
 * The main loop runs at approximately 1 ms granularity so it is responsive
 * without burning a CPU core.
 *
 * New connections are placed in a lock-free queue so the game-thread ticker
 * (UCustomWebSocketServer) can pick them up safely.
 */
class FWSServerThread : public FRunnable
{
public:
	explicit FWSServerThread(int32 InPort);
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

	TAtomic<bool> bShouldRun{false};

	/** Connections owned by the server thread (accept + read). */
	TArray<TSharedPtr<FWSClientConnection>> ActiveConnections;

	/**
	 * MPSC queue bridging accepted connections from the server thread to the
	 * game thread. The server thread enqueues; the game thread dequeues.
	 */
	TQueue<TSharedPtr<FWSClientConnection>, EQueueMode::Mpsc> NewConnectionQueue;
};
