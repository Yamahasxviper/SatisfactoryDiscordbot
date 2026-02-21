// Copyright Coffee Stain Studios. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Containers/Ticker.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "SMLWebSocketClient.h"
#include "DiscordBridgeConfig.h"
#include "DiscordBridgeSubsystem.generated.h"

// ── Delegate declarations ─────────────────────────────────────────────────────

/**
 * Fired on the game thread when a message is received from the bridged Discord
 * channel.  Bind this in Blueprint to forward the message to in-game chat.
 *
 * @param Username  Display name of the Discord user who sent the message.
 * @param Message   Plain text content of the Discord message.
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FDiscordMessageReceivedDelegate,
                                             const FString&, Username,
                                             const FString&, Message);

/**
 * Fired on the game thread when the Discord Gateway connection is established
 * and the bot has been identified successfully (Ready event received).
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FDiscordConnectedDelegate);

/**
 * Fired on the game thread when the Discord Gateway connection is lost.
 *
 * @param Reason  Human-readable description of why the connection ended.
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FDiscordDisconnectedDelegate,
                                            const FString&, Reason);

// ── Discord Gateway opcodes (Discord API reference §Gateway Opcodes) ──────────
namespace EDiscordGatewayOpcode
{
	static constexpr int32 Dispatch          = 0;  // Server → Client: an event was dispatched
	static constexpr int32 Heartbeat         = 1;  // Client → Server: keep-alive heartbeat
	static constexpr int32 Identify          = 2;  // Client → Server: trigger authentication
	static constexpr int32 UpdatePresence    = 3;  // Client → Server: update bot presence/status
	static constexpr int32 Resume            = 6;  // Client → Server: resume a dropped session
	static constexpr int32 Reconnect         = 7;  // Server → Client: client should reconnect
	static constexpr int32 InvalidSession    = 9;  // Server → Client: session is invalid
	static constexpr int32 Hello            = 10;  // Server → Client: sent immediately after connecting
	static constexpr int32 HeartbeatAck     = 11;  // Server → Client: heartbeat was acknowledged
}

// ── Discord Gateway intent bit-flags (Discord API reference §Gateway Intents) ─
namespace EDiscordGatewayIntent
{
	// Non-privileged
	static constexpr int32 Guilds         = 1 << 0;   //    1
	static constexpr int32 GuildMessages  = 1 << 9;   //  512

	// Privileged – must be enabled in the Discord Developer Portal
	static constexpr int32 GuildMembers   = 1 << 1;   //    2  (Server Members Intent)
	static constexpr int32 GuildPresences = 1 << 8;   //  256  (Presence Intent)
	static constexpr int32 MessageContent = 1 << 15;  // 32768 (Message Content Intent)

	// Combined value used when connecting to the Gateway.
	static constexpr int32 All =
		Guilds | GuildMembers | GuildPresences | GuildMessages | MessageContent;
	// = 1 + 2 + 256 + 512 + 32768 = 33539
}

/**
 * UDiscordBridgeSubsystem
 *
 * A GameInstance-level subsystem that bridges Satisfactory in-game chat with
 * a Discord text channel.
 *
 * How it works
 * ────────────
 *  • Connects to the Discord Gateway (wss://gateway.discord.gg/?v=10&encoding=json)
 *    using USMLWebSocketClient from the SMLWebSocket plugin.
 *  • Authenticates with the configured BotToken and requests the three privileged
 *    intents: Presence Intent, Server Members Intent, Message Content Intent.
 *  • Discord → Game: MESSAGE_CREATE events on the configured channel fire
 *    OnDiscordMessageReceived so that Blueprint (or another C++ subsystem) can
 *    inject the message into the Satisfactory chat.
 *  • Game → Discord: Call SendGameMessageToDiscord() to POST the message to the
 *    Discord REST API (https://discord.com/api/v10/channels/{id}/messages).
 *
 * Setup
 * ─────
 *  1. Create a Discord application and bot in the Discord Developer Portal.
 *  2. Enable all three Privileged Gateway Intents on the Bot page.
 *  3. Invite the bot to your server with "Send Messages" + "Read Message History".
 *  4. Fill in BotToken and ChannelId in Configs/DiscordBridge.cfg (auto-created
 *     on first server start) and restart the server.
 *  5. Optionally customise GameToDiscordFormat and DiscordToGameFormat.
 *  6. In Blueprint, bind to OnDiscordMessageReceived and call
 *     SendGameMessageToDiscord() from your chat hooks.
 */
