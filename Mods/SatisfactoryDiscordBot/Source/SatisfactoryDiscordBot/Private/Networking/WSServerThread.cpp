// Copyright SatisfactoryDiscordbot. All Rights Reserved.

#include "Networking/WSServerThread.h"
#include "Networking/WSClientConnection.h"

#include "Sockets.h"
#include "SocketSubsystem.h"

DEFINE_LOG_CATEGORY_STATIC(LogWSServerThread, Log, All);

// ---------------------------------------------------------------------------

FWSServerThread::FWSServerThread(int32 InPort, ssl_ctx_st* InSslContext)
	: Port(InPort)
	, SslContext(InSslContext)
{
}

FWSServerThread::~FWSServerThread()
{
	// Clean up the listen socket if Init() created it but Run() never ran
	// (or if Stop() + thread join happened already).
	if (ListenSocket && SocketSubsystem)
	{
		SocketSubsystem->DestroySocket(ListenSocket);
		ListenSocket = nullptr;
	}
}

// ---------------------------------------------------------------------------
// FRunnable interface
// ---------------------------------------------------------------------------

bool FWSServerThread::Init()
{
	SocketSubsystem = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM);
	if (!SocketSubsystem)
	{
		UE_LOG(LogWSServerThread, Error, TEXT("Could not get socket subsystem"));
		return false;
	}

	// Create a TCP listen socket.
	ListenSocket = SocketSubsystem->CreateSocket(NAME_Stream, TEXT("CustomWebSocketServer"), false);
	if (!ListenSocket)
	{
		UE_LOG(LogWSServerThread, Error, TEXT("Failed to create listen socket"));
		return false;
	}

	ListenSocket->SetReuseAddr(true);
	ListenSocket->SetNonBlocking(false); // blocking; we use WaitForPendingConnection

	// Bind to all interfaces on the requested port.
	TSharedRef<FInternetAddr> BindAddr = SocketSubsystem->CreateInternetAddr();
	BindAddr->SetAnyAddress();
	BindAddr->SetPort(Port);

	if (!ListenSocket->Bind(*BindAddr))
	{
		UE_LOG(LogWSServerThread, Error,
			TEXT("Failed to bind WebSocket listen socket to port %d"), Port);
		SocketSubsystem->DestroySocket(ListenSocket);
		ListenSocket = nullptr;
		return false;
	}

	if (!ListenSocket->Listen(8))
	{
		UE_LOG(LogWSServerThread, Error,
			TEXT("Failed to listen on WebSocket port %d"), Port);
		SocketSubsystem->DestroySocket(ListenSocket);
		ListenSocket = nullptr;
		return false;
	}

	bShouldRun.Store(true);
	UE_LOG(LogWSServerThread, Log, TEXT("WebSocket server listening on port %d"), Port);
	return true;
}

uint32 FWSServerThread::Run()
{
	while (bShouldRun.Load())
	{
		// ----------------------------------------------------------------
		// Accept new connections (non-blocking, 1 ms timeout).
		// ----------------------------------------------------------------
		bool bHasPending = false;
		if (ListenSocket->WaitForPendingConnection(bHasPending, FTimespan::FromMilliseconds(1)))
		{
			if (bHasPending)
			{
				TSharedRef<FInternetAddr> ClientAddr =
					SocketSubsystem->CreateInternetAddr();

				FSocket* ClientSocket = ListenSocket->Accept(*ClientAddr, TEXT("WSClient"));
				if (ClientSocket)
				{
					const FString RemoteAddr = ClientAddr->ToString(true);
					UE_LOG(LogWSServerThread, Log,
						TEXT("Accepted TCP connection from %s"), *RemoteAddr);

					auto Conn = MakeShared<FWSClientConnection>(
						ClientSocket, SocketSubsystem, RemoteAddr);

					// When a TLS context is set, wrap the connection in SSL
					// before running the WebSocket handshake.
					if (SslContext && !Conn->InitSSL(SslContext))
					{
						UE_LOG(LogWSServerThread, Warning,
							TEXT("SSL init failed for %s; dropping"), *RemoteAddr);
					}
					else if (Conn->PerformHandshake())
					{
						ActiveConnections.Add(Conn);
						NewConnectionQueue.Enqueue(Conn);
					}
					else
					{
						UE_LOG(LogWSServerThread, Warning,
							TEXT("WebSocket handshake failed for %s; dropping"), *RemoteAddr);
					}
				}
			}
		}

		// ----------------------------------------------------------------
		// Poll active connections for incoming data.
		// ----------------------------------------------------------------
		for (int32 i = ActiveConnections.Num() - 1; i >= 0; --i)
		{
			FWSClientConnection& Conn = *ActiveConnections[i];
			Conn.ReadPendingData();

			if (!Conn.IsConnected())
			{
				UE_LOG(LogWSServerThread, Log,
					TEXT("Removing disconnected client %s"), *Conn.GetRemoteAddress());
				ActiveConnections.RemoveAt(i, 1, /*bAllowShrinking=*/false);
			}
		}
	}

	// ----------------------------------------------------------------
	// Shutdown: send close frames to all remaining clients.
	// ----------------------------------------------------------------
	for (TSharedPtr<FWSClientConnection>& Conn : ActiveConnections)
	{
		if (Conn.IsValid() && Conn->IsConnected())
		{
			Conn->SendClose(1001, TEXT("Server going away"));
		}
	}
	ActiveConnections.Empty();

	if (ListenSocket)
	{
		SocketSubsystem->DestroySocket(ListenSocket);
		ListenSocket = nullptr;
	}

	UE_LOG(LogWSServerThread, Log, TEXT("WebSocket server thread stopped"));
	return 0;
}

void FWSServerThread::Stop()
{
	bShouldRun.Store(false);
}

// ---------------------------------------------------------------------------
// Game-thread API
// ---------------------------------------------------------------------------

TSharedPtr<FWSClientConnection> FWSServerThread::TryGetNewConnection()
{
	TSharedPtr<FWSClientConnection> Out;
	NewConnectionQueue.Dequeue(Out);
	return Out;
}

bool FWSServerThread::IsRunning() const
{
	return bShouldRun.Load();
}
