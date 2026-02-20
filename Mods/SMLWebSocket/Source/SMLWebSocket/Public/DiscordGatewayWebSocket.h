// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "SMLWebSocket.h"
#include "DiscordGatewayWebSocket.generated.h"

// ---------------------------------------------------------------------------
// Discord Gateway Intents  (Discord API v10)
// https://discord.com/developers/docs/topics/gateway#gateway-intents
// ---------------------------------------------------------------------------

/**
 * Bit-flag enum representing Discord Gateway intents.
 *
 * Combine the flags you need and pass the bitmask to ConnectToDiscord().
 * Three of these require explicit enablement in the Discord Developer Portal
 * (marked PRIVILEGED):
 *   - GuildPresences  (Presence Intent)
 *   - GuildMembers    (Server Members Intent)
 *   - MessageContent  (Message Content Intent)
 *
 * Blueprint usage: declare an int32 UPROPERTY with
 *   Meta = (Bitmask, BitmaskEnum = "EDiscordGatewayIntent")
 * and combine the bits in the editor.
 */
UENUM(BlueprintType, Meta = (Bitflags, UseEnumValuesAsMaskValuesInEditor = "true"))
enum class EDiscordGatewayIntent : int32
{
	/** Receive guild-level events (create/update/delete, role events, etc.). */
	Guilds                      UMETA(DisplayName = "Guilds")                          = 1,        // 1 << 0

	/**
	 * PRIVILEGED – Server Members Intent.
	 * Receive GUILD_MEMBER_ADD / GUILD_MEMBER_UPDATE / GUILD_MEMBER_REMOVE events.
	 * Must be enabled in the Discord Developer Portal.
	 */
	GuildMembers                UMETA(DisplayName = "Server Members (Privileged)")     = 2,        // 1 << 1

	/** Receive guild ban / unban events. */
	GuildModeration             UMETA(DisplayName = "Guild Moderation")                = 4,        // 1 << 2

	/** Receive guild emoji and sticker update events. */
	GuildEmojisAndStickers      UMETA(DisplayName = "Guild Emojis & Stickers")         = 8,        // 1 << 3

	/** Receive guild integration events. */
	GuildIntegrations           UMETA(DisplayName = "Guild Integrations")              = 16,       // 1 << 4

	/** Receive guild webhook events. */
	GuildWebhooks               UMETA(DisplayName = "Guild Webhooks")                  = 32,       // 1 << 5

	/** Receive guild invite events. */
	GuildInvites                UMETA(DisplayName = "Guild Invites")                   = 64,       // 1 << 6

	/** Receive voice-state update events. */
	GuildVoiceStates            UMETA(DisplayName = "Guild Voice States")              = 128,      // 1 << 7

	/**
	 * PRIVILEGED – Presence Intent.
	 * Receive PRESENCE_UPDATE events.
	 * Must be enabled in the Discord Developer Portal.
	 */
	GuildPresences              UMETA(DisplayName = "Presence (Privileged)")           = 256,      // 1 << 8

	/** Receive message-related events in guild text channels. */
	GuildMessages               UMETA(DisplayName = "Guild Messages")                  = 512,      // 1 << 9

	/** Receive guild message reaction events. */
	GuildMessageReactions       UMETA(DisplayName = "Guild Message Reactions")         = 1024,     // 1 << 10

	/** Receive guild message typing events. */
	GuildMessageTyping          UMETA(DisplayName = "Guild Message Typing")            = 2048,     // 1 << 11

	/** Receive DM message events. */
	DirectMessages              UMETA(DisplayName = "Direct Messages")                 = 4096,     // 1 << 12

	/** Receive DM message reaction events. */
	DirectMessageReactions      UMETA(DisplayName = "DM Reactions")                   = 8192,     // 1 << 13

	/** Receive DM typing events. */
	DirectMessageTyping         UMETA(DisplayName = "DM Typing")                       = 16384,    // 1 << 14

