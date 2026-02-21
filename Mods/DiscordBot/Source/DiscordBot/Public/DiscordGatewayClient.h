#pragma once

#include "CoreMinimal.h"
#include "IWebSocket.h"
#include "Dom/JsonObject.h"
#include "DiscordGatewayClient.generated.h"

/**
 * Discord Gateway Intent bit-flags.
 *
 * The three privileged intents required by this module:
 *   - Presence Intent       (GuildPresences) : bit 8  = 256
 *   - Server Members Intent (GuildMembers)   : bit 1  = 2
 *   - Message Content Intent (MessageContent): bit 15 = 32768
 *
 * Combined value passed in the IDENTIFY payload: 33026
 *
 * IMPORTANT: All three are PRIVILEGED intents and must be enabled in the
 * Discord Developer Portal for your application before they will work.
 */
UENUM(BlueprintType, Meta = (Bitflags, UseEnumValuesAsMaskValuesInEditor = "true"))
enum class EDiscordGatewayIntent : int32
{
    None                   = 0,
    /** Non-privileged: basic guild information. */
    Guilds                 = 1,
    /** PRIVILEGED — Server Members Intent: guild member add/update/remove events. */
    GuildMembers           = 2,
    /** Non-privileged: guild ban/unban events. */
    GuildModeration        = 4,
    /** Non-privileged: emoji and sticker updates. */
    GuildEmojisAndStickers = 8,
    /** Non-privileged: integration events. */
    GuildIntegrations      = 16,
    /** Non-privileged: webhook events. */
    GuildWebhooks          = 32,
    /** Non-privileged: invite create/delete events. */
    GuildInvites           = 64,
    /** Non-privileged: voice state events. */
    GuildVoiceStates       = 128,
    /** PRIVILEGED — Presence Intent: presence update events. */
    GuildPresences         = 256,
    /** Non-privileged: guild message create/update/delete (content excluded without MessageContent). */
    GuildMessages          = 512,
    /** Non-privileged: guild message reaction events. */
    GuildMessageReactions  = 1024,
    /** Non-privileged: guild message typing events. */
    GuildMessageTyping     = 2048,
    /** Non-privileged: direct message events. */
    DirectMessages         = 4096,
    /** Non-privileged: DM reaction events. */
    DirectMessageReactions = 8192,
    /** Non-privileged: DM typing events. */
    DirectMessageTyping    = 16384,
    /** PRIVILEGED — Message Content Intent: exposes message content in MESSAGE_CREATE/UPDATE events. */
    MessageContent         = 32768,
    /** Non-privileged: scheduled event events. */
    GuildScheduledEvents   = 65536,
};
ENUM_CLASS_FLAGS(EDiscordGatewayIntent);

/**
 * The combined gateway intents bitmask for this bot:
 *   GuildMembers (2) | GuildPresences (256) | MessageContent (32768) = 33026
 */
static constexpr int32 DISCORD_BOT_REQUIRED_INTENTS = 33026;

/** Discord Gateway op-codes (https://discord.com/developers/docs/topics/opcodes-and-status-codes). */
namespace EDiscordOpCode
{
    static constexpr int32 Dispatch     = 0;
    static constexpr int32 Heartbeat    = 1;
    static constexpr int32 Identify     = 2;
    static constexpr int32 Hello        = 10;
    static constexpr int32 HeartbeatAck = 11;
}

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDiscordMessageReceived, const FString&, MessageContent);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDiscordPresenceUpdated, const FString&, UserId);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDiscordMemberUpdated, const FString&, UserId);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnDiscordConnected, bool, bSuccess, const FString&, ErrorMessage);

/**
 * Manages the connection to the Discord Gateway WebSocket endpoint.
 *
 * Features:
 *  - Connects to wss://gateway.discord.gg (v10, JSON encoding)
 *  - Sends IDENTIFY with the three privileged gateway intents
 *  - Maintains the heartbeat loop required by Discord
 *  - Fires Blueprint-assignable delegates for incoming events
 *
 * C++ usage:
 *   UDiscordGatewayClient* Client = NewObject<UDiscordGatewayClient>(Outer);
 *   Client->Connect(TEXT("Bot YOUR_BOT_TOKEN"));
 *
 * Blueprint usage:
 *   Obtain a reference via the DiscordBotSubsystem, then call Connect / Disconnect.
 */
UCLASS(BlueprintType, Blueprintable)
class DISCORDBOT_API UDiscordGatewayClient : public UObject
{
    GENERATED_BODY()

public:
    UDiscordGatewayClient();

    // ---- Connection --------------------------------------------------------

    /**
     * Open a WebSocket connection to the Discord Gateway and authenticate.
     * @param InBotToken  Full token string, e.g. "Bot MTk4NjIy..."
     * @param InIntents   Gateway intents bitmask. Defaults to 33026
     *                    (GuildMembers | GuildPresences | MessageContent).
     */
    UFUNCTION(BlueprintCallable, Category = "Discord|Gateway")
    void Connect(const FString& InBotToken, int32 InIntents = 33026);

    /** Close the WebSocket connection gracefully. */
    UFUNCTION(BlueprintCallable, Category = "Discord|Gateway")
    void Disconnect();

    /** Returns true while the WebSocket is open and the bot is identified. */
    UFUNCTION(BlueprintPure, Category = "Discord|Gateway")
    bool IsConnected() const { return bIdentified; }

    // ---- Delegates ---------------------------------------------------------

    /** Fired when a MESSAGE_CREATE event is received (requires MessageContent intent). */
    UPROPERTY(BlueprintAssignable, Category = "Discord|Events")
    FOnDiscordMessageReceived OnMessageReceived;

    /** Fired when a PRESENCE_UPDATE event is received (requires GuildPresences intent). */
    UPROPERTY(BlueprintAssignable, Category = "Discord|Events")
    FOnDiscordPresenceUpdated OnPresenceUpdated;

    /** Fired when a GUILD_MEMBER_ADD/UPDATE/REMOVE event is received (requires GuildMembers intent). */
    UPROPERTY(BlueprintAssignable, Category = "Discord|Events")
    FOnDiscordMemberUpdated OnMemberUpdated;

    /** Fired when the connection attempt completes (success or failure). */
    UPROPERTY(BlueprintAssignable, Category = "Discord|Events")
    FOnDiscordConnected OnConnected;

private:
    // WebSocket callbacks
    void OnWebSocketConnected();
    void OnWebSocketConnectionError(const FString& Error);
    void OnWebSocketClosed(int32 StatusCode, const FString& Reason, bool bWasClean);
    void OnWebSocketMessage(const FString& Message);

    // Gateway logic
    void SendHeartbeat();
    void SendIdentify();
    void HandleDispatch(const FString& EventName, const TSharedPtr<FJsonObject>& Data);

    TSharedPtr<IWebSocket> WebSocket;
    FTimerHandle HeartbeatTimerHandle;
    FString BotToken;
    int32 Intents = DISCORD_BOT_REQUIRED_INTENTS;
    TOptional<int32> LastSequenceNumber;
    bool bIdentified = false;
};
