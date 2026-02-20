// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "Containers/Ticker.h"
#include "SMLWebSocket.h"
#include "DiscordGatewayClient.generated.h"

// ---------------------------------------------------------------------------
// Discord Gateway opcodes (https://discord.com/developers/docs/topics/opcodes-and-status-codes)
// ---------------------------------------------------------------------------

UENUM(BlueprintType)
enum class EDiscordGatewayOpcode : uint8
{
	Dispatch        = 0,   // Server → Client: an event was dispatched
	Heartbeat       = 1,   // Send/Receive: fire a heartbeat immediately
	Identify        = 2,   // Client → Server: start a new session
	Resume          = 6,   // Client → Server: resume a previous session
	Reconnect       = 7,   // Server → Client: disconnect and reconnect
	InvalidSession  = 9,   // Server → Client: session is invalid
	Hello           = 10,  // Server → Client: sent immediately after connecting
	HeartbeatAck    = 11,  // Server → Client: acknowledge a heartbeat
};

// ---------------------------------------------------------------------------
// Non-privileged Discord Gateway intents
//
// Privileged intents that are intentionally excluded:
//   GUILD_PRESENCES   (1 << 8  = 256)   – Presence Intent
//   GUILD_MEMBERS     (1 << 1  = 2)     – Server Members Intent
//   MESSAGE_CONTENT   (1 << 15 = 32768) – Message Content Intent
// ---------------------------------------------------------------------------

UENUM(BlueprintType, Meta = (Bitflags, UseEnumValuesAsMaskValuesInEditor = "true"))
enum class EDiscordGatewayIntent : int32
{
	None                   = 0,
	Guilds                 = 1,      // 1 << 0
	GuildBans              = 4,      // 1 << 2
	GuildEmojisAndStickers = 8,      // 1 << 3
	GuildIntegrations      = 16,     // 1 << 4
	GuildWebhooks          = 32,     // 1 << 5
	GuildInvites           = 64,     // 1 << 6
	GuildVoiceStates       = 128,    // 1 << 7
	GuildMessages          = 512,    // 1 << 9  (non-privileged; receives events but NOT message content)
	GuildMessageReactions  = 1024,   // 1 << 10
	GuildMessageTyping     = 2048,   // 1 << 11
	DirectMessages         = 4096,   // 1 << 12
	DirectMessageReactions = 8192,   // 1 << 13
	DirectMessageTyping    = 16384,  // 1 << 14
	GuildScheduledEvents   = 65536,  // 1 << 16
};

// ---------------------------------------------------------------------------
// Delegate declarations
// ---------------------------------------------------------------------------

/** Fired when the bot is fully authenticated with the Discord Gateway (READY event received). */
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnDiscordReady);

/**
 * Fired for every DISPATCH event received from the Discord Gateway.
 * @param EventName   The event type (e.g. "MESSAGE_CREATE", "GUILD_CREATE").
 * @param PayloadJson The full "d" (data) field of the gateway payload as a JSON string.
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnDiscordDispatch,
	const FString&, EventName, const FString&, PayloadJson);

/** Fired when the gateway connection is lost or an authentication error occurs. */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDiscordGatewayError, const FString&, Error);

/** Fired when the gateway is instructed by Discord to reconnect. */
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnDiscordReconnect);

// ---------------------------------------------------------------------------
// UDiscordGatewayClient
// ---------------------------------------------------------------------------

/**
 * Handles the Discord Gateway WebSocket protocol on top of USMLWebSocket.
 *
 * Connects to the Discord Gateway, performs the HELLO → IDENTIFY handshake,
 * manages heartbeating, and dispatches incoming events to Blueprint.
 *
 * Only non-privileged intents are used by default, so no special enablement
 * in the Discord Developer Portal is required:
 *   - Presence Intent       is NOT requested
 *   - Server Members Intent is NOT requested
 *   - Message Content Intent is NOT requested
 *
 * NOTE: Discord's Gateway endpoint uses wss:// (TLS). If your environment
 * does not have a TLS-capable WebSocket client, route the connection through
 * a local TLS-terminating proxy (e.g. stunnel or nginx) and point this client
 * at ws://localhost:<proxy_port>.
 *
 * Usage (C++):
 * @code
 *   UDiscordGatewayClient* GW = NewObject<UDiscordGatewayClient>(this);
 *   GW->OnReady.AddDynamic(this, &UMyClass::HandleReady);
 *   GW->OnDispatch.AddDynamic(this, &UMyClass::HandleDispatch);
 *   GW->Connect(TEXT("Bot MyBotToken"), TEXT("wss://gateway.discord.gg/?v=10&encoding=json"));
 * @endcode
 */
UCLASS(BlueprintType, Blueprintable)
class DISCORDBOT_API UDiscordGatewayClient : public UObject
{
	GENERATED_BODY()

public:
	UDiscordGatewayClient();
	virtual void BeginDestroy() override;

	// -----------------------------------------------------------------------
	// Connection API
	// -----------------------------------------------------------------------