	/**
	 * PRIVILEGED – Message Content Intent.
	 * Receive the content, attachments, embeds, and components of messages.
	 * Must be enabled in the Discord Developer Portal.
	 */
	MessageContent              UMETA(DisplayName = "Message Content (Privileged)")    = 32768,    // 1 << 15

	/** Receive guild scheduled-event events. */
	GuildScheduledEvents        UMETA(DisplayName = "Guild Scheduled Events")          = 65536,    // 1 << 16

	/** Receive Auto Moderation configuration events. */
	AutoModerationConfiguration UMETA(DisplayName = "AutoMod Configuration")          = 1048576,  // 1 << 20

	/** Receive Auto Moderation action execution events. */
	AutoModerationExecution     UMETA(DisplayName = "AutoMod Execution")              = 2097152,  // 1 << 21

	/** Receive message-poll events in guilds. */
	GuildMessagePolls           UMETA(DisplayName = "Guild Message Polls")             = 16777216, // 1 << 24

	/** Receive message-poll events in DMs. */
	DirectMessagePolls          UMETA(DisplayName = "DM Polls")                        = 33554432, // 1 << 25
};
ENUM_CLASS_FLAGS(EDiscordGatewayIntent);

// ---------------------------------------------------------------------------
// Convenience constant – the three privileged intents from the issue.
// ---------------------------------------------------------------------------

/**
 * Bitmask combining all three privileged Discord Gateway intents:
 *   - Presence Intent       (GuildPresences,  1 << 8)
 *   - Server Members Intent (GuildMembers,    1 << 1)
 *   - Message Content Intent (MessageContent, 1 << 15)
 *
 * These must also be enabled in the Discord Developer Portal under
 * Bot → Privileged Gateway Intents before they will function.
 */
inline constexpr int32 DiscordPrivilegedIntents =
	static_cast<int32>(EDiscordGatewayIntent::GuildPresences) |
	static_cast<int32>(EDiscordGatewayIntent::GuildMembers)   |
	static_cast<int32>(EDiscordGatewayIntent::MessageContent);

// ---------------------------------------------------------------------------
// Delegates
// ---------------------------------------------------------------------------

/**
 * Fired when the bot has successfully identified (received READY from Discord).
 * @param SessionId  The Gateway session ID (used for resuming sessions).
 * @param UserId     The Discord user ID of the bot.
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnDiscordReady,
	const FString&, SessionId, const FString&, UserId);

/**
 * Fired for every raw dispatch event (op=0) received from Discord.
 * @param EventName      The Discord event name (e.g. "MESSAGE_CREATE").
 * @param EventDataJson  The "d" (data) field of the payload, serialised as JSON.
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnDiscordEvent,
	const FString&, EventName, const FString&, EventDataJson);

/**
 * Fired on PRESENCE_UPDATE events (requires GuildPresences intent).
 * @param UserId  Discord user ID whose presence changed.
 * @param Status  New status string ("online", "idle", "dnd", "offline").
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnDiscordPresenceUpdate,
	const FString&, UserId, const FString&, Status);

/**
 * Fired on GUILD_MEMBER_ADD events (requires GuildMembers intent).
 * @param GuildId  The guild the member joined.
 * @param UserId   Discord user ID of the new member.
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnDiscordGuildMemberAdd,
	const FString&, GuildId, const FString&, UserId);

/**
 * Fired on MESSAGE_CREATE events (requires GuildMessages + MessageContent intents).
 * @param ChannelId  Channel the message was posted in.
 * @param AuthorId   Discord user ID of the message author.
 * @param Content    Text content of the message.
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnDiscordMessageCreate,
	const FString&, ChannelId, const FString&, AuthorId, const FString&, Content);

// ---------------------------------------------------------------------------
// UDiscordGatewayWebSocket
// ---------------------------------------------------------------------------

/**
 * Discord Gateway WebSocket client built on top of USMLWebSocket.
 *
 * Handles the Discord Gateway protocol (op-codes defined at
 * https://discord.com/developers/docs/topics/opcodes-and-status-codes):
 *   - Heartbeating    (op=1 sent / op=11 received)
 *   - Identify        (op=2, sends bot token + intents)
 *   - Event dispatch  (op=0, routes common event types)
 *
 * The three privileged intents (Presence, Server Members, Message Content)
 * must ALSO be enabled in the Discord Developer Portal (Bot → Privileged
 * Gateway Intents) before Discord will deliver the corresponding events.
 *
 * Usage (C++):
 * @code
 *   UDiscordGatewayWebSocket* GW =
 *       UDiscordGatewayWebSocket::CreateDiscordGateway(this);
 *   GW->OnReady.AddDynamic(this, &UMyClass::HandleReady);
 *   GW->OnMessageCreate.AddDynamic(this, &UMyClass::HandleMessage);
 *   GW->ConnectToDiscord(TEXT("Bot MY_TOKEN"), DiscordPrivilegedIntents);
 * @endcode
 *
 * @note Discord requires a TLS connection (wss://). Because this plugin uses
 *       raw TCP sockets, you must front the Discord Gateway with a
 *       TLS-terminating proxy (e.g. nginx, stunnel) and connect to its
 *       ws:// endpoint, OR pass a wss:// URL and let the URL parser log the
 *       advisory message.  The default GatewayUrl in ConnectToDiscord() is
 *       a ws:// URL intended for use behind such a proxy.
 */
