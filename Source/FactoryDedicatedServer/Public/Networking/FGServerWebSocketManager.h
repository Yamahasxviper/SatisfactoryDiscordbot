// Copyright Coffee Stain Studios. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "Server/FGDSAuthenticationTypes.h"
#include "FGServerWebSocketManager.generated.h"

class UFGServerSubsystem;
class INetworkingWebSocket;
class IWebSocketServer;

DECLARE_MULTICAST_DELEGATE_TwoParams( FFGWebSocketMessageDelegate, const FString& /* ClientId */, const FString& /* Message */ );
DECLARE_MULTICAST_DELEGATE_OneParam( FFGWebSocketClientDelegate, const FString& /* ClientId */ );

/**
 * Manages incoming WebSocket connections for the dedicated server.
 *
 * WebSocket support is provided by the custom UnrealEngine-CSS (5.3.2-CSS) via the
 * built-in WebSockets / WebSocketNetworking modules. This manager is initialised by
 * UFGServerSubsystem alongside the existing HTTPS API.
 *
 * Clients must supply a valid Bearer token (the same tokens used for the HTTP API)
 * in the "Authorization" header of the initial HTTP upgrade request.
 */
UCLASS( Within=FGServerSubsystem )
class FACTORYDEDICATEDSERVER_API UFGServerWebSocketManager : public UObject
{
	GENERATED_BODY()

public:
	UFGServerWebSocketManager();

	/**
	 * Starts the WebSocket server on the specified port.
	 * @return true if the server was successfully started.
	 */
	bool Initialize( int32 WebSocketPort );

	/** Shuts down the WebSocket server and disconnects all clients. */
	void Shutdown();

	/** Returns the port the WebSocket server is listening on. */
	FORCEINLINE int32 GetWebSocketPort() const { return mWebSocketPort; }

	/** Returns true if the WebSocket server is currently running. */
	bool IsRunning() const;

	/**
	 * Broadcasts a JSON message to every authenticated WebSocket client.
	 * @param Message  UTF-8 encoded JSON string to send.
	 */
	void BroadcastMessage( const FString& Message );

	/**
	 * Sends a JSON message to a single WebSocket client identified by ClientId.
	 * @return true if the client was found and the send was enqueued.
	 */
	bool SendMessageToClient( const FString& ClientId, const FString& Message );

	/** Disconnects a single WebSocket client by ClientId. */
	void DisconnectClient( const FString& ClientId );

	/** Returns the outer UFGServerSubsystem. */
	UFGServerSubsystem* GetOuterServerSubsystem() const;

	/** Fired when an authenticated client sends a message. */
	FFGWebSocketMessageDelegate OnMessageReceived;

	/** Fired when an authenticated client connects. */
	FFGWebSocketClientDelegate OnClientConnected;

	/** Fired when a client disconnects. */
	FFGWebSocketClientDelegate OnClientDisconnected;

private:
	/** Called by the WebSocket server when a new raw connection arrives. */
	void OnWebSocketClientConnected( INetworkingWebSocket* Socket );

	/** Called when a connected client sends a text frame. */
	void OnWebSocketRawMessage( const void* Data, int32 Size, int32 BytesRemaining );

	/**
	 * Validates the Bearer token from the upgrade-request headers.
	 * OutClientId is an out-parameter populated on success; the method itself does not mutate object state.
	 */
	bool AuthenticateWebSocketClient( const TMap<FString, FString>& UpgradeHeaders, FString& OutClientId ) const;

	/** The underlying IWebSocketServer instance (from the WebSockets / WebSocketNetworking modules). */
	TSharedPtr<IWebSocketServer> mWebSocketServer;

	/**
	 * Non-owning pointers to authenticated client sockets; lifetime is managed by mWebSocketServer.
	 * Entries are removed in OnClientDisconnected before the underlying socket is destroyed.
	 */
	TMap<FString, INetworkingWebSocket*> mAuthenticatedClients;

	/** Port the WebSocket server is bound to. */
	int32 mWebSocketPort{0};
};
