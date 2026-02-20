// Copyright SatisfactoryDiscordbot. All Rights Reserved.

#include "Networking/CustomWebSocketServer.h"
#include "Networking/WSServerThread.h"
#include "Networking/WSClientConnection.h"
#include "HAL/RunnableThread.h"
#include "SatisfactoryDiscordBotModule.h"

// ---------------------------------------------------------------------------
// Blueprint API
// ---------------------------------------------------------------------------

bool UCustomWebSocketServer::StartListening(int32 Port)
{
	if (IsListening())
	{
		UE_LOG(LogSatisfactoryDiscordBot, Warning,
			TEXT("UCustomWebSocketServer: already listening; call StopListening first"));
		return false;
	}

	if (Port <= 0 || Port >= 65536)
	{
		UE_LOG(LogSatisfactoryDiscordBot, Error,
			TEXT("UCustomWebSocketServer: invalid port %d"), Port);
		return false;
	}

	ServerRunnable = MakeUnique<FWSServerThread>(Port);
	ServerThread   = FRunnableThread::Create(
		ServerRunnable.Get(),
		TEXT("SatisfactoryDiscordBot_WSServer"),
		0,
		TPri_Normal);

	if (!ServerThread)
	{
		UE_LOG(LogSatisfactoryDiscordBot, Error,
			TEXT("UCustomWebSocketServer: failed to create server thread"));
		ServerRunnable.Reset();
		return false;
	}

	// Register a per-frame ticker to dispatch events on the game thread.
	TickHandle = FTSTicker::GetCoreTicker().AddTicker(
		FTickerDelegate::CreateUObject(this, &UCustomWebSocketServer::Tick));

	UE_LOG(LogSatisfactoryDiscordBot, Log,
		TEXT("UCustomWebSocketServer: listening on port %d"), Port);
	return true;
}

void UCustomWebSocketServer::StopListening()
{
	ShutdownInternal();
}

bool UCustomWebSocketServer::IsListening() const
{
	return ServerRunnable.IsValid() && ServerRunnable->IsRunning();
}

// ---------------------------------------------------------------------------
// UObject
// ---------------------------------------------------------------------------

void UCustomWebSocketServer::BeginDestroy()
{
	ShutdownInternal();
	Super::BeginDestroy();
}

// ---------------------------------------------------------------------------
// Internal
// ---------------------------------------------------------------------------

bool UCustomWebSocketServer::Tick(float /*DeltaTime*/)
{
	if (!ServerRunnable.IsValid())
	{
		return false; // Server stopped; remove this ticker.
	}

	// Pick up newly accepted connections.
	TSharedPtr<FWSClientConnection> NewConn;
	while ((NewConn = ServerRunnable->TryGetNewConnection()).IsValid())
	{
		UCustomWebSocketConnection* ConnObj =
			NewObject<UCustomWebSocketConnection>(this);
		ConnObj->InitWithInternalConnection(NewConn);
		ActiveConnections.Add(ConnObj);

		// Fire delegate AFTER we've stored the connection so the user can
		// immediately call SendText / etc. from the handler.
		OnClientConnected.Broadcast(ConnObj);
	}

	// Process messages on all active connections; remove closed ones.
	for (int32 i = ActiveConnections.Num() - 1; i >= 0; --i)
	{
		UCustomWebSocketConnection* Conn = ActiveConnections[i];
		if (!Conn || !Conn->ProcessPendingMessages())
		{
			ActiveConnections.RemoveAt(i, 1, /*bAllowShrinking=*/false);
		}
	}

	return true; // Keep ticking.
}

void UCustomWebSocketServer::ShutdownInternal()
{
	if (TickHandle.IsValid())
	{
		FTSTicker::GetCoreTicker().RemoveTicker(TickHandle);
		TickHandle.Reset();
	}

	if (ServerRunnable.IsValid())
	{
		ServerRunnable->Stop();
	}

	if (ServerThread)
	{
		ServerThread->Kill(/*bShouldWait=*/true);
		delete ServerThread;
		ServerThread = nullptr;
	}

	ServerRunnable.Reset();
	ActiveConnections.Empty();
}