UCLASS(BlueprintType, Blueprintable)
class SMLWEBSOCKET_API UDiscordGatewayWebSocket : public UObject
{
	GENERATED_BODY()

public:
	UDiscordGatewayWebSocket();
	virtual void BeginDestroy() override;

	// -----------------------------------------------------------------------
	// Factory
	// -----------------------------------------------------------------------

	/**
	 * Create a new, unconnected Discord Gateway WebSocket.
	 * Call ConnectToDiscord() on the returned object to open the connection.
	 *
	 * @param WorldContextObject  Any UObject in the current world (for GC rooting
	 *                            and timer access).
	 * @return A new UDiscordGatewayWebSocket instance.
	 */
	UFUNCTION(BlueprintCallable, Category = "SML|Discord",
		meta = (WorldContext = "WorldContextObject"))
	static UDiscordGatewayWebSocket* CreateDiscordGateway(UObject* WorldContextObject);

	// -----------------------------------------------------------------------
	// Connection API
	// -----------------------------------------------------------------------

	/**
	 * Connect to the Discord Gateway and identify with the given intents.
	 *
	 * @param BotToken    Bot token.  The "Bot " prefix is added automatically
	 *                    if not already present (e.g. pass "MY_TOKEN" or
	 *                    "Bot MY_TOKEN" – both work).
	 * @param Intents     Bitmask of EDiscordGatewayIntent flags.
	 *                    Use DiscordPrivilegedIntents to request all three
	 *                    privileged intents, or combine specific flags with |.
	 * @param GatewayUrl  WebSocket URL to connect to.  The default points to
	 *                    a ws:// URL for use behind a TLS-terminating proxy.
	 *                    Pass a wss:// URL to connect directly (note: actual
	 *                    TLS encryption still requires a proxy or SSL layer).
	 */
	UFUNCTION(BlueprintCallable, Category = "SML|Discord")
	void ConnectToDiscord(
		const FString& BotToken,
		int32          Intents,
		const FString& GatewayUrl = TEXT("ws://gateway.discord.gg/?v=10&encoding=json"));

	/** Close the Discord Gateway connection gracefully. */
	UFUNCTION(BlueprintCallable, Category = "SML|Discord")
	void Disconnect();

	/**
	 * Returns true once the Gateway has been successfully identified
	 * (i.e. after the READY event is received).
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "SML|Discord")
	bool IsIdentified() const { return bIdentified; }

	// -----------------------------------------------------------------------
	// Events (Blueprint-assignable delegates)
	// -----------------------------------------------------------------------

	/** Fired when the bot has successfully identified (READY event). */
	UPROPERTY(BlueprintAssignable, Category = "SML|Discord")
	FOnDiscordReady OnReady;