	/**
	 * Connect to the Discord Gateway.
	 *
	 * @param InBotToken     Your Discord bot token. Include the "Bot " prefix
	 *                       (e.g. "Bot MTIz..."). This is sent as the Authorization
	 *                       header in the IDENTIFY payload; it is never transmitted
	 *                       in HTTP headers.
	 * @param GatewayUrl     The WebSocket URL to connect to.  Defaults to the
	 *                       standard Discord Gateway with API v10.
	 *                       Use a ws:// URL pointing at a local TLS-terminating
	 *                       proxy if direct wss:// is unavailable.
	 * @param Intents        Bitmask of EDiscordGatewayIntent values to subscribe
	 *                       to. Defaults to Guilds + GuildMessages +
	 *                       GuildMessageReactions + DirectMessages (5633).
	 *                       Do NOT include privileged intent values (256, 2, 32768).
	 */
	UFUNCTION(BlueprintCallable, Category = "Discord|Gateway")
	void Connect(
		const FString& InBotToken,
		const FString& GatewayUrl = TEXT("wss://gateway.discord.gg/?v=10&encoding=json"),
		int32 Intents = 5633 /* Guilds=1 | GuildMessages=512 | GuildMessageReactions=1024 | DirectMessages=4096 */);

	/** Cleanly disconnect from the Discord Gateway. */
	UFUNCTION(BlueprintCallable, Category = "Discord|Gateway")
	void Disconnect();

	// -----------------------------------------------------------------------
	// State queries
	// -----------------------------------------------------------------------

	/** Returns true after the READY event has been received from the Gateway. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Discord|Gateway")
	bool IsReady() const { return bIsReady; }

	/** Returns the session ID provided by Discord in the READY event. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Discord|Gateway")
	FString GetSessionId() const { return SessionId; }

	// -----------------------------------------------------------------------
	// Events (Blueprint-assignable delegates)
	// -----------------------------------------------------------------------

	/** Fired once the Gateway READY event is received (bot is authenticated). */
	UPROPERTY(BlueprintAssignable, Category = "Discord|Gateway")
	FOnDiscordReady OnReady;

	/**
	 * Fired for every DISPATCH event from the Gateway.
	 * Bind this to receive any event type (MESSAGE_CREATE, GUILD_CREATE, etc.).
	 */
	UPROPERTY(BlueprintAssignable, Category = "Discord|Gateway")
	FOnDiscordDispatch OnDispatch;

	/** Fired when a connection error or authentication failure occurs. */
	UPROPERTY(BlueprintAssignable, Category = "Discord|Gateway")
	FOnDiscordGatewayError OnError;

	/** Fired when Discord requests a reconnect (opcode 7). */
	UPROPERTY(BlueprintAssignable, Category = "Discord|Gateway")
	FOnDiscordReconnect OnReconnect;

private:
	// The underlying WebSocket transport.
	UPROPERTY()
	USMLWebSocket* WebSocket;

	// Bot token (stored for session resume / re-identify).
	FString BotToken;

	// Resolved intents bitmask.
	int32 GatewayIntents;

	// Last sequence number received from the Gateway (used for heartbeats and resume).
	int32 LastSequenceNumber;

	// Session ID provided by the READY event.
	FString SessionId;

	// True after the READY event has been successfully received.
	bool bIsReady;

	// Heartbeat state.
	FTSTicker::FDelegateHandle HeartbeatTickerHandle;
	float HeartbeatIntervalSeconds;
	float HeartbeatElapsedSeconds;
	bool  bHeartbeatAckReceived;

	// -----------------------------------------------------------------------
	// WebSocket event handlers
	// -----------------------------------------------------------------------

	UFUNCTION()
	void HandleWebSocketConnected();

	UFUNCTION()
	void HandleWebSocketMessage(const FString& Message);

	UFUNCTION()
	void HandleWebSocketError(const FString& Error);

	UFUNCTION()
	void HandleWebSocketClosed(int32 StatusCode, const FString& Reason);

	// -----------------------------------------------------------------------
	// Gateway protocol helpers
	// -----------------------------------------------------------------------

	/** Process a fully parsed Gateway payload. */
	void HandleGatewayPayload(int32 Op, int32 Seq, const FString& EventName,
		const TSharedPtr<FJsonObject>& DataObj, const FString& RawDataJson);

	/** Send the IDENTIFY payload (opcode 2) with non-privileged intents. */
	void SendIdentify();

	/** Send a heartbeat payload (opcode 1). */
	void SendHeartbeat();

	/** Serialize and send a JSON object over the WebSocket. */
	void SendJson(const TSharedRef<FJsonObject>& Payload);

	/** Start the heartbeat ticker at the given interval. */
	void StartHeartbeat(float IntervalSeconds);

	/** Stop the heartbeat ticker. */
	void StopHeartbeat();
};

// ---------------------------------------------------------------------------
// UDiscordGatewayClientBlueprintLibrary  –  factory helper
// ---------------------------------------------------------------------------

/** Blueprint function library with factory helpers for UDiscordGatewayClient. */
UCLASS()
class DISCORDBOT_API UDiscordGatewayClientBlueprintLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/**
	 * Create a new UDiscordGatewayClient instance.
	 * Call Connect() on the returned object to open the Discord Gateway connection.
	 *
	 * @param WorldContextObject  Any UObject in the current world (for GC rooting).
	 * @return A new, unconnected Discord Gateway client.
	 */
	UFUNCTION(BlueprintCallable, Category = "Discord|Gateway",
		meta = (WorldContext = "WorldContextObject"))
	static UDiscordGatewayClient* CreateDiscordGatewayClient(UObject* WorldContextObject);
};
