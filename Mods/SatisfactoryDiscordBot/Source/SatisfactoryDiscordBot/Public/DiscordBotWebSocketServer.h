#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Containers/Ticker.h"
#include "DiscordBotWebSocketServer.generated.h"

class IWebSocketServer;
class INetworkingWebSocket;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnWebSocketMessageReceived, int32, ClientIndex, const FString&, Message);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnWebSocketClientConnected, int32, ClientIndex);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnWebSocketClientDisconnected, int32, ClientIndex);

/**
 * WebSocket server subsystem for Discord bot integration.
 *
 * This subsystem starts a WebSocket server on the game instance so that an
 * external Discord bot (or any WebSocket client) can connect and exchange
 * messages with the running Satisfactory server.
 *
 * Usage:
 *   - Bind to OnMessageReceived to react to incoming messages.
 *   - Call BroadcastMessage() to send data to every connected client.
 *   - Call SendMessageToClient() to target a single client.
 */
UCLASS()
class SATISFACTORYDISCORDBOT_API UDiscordBotWebSocketServer : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	/**
	 * Sends a text message to all connected WebSocket clients.
	 * @param Message  UTF-8 compatible string to broadcast.
	 */
	UFUNCTION(BlueprintCallable, Category = "Discord Bot|WebSocket")
	void BroadcastMessage(const FString& Message);

	/**
	 * Sends a text message to a single connected client.
	 * @param ClientIndex  Zero-based index of the target client.
	 * @param Message      UTF-8 compatible string to send.
	 */
	UFUNCTION(BlueprintCallable, Category = "Discord Bot|WebSocket")
	void SendMessageToClient(int32 ClientIndex, const FString& Message);

	/** Returns the number of currently connected WebSocket clients. */
	UFUNCTION(BlueprintPure, Category = "Discord Bot|WebSocket")
	int32 GetConnectedClientCount() const;

	/** Fired when a text message is received from a client. */
	UPROPERTY(BlueprintAssignable, Category = "Discord Bot|WebSocket")
	FOnWebSocketMessageReceived OnMessageReceived;

	/** Fired when a new WebSocket client establishes a connection. */
	UPROPERTY(BlueprintAssignable, Category = "Discord Bot|WebSocket")
	FOnWebSocketClientConnected OnClientConnected;

	/** Fired when a WebSocket client closes its connection. */
	UPROPERTY(BlueprintAssignable, Category = "Discord Bot|WebSocket")
	FOnWebSocketClientDisconnected OnClientDisconnected;

	/** TCP port the WebSocket server listens on. Change before the game instance starts to take effect. */
	UPROPERTY(BlueprintReadWrite, Category = "Discord Bot|WebSocket")
	int32 Port = 8765;

private:
	void StartServer();
	void StopServer();

	void OnClientConnectedCallback(INetworkingWebSocket* Socket);
	bool OnTick(float DeltaTime);

	TSharedPtr<IWebSocketServer> WebSocketServer;
	TArray<INetworkingWebSocket*> ConnectedClients;
	FTSTicker::FDelegateHandle TickHandle;
};
