// Copyright SatisfactoryDiscordbot. All Rights Reserved.

#include "Networking/CustomWebSocketServer.h"
#include "Networking/WSServerThread.h"
#include "Networking/WSClientConnection.h"
#include "HAL/RunnableThread.h"
#include "SatisfactoryDiscordBotModule.h"

// OpenSSL – only required for wss:// (StartListeningSSL).
#if PLATFORM_SUPPORTS_OPENSSL
#include "openssl/ssl.h"
#include "openssl/err.h"
#endif

// ---------------------------------------------------------------------------
// Blueprint API
// ---------------------------------------------------------------------------

bool UCustomWebSocketServer::StartListening(int32 Port)
{
	return StartListeningInternal(Port, nullptr);
}

bool UCustomWebSocketServer::StartListeningSSL(
	int32          Port,
	const FString& CertificatePath,
	const FString& PrivateKeyPath)
{
#if PLATFORM_SUPPORTS_OPENSSL
	// Create a server-side TLS context backed by the OpenSSL library that is
	// bundled with Unreal Engine.  This does not depend on the Unreal WebSockets
	// module or any Satisfactory / CSS custom-engine feature.
	ssl_ctx_st* NewCtx = SSL_CTX_new(TLS_server_method());
	if (!NewCtx)
	{
		UE_LOG(LogSatisfactoryDiscordBot, Error,
			TEXT("UCustomWebSocketServer::StartListeningSSL: SSL_CTX_new failed"));
		return false;
	}

	// Enforce TLS 1.2+ by disabling older protocol versions.
	SSL_CTX_set_options(NewCtx,
		SSL_OP_NO_SSLv2 | SSL_OP_NO_SSLv3 |
		SSL_OP_NO_TLSv1 | SSL_OP_NO_TLSv1_1);

	// Load the certificate (PEM, may include intermediate chain).
	if (SSL_CTX_use_certificate_file(
	        NewCtx, TCHAR_TO_UTF8(*CertificatePath), SSL_FILETYPE_PEM) != 1)
	{
		UE_LOG(LogSatisfactoryDiscordBot, Error,
			TEXT("UCustomWebSocketServer::StartListeningSSL: "
			     "failed to load certificate '%s'"), *CertificatePath);
		SSL_CTX_free(NewCtx);
		return false;
	}

	// Load the private key (PEM).
	if (SSL_CTX_use_PrivateKey_file(
	        NewCtx, TCHAR_TO_UTF8(*PrivateKeyPath), SSL_FILETYPE_PEM) != 1)
	{
		UE_LOG(LogSatisfactoryDiscordBot, Error,
			TEXT("UCustomWebSocketServer::StartListeningSSL: "
			     "failed to load private key '%s'"), *PrivateKeyPath);
		SSL_CTX_free(NewCtx);
		return false;
	}

	// Verify that the certificate and private key form a matching pair.
	if (SSL_CTX_check_private_key(NewCtx) != 1)
	{
		UE_LOG(LogSatisfactoryDiscordBot, Error,
			TEXT("UCustomWebSocketServer::StartListeningSSL: "
			     "certificate and private key do not match"));
		SSL_CTX_free(NewCtx);
		return false;
	}

	// Ownership of NewCtx transfers to this object on success; freed in ShutdownInternal.
	if (!StartListeningInternal(Port, NewCtx))
	{
		// StartListeningInternal already logged the error; discard the new context.
		SSL_CTX_free(NewCtx);
		return false;
	}

	SslContext = NewCtx;

	UE_LOG(LogSatisfactoryDiscordBot, Log,
		TEXT("UCustomWebSocketServer: WSS (TLS) server listening on port %d"), Port);
	return true;
#else
	UE_LOG(LogSatisfactoryDiscordBot, Warning,
		TEXT("UCustomWebSocketServer::StartListeningSSL: "
		     "SSL not supported on this platform; falling back to plain ws://"));
	return StartListeningInternal(Port, nullptr);
#endif
}

bool UCustomWebSocketServer::StartListeningInternal(int32 Port, ssl_ctx_st* InSslContext)
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

	ServerRunnable = MakeUnique<FWSServerThread>(Port, InSslContext);
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

	// Free the SSL context after the thread has exited and all FWSClientConnection
	// objects owned by the server thread have been destroyed (their SSL* handles
	// hold references to this context).  OpenSSL uses reference counting, so any
	// SSL* objects still held by game-thread UCustomWebSocketConnection UObjects
	// keep the context alive until they are garbage-collected.
#if PLATFORM_SUPPORTS_OPENSSL
	if (SslContext)
	{
		SSL_CTX_free(SslContext);
		SslContext = nullptr;
	}
#endif
}