UCLASS(BlueprintType)
class DISCORDBRIDGE_API UDiscordBridgeSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	// ── USubsystem ────────────────────────────────────────────────────────────

	/** Restrict this subsystem to dedicated servers only. */
	virtual bool ShouldCreateSubsystem(UObject* Outer) const override;
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	// ── Delegates ─────────────────────────────────────────────────────────────

	/**
	 * Fired when a message is received from the bridged Discord channel.
	 * Bind this in Blueprint to forward the message to the Satisfactory chat.
	 */
	UPROPERTY(BlueprintAssignable, Category="Discord Bridge")
	FDiscordMessageReceivedDelegate OnDiscordMessageReceived;

	/** Fired when the Discord Gateway connection is ready. */
	UPROPERTY(BlueprintAssignable, Category="Discord Bridge")
	FDiscordConnectedDelegate OnDiscordConnected;

	/** Fired when the Discord Gateway connection is lost. */
	UPROPERTY(BlueprintAssignable, Category="Discord Bridge")
	FDiscordDisconnectedDelegate OnDiscordDisconnected;

	// ── Public API ────────────────────────────────────────────────────────────

	/**
	 * Forward a Satisfactory chat message to the bridged Discord channel via
	 * the Discord REST API.
	 *
	 * @param PlayerName  The in-game name of the player who sent the message.
	 *                    Substituted for {PlayerName} in GameToDiscordFormat.
	 * @param Message     The chat message text.
	 *                    Substituted for {Message} in GameToDiscordFormat.
	 */
	UFUNCTION(BlueprintCallable, Category="Discord Bridge")
	void SendGameMessageToDiscord(const FString& PlayerName, const FString& Message);

	/**
	 * Manually trigger a connection to the Discord Gateway.
	 * Called automatically during Initialize() when BotToken and ChannelId are set.
	 */
	UFUNCTION(BlueprintCallable, Category="Discord Bridge")
	void Connect();

	/**
	 * Disconnect from the Discord Gateway and cancel the heartbeat timer.
	 * Called automatically during Deinitialize().
	 */
	UFUNCTION(BlueprintCallable, Category="Discord Bridge")
	void Disconnect();

	/**
	 * Send a presence update to Discord to set the bot's online status.
	 *
	 * @param Status  One of "online", "idle", "dnd", or "invisible".
	 */
	UFUNCTION(BlueprintCallable, Category="Discord Bridge")
	void SendUpdatePresence(const FString& Status);

	/**
	 * Returns true when the Gateway WebSocket is open and the bot has been
	 * identified (Ready event received from Discord).
	 */
	UFUNCTION(BlueprintPure, Category="Discord Bridge")
	bool IsConnected() const { return bGatewayReady; }

private:
	// ── WebSocket event handlers (called on the game thread) ──────────────────

	UFUNCTION()
	void OnWebSocketConnected();

	UFUNCTION()
	void OnWebSocketMessage(const FString& RawJson);

	UFUNCTION()
	void OnWebSocketClosed(int32 StatusCode, const FString& Reason);

	UFUNCTION()
	void OnWebSocketError(const FString& ErrorMessage);

	UFUNCTION()
	void OnWebSocketReconnecting(int32 AttemptNumber, float DelaySeconds);

	// ── Discord Gateway protocol ──────────────────────────────────────────────

	/** Dispatch the correct handler based on the Gateway opcode. */
	void HandleGatewayPayload(const FString& RawJson);

	/** op=10: Server sent Hello; start heartbeating and send Identify. */
	void HandleHello(const TSharedPtr<FJsonObject>& DataObj);

	/** op=0: Server dispatched an event. Routes to the correct event handler. */
	void HandleDispatch(const FString& EventType, int32 Sequence,
	                    const TSharedPtr<FJsonObject>& DataObj);

	/** op=11: Server acknowledged our heartbeat. */
	void HandleHeartbeatAck();

	/** op=7: Server asked us to reconnect. */
	void HandleReconnect();

	/** op=9: Session is invalid; re-identify or start fresh. */
	void HandleInvalidSession(bool bResumable);

	/** t=READY: Bot is authenticated and ready. */
	void HandleReady(const TSharedPtr<FJsonObject>& DataObj);

	/** t=MESSAGE_CREATE: A new message was posted in a channel. */
	void HandleMessageCreate(const TSharedPtr<FJsonObject>& DataObj);

	/** Handle the !logs command: read the server log and post recent lines to Discord. */
	void HandleLogsCommand(const FString& ChannelId);

	/** Send the Identify payload (op=2) to authenticate the bot. */
	void SendIdentify();

	/** Send a heartbeat (op=1) to keep the Gateway connection alive. */
	void SendHeartbeat();

	/** Send a plain text message to the configured Discord channel via the REST API. */
	void SendStatusMessageToDiscord(const FString& Message);

	/** POST a plain text message to any Discord channel via the REST API. */
	void PostTextToChannel(const FString& TargetChannelId, const FString& Message);

	/** Serialise a JSON object and send it as a text WebSocket frame. */
	void SendGatewayPayload(const TSharedPtr<FJsonObject>& Payload);

	// ── Heartbeat timer ───────────────────────────────────────────────────────

	/** Timer callback – fires SendHeartbeat() at the interval Discord requested. */
	bool HeartbeatTick(float DeltaTime);

	FTSTicker::FDelegateHandle HeartbeatTickerHandle;
	float HeartbeatIntervalSeconds{0.0f};

	// ── Internal state ────────────────────────────────────────────────────────

	/** The WebSocket client connected to the Discord Gateway. */
	UPROPERTY()
	USMLWebSocketClient* WebSocketClient{nullptr};

	/** Loaded configuration (populated in Initialize()). */
	FDiscordBridgeConfig Config;

	/** Last sequence number received from Discord (used in heartbeats). */
	int32 LastSequenceNumber{-1};

	/** true after the READY dispatch has been received from Discord. */
	bool bGatewayReady{false};

	/** Snowflake ID of the bot user; used to filter out self-sent messages. */
	FString BotUserId;
};