	/** Fired for every raw dispatch event (op=0) received from Discord. */
	UPROPERTY(BlueprintAssignable, Category = "SML|Discord")
	FOnDiscordEvent OnDiscordEvent;

	/**
	 * Fired on PRESENCE_UPDATE events.
	 * Requires the GuildPresences (Presence) intent.
	 */
	UPROPERTY(BlueprintAssignable, Category = "SML|Discord")
	FOnDiscordPresenceUpdate OnPresenceUpdate;

	/**
	 * Fired on GUILD_MEMBER_ADD events.
	 * Requires the GuildMembers (Server Members) intent.
	 */
	UPROPERTY(BlueprintAssignable, Category = "SML|Discord")
	FOnDiscordGuildMemberAdd OnGuildMemberAdd;

	/**
	 * Fired on MESSAGE_CREATE events.
	 * Requires the GuildMessages intent; content requires MessageContent intent.
	 */
	UPROPERTY(BlueprintAssignable, Category = "SML|Discord")
	FOnDiscordMessageCreate OnMessageCreate;

	/** Forwarded from the underlying USMLWebSocket on connection error. */
	UPROPERTY(BlueprintAssignable, Category = "SML|Discord")
	FOnSMLWebSocketConnectionError OnConnectionError;

	/** Forwarded from the underlying USMLWebSocket when the connection closes. */
	UPROPERTY(BlueprintAssignable, Category = "SML|Discord")
	FOnSMLWebSocketClosed OnClosed;

private:
	// -----------------------------------------------------------------------
	// USMLWebSocket event handlers  (bound with AddDynamic)
	// -----------------------------------------------------------------------

	UFUNCTION()
	void HandleConnected();

	UFUNCTION()
	void HandleConnectionError(const FString& Error);

	UFUNCTION()
	void HandleClosed(int32 Code, const FString& Reason);

	UFUNCTION()
	void HandleMessage(const FString& Message);

	// -----------------------------------------------------------------------
	// Gateway protocol helpers
	// -----------------------------------------------------------------------

	/** Parse a raw JSON Gateway payload and dispatch to the right handler. */
	void HandleGatewayPayload(const FString& Json);

	/** Handle op=10 HELLO – start heartbeating, then send IDENTIFY. */
	void HandleHello(int32 HeartbeatIntervalMs);

	/** Handle op=0 dispatch events. */
	void HandleDispatch(const FString& EventName, const TSharedPtr<class FJsonObject>& Data);

	/** Handle op=11 HEARTBEAT_ACK. */
	void HandleHeartbeatAck();

	/** Send op=2 IDENTIFY to Discord. */
	void SendIdentify();

	/** Send op=1 HEARTBEAT with the current sequence number. */
	void SendHeartbeat();

	/** Start the periodic heartbeat timer. */
	void StartHeartbeat(int32 IntervalMs);

	/** Cancel the heartbeat timer. */
	void StopHeartbeat();

	// -----------------------------------------------------------------------
	// Member data
	// -----------------------------------------------------------------------

	/** Underlying WebSocket.  Kept as a UPROPERTY to prevent GC. */
	UPROPERTY()
	USMLWebSocket* WebSocket;

	/** Bot token, already normalised to "Bot TOKEN". */
	FString Token;

	/** Gateway intents bitmask passed to ConnectToDiscord(). */
	int32 GatewayIntents;

	/**
	 * Most-recent sequence number received from Discord (the "s" field).
	 * Sent with every HEARTBEAT; -1 means no sequence number yet (send null).
	 */
	int32 SequenceNumber;

	/** True after a READY event has been received and processed. */
	bool bIdentified;

	/** Timer handle for the periodic heartbeat. */
	FTimerHandle HeartbeatTimerHandle;
};
