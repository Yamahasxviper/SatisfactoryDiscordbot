// Copyright (c) 2024 Discord Chat Bridge Contributors

#pragma once

#include "CoreMinimal.h"
#if WITH_WEBSOCKETS_SUPPORT
#include "IWebSocket.h"
#endif
#include "DiscordGateway.generated.h"

/**
 * Discord Gateway opcodes
 */
enum class EDiscordGatewayOpcode : uint8
{
	Dispatch = 0,           // Receive server events
	Heartbeat = 1,          // Send/receive heartbeat
	Identify = 2,           // Authenticate with Discord
	PresenceUpdate = 3,     // Update bot presence/status
	VoiceStateUpdate = 4,   // Join/leave voice channels
	Resume = 6,             // Resume a disconnected session
	Reconnect = 7,          // Server requests reconnect
	RequestGuildMembers = 8,// Request guild member info
	InvalidSession = 9,     // Session invalid, must re-identify
	Hello = 10,             // Server greeting with heartbeat_interval
	HeartbeatAck = 11       // Server acknowledges heartbeat
};

/**
 * Discord Gateway connection states
 */
enum class EGatewayConnectionState : uint8
{
	Disconnected,
	Connecting,
	Identifying,
	Connected,
	Reconnecting
};

/**
 * Delegate for Gateway connection events
 */
DECLARE_DELEGATE(FOnGatewayConnected);
DECLARE_DELEGATE_OneParam(FOnGatewayDisconnected, const FString& /* Reason */);

/**
 * Discord Gateway WebSocket client for bot presence and real-time events
 */
UCLASS()
class DISCORDCHATBRIDGE_API UDiscordGateway : public UObject
{
	GENERATED_BODY()

public:
	UDiscordGateway();
	virtual ~UDiscordGateway();

	/** Initialize the Gateway with bot token */
	void Initialize(const FString& InBotToken);

	/** Connect to Discord Gateway */
	void Connect();

	/** Disconnect from Discord Gateway */
	void Disconnect();

	/** Update bot presence with activity */
	void UpdatePresence(const FString& ActivityName, int32 ActivityType = 0);

	/** Set bot status to offline (invisible) */
	void SetOfflineStatus();

	/** Check if Gateway is connected */
	bool IsConnected() const { return ConnectionState == EGatewayConnectionState::Connected; }

	/** Get current connection state */
	EGatewayConnectionState GetConnectionState() const { return ConnectionState; }

	/** Delegates */
	FOnGatewayConnected OnConnected;
	FOnGatewayDisconnected OnDisconnected;

private:
#if WITH_WEBSOCKETS_SUPPORT
	/** WebSocket connection */
	TSharedPtr<IWebSocket> WebSocket;
#endif

	/** Bot token for authentication */
	FString BotToken;

	/** Session ID (received after IDENTIFY) */
	FString SessionId;

	/** Last sequence number received */
	int32 LastSequenceNumber;

	/** Heartbeat interval in milliseconds */
	float HeartbeatIntervalMs;

	/** Timer handle for heartbeat */
	FTimerHandle HeartbeatTimerHandle;

	/** Current connection state */
	EGatewayConnectionState ConnectionState;

	/** Whether we received a heartbeat ACK */
	bool bReceivedHeartbeatAck;

	/** Gateway URL */
	static constexpr const TCHAR* GATEWAY_URL = TEXT("wss://gateway.discord.gg/?v=10&encoding=json");

	/** WebSocket event handlers */
	void OnWebSocketConnected();
	void OnWebSocketConnectionError(const FString& Error);
	void OnWebSocketClosed(int32 StatusCode, const FString& Reason, bool bWasClean);
	void OnWebSocketMessage(const FString& Message);

	/** Gateway message handlers */
	void HandleGatewayMessage(const TSharedPtr<FJsonObject>& JsonObject);
	void HandleHello(const TSharedPtr<FJsonObject>& Data);
	void HandleReady(const TSharedPtr<FJsonObject>& Data);
	void HandleHeartbeatAck();
	void HandleInvalidSession(bool CanResume);
	void HandleReconnect();

	/** Gateway operations */
	void SendIdentify();
	void SendHeartbeat();
	void SendPresenceUpdate(const FString& ActivityName, int32 ActivityType);
	void SendResume();

	/** Send a JSON payload to the Gateway */
	void SendPayload(const TSharedPtr<FJsonObject>& Payload);

	/** Start the heartbeat timer */
	void StartHeartbeat();

	/** Stop the heartbeat timer */
	void StopHeartbeat();

	/** Heartbeat timer callback */
	void OnHeartbeatTimer();
};
