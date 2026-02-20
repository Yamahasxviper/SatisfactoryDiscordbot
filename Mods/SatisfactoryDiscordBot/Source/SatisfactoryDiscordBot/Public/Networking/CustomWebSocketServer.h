// Copyright SatisfactoryDiscordbot. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "HAL/Ticker.h"
#include "CustomWebSocketConnection.h"
#include "CustomWebSocketServer.generated.h"

class FWSServerThread;
class FRunnableThread;

// Forward declaration – avoids propagating OpenSSL headers.
struct ssl_ctx_st;

// ---------------------------------------------------------------------------
// Delegates
// ---------------------------------------------------------------------------

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnWSClientConnected,
	UCustomWebSocketConnection*, Connection);

// ---------------------------------------------------------------------------

/**
 * Custom WebSocket server implemented on top of raw Unreal Engine TCP sockets
 * (the Sockets module).  It does NOT depend on the WebSockets module, which is
 * absent from the Coffee Stain Studios custom engine used by Satisfactory.
 *
 * Protocol support:
 *   - RFC 6455 opening handshake (HTTP → WebSocket upgrade)
 *   - Text frames (UTF-8), binary frames, ping/pong, close
 *   - Fragmented messages
 *
 * Typical Blueprint usage:
 *   1. Create a UCustomWebSocketServer object (e.g. in BeginPlay).
 *   2. Bind delegates (OnClientConnected, connection's OnTextMessage, etc.).
 *   3. Call StartListening(Port).
 *   4. Call StopListening() in EndPlay or when done.
 *
 * Thread safety:
 *   - All delegates are dispatched on the game thread.
 *   - SendText / SendBinary / Close on UCustomWebSocketConnection are thread-safe
 *     and may be called from any thread.
 */
UCLASS(BlueprintType, Blueprintable)
class SATISFACTORYDISCORDBOT_API UCustomWebSocketServer : public UObject
{
	GENERATED_BODY()

public:
	// -------------------------------------------------------------------------
	// Blueprint API
	// -------------------------------------------------------------------------

	/**
	 * Start listening for incoming WebSocket connections on the given TCP port.
	 *
	 * @param Port   TCP port number (e.g. 8765).  Must be > 0 and < 65536.
	 * @return       True if the server socket was created and listening started.
	 */
	UFUNCTION(BlueprintCallable, Category = "Discord Bot|WebSocket")
	bool StartListening(int32 Port);

	/**
	 * Start listening for incoming *secure* WebSocket (wss://) connections.
	 *
	 * The server performs a standard TLS handshake (using OpenSSL bundled with
	 * Unreal Engine) on every accepted connection before the WebSocket upgrade.
	 * This means no dependency on the Unreal WebSockets module or any
	 * Satisfactory-specific custom-engine feature.
	 *
	 * @param Port             TCP port number (e.g. 8766).
	 * @param CertificatePath  Absolute path to a PEM-encoded certificate file
	 *                         (may include a full chain).
	 * @param PrivateKeyPath   Absolute path to a PEM-encoded private key file.
	 * @return                 True if the SSL context was loaded and the server
	 *                         started listening.
	 */
	UFUNCTION(BlueprintCallable, Category = "Discord Bot|WebSocket")
	bool StartListeningSSL(int32 Port, const FString& CertificatePath,
	                        const FString& PrivateKeyPath);

	/**
	 * Stop listening and close all active connections gracefully.
	 * Safe to call even if the server is not currently listening.
	 */
	UFUNCTION(BlueprintCallable, Category = "Discord Bot|WebSocket")
	void StopListening();

	/** Returns true if the server is currently listening for connections. */
	UFUNCTION(BlueprintPure, Category = "Discord Bot|WebSocket")
	bool IsListening() const;

	// -------------------------------------------------------------------------
	// Delegates
	// -------------------------------------------------------------------------

	/**
	 * Fired on the game thread whenever a new client completes the WebSocket
	 * handshake.  Bind to this to receive the UCustomWebSocketConnection object,
	 * then subscribe to its own delegates (OnTextMessage, OnBinaryMessage, OnClosed).
	 */
	UPROPERTY(BlueprintAssignable, Category = "Discord Bot|WebSocket")
	FOnWSClientConnected OnClientConnected;

	// -------------------------------------------------------------------------
	// UObject overrides
	// -------------------------------------------------------------------------

	virtual void BeginDestroy() override;

private:
	/** Called once per frame by the core ticker to dispatch events on the game thread. */
	bool Tick(float DeltaTime);

	/** Shuts down the background thread and cleans up resources. */
	void ShutdownInternal();

	/**
	 * Shared implementation used by StartListening and StartListeningSSL.
	 * @param Port         TCP port to bind.
	 * @param InSslContext Optional OpenSSL context for wss://.  Ownership stays
	 *                     with UCustomWebSocketServer (freed in ShutdownInternal).
	 */
	bool StartListeningInternal(int32 Port, ssl_ctx_st* InSslContext);

	// -------------------------------------------------------------------------
	// Members
	// -------------------------------------------------------------------------

	/** Background thread that owns the listen socket and polls client sockets. */
	TUniquePtr<FWSServerThread> ServerRunnable;

	/** OS-level thread wrapper around ServerRunnable. */
	FRunnableThread* ServerThread{nullptr};

	/** Live connections whose delegates we broadcast each tick. */
	UPROPERTY()
	TArray<UCustomWebSocketConnection*> ActiveConnections;

	/** Handle returned by FTSTicker so we can remove the tick delegate on shutdown. */
	FTSTicker::FDelegateHandle TickHandle;

	/**
	 * OpenSSL SSL_CTX created by StartListeningSSL.  Null when the server is
	 * operating in plain ws:// mode.  Freed in ShutdownInternal after the
	 * server thread (and therefore all SSL objects referencing it) has exited.
	 */
	ssl_ctx_st* SslContext{nullptr};
};
