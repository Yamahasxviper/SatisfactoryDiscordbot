#pragma once

#include "CoreMinimal.h"
#include "Interfaces/IHttpRequest.h"
#include "Dom/JsonObject.h"
#include "DiscordGatewayClient.generated.h"

/**
 * Discord Gateway Intent bit-flags (for documentation reference).
 *
 * The three privileged intents covered by this module via HTTP polling:
 *   - Presence Intent       (GuildPresences) : bit 8  = 256
 *   - Server Members Intent (GuildMembers)   : bit 1  = 2
 *   - Message Content Intent (MessageContent): bit 15 = 32768
 *
 * NOTE: These are Discord Gateway (WebSocket) concepts. Because this
 * implementation uses the Discord REST API via HTTP polling instead of
 * the WebSocket Gateway, InIntents is accepted for API compatibility
 * but has no runtime effect — access is determined by which bot
 * permissions and scopes are configured in the Discord Developer Portal.
 */
UENUM(BlueprintType, Meta = (Bitflags, UseEnumValuesAsMaskValuesInEditor = "true"))
enum class EDiscordGatewayIntent : int32
{
    None                   = 0,
    /** Non-privileged: basic guild information. */
    Guilds                 = 1,
    /** PRIVILEGED — Server Members Intent: guild member events. */
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
    /** Non-privileged: guild message events. */
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
    /** PRIVILEGED — Message Content Intent. */
    MessageContent         = 32768,
    /** Non-privileged: scheduled event events. */
    GuildScheduledEvents   = 65536,
};
ENUM_CLASS_FLAGS(EDiscordGatewayIntent);

/** Kept for source compatibility. Not used at runtime in HTTP polling mode. */
static constexpr int32 DISCORD_BOT_REQUIRED_INTENTS = 33026;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDiscordMessageReceived, const FString&, MessageContent);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDiscordPresenceUpdated, const FString&, UserId);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDiscordMemberUpdated, const FString&, UserId);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnDiscordConnected, bool, bSuccess, const FString&, ErrorMessage);

/**
 * Polls the Discord REST API to surface message, member, and presence data.
 *
 * This implementation uses UE's built-in HTTP module (confirmed available in
 * the 5.3.2-CSS engine via FactoryGame's own usage) instead of the WebSocket
 * Gateway, so it works even when the WebSockets module is absent.
 *
 * What is polled:
 *  - Messages  : GET /channels/{ChannelId}/messages?after={last_id}&limit=100
 *  - Members   : GET /guilds/{GuildId}/members?limit=1000
 *
 * The three privileged intents are obtained by configuring your bot in the
 * Discord Developer Portal (Bot → Privileged Gateway Intents).  No special
 * runtime setting is needed here because REST access is controlled by bot
 * permissions, not intents.
 *
 * C++ usage:
 *   UDiscordGatewayClient* Client = NewObject<UDiscordGatewayClient>(Outer);
 *   Client->ChannelId = TEXT("123456789");
 *   Client->GuildId   = TEXT("987654321");
 *   Client->Connect(TEXT("Bot YOUR_BOT_TOKEN"));
 *
 * Blueprint usage:
 *   Obtain via DiscordBotSubsystem → GetGatewayClient(), then set ChannelId/
 *   GuildId and call Connect.  Or use bAutoConnect with the .ini config.
 */
UCLASS(BlueprintType, Blueprintable)
class DISCORDBOT_API UDiscordGatewayClient : public UObject
{
    GENERATED_BODY()

public:
    UDiscordGatewayClient();

    // ---- Connection --------------------------------------------------------

    /**
     * Start HTTP polling of the Discord REST API.
     * @param InBotToken  Full token string, e.g. "Bot MTk4NjIy..."
     * @param InIntents   Accepted for API compatibility; not used in HTTP mode.
     */
    UFUNCTION(BlueprintCallable, Category = "Discord|Gateway")
    void Connect(const FString& InBotToken, int32 InIntents = 33026);

    /** Stop polling and clear the timer. */
    UFUNCTION(BlueprintCallable, Category = "Discord|Gateway")
    void Disconnect();

    /** Returns true while polling is active. */
    UFUNCTION(BlueprintPure, Category = "Discord|Gateway")
    bool IsConnected() const { return bPolling; }

    // ---- Target configuration ----------------------------------------------

    /**
     * Discord channel ID to poll for new messages.
     * Must be set before calling Connect(), or via Config/DefaultDiscordBot.ini.
     */
    UPROPERTY(BlueprintReadWrite, Category = "Discord|Config")
    FString ChannelId;

    /**
     * Discord guild (server) ID to poll for member and presence data.
     * Must be set before calling Connect(), or via Config/DefaultDiscordBot.ini.
     */
    UPROPERTY(BlueprintReadWrite, Category = "Discord|Config")
    FString GuildId;

    /**
     * How often (in seconds) to poll each REST endpoint.
     * Default is 5 seconds. Minimum recommended value: 1 second.
     * Respect Discord's rate limits: most endpoints allow 5 requests/second.
     */
    UPROPERTY(BlueprintReadWrite, Category = "Discord|Config")
    float PollIntervalSeconds = 5.0f;

    // ---- Delegates ---------------------------------------------------------

    /** Fired for each new message in ChannelId (requires Message Content access). */
    UPROPERTY(BlueprintAssignable, Category = "Discord|Events")
    FOnDiscordMessageReceived OnMessageReceived;

    /**
     * Fired for each member whose online_status changed since last poll
     * (best-effort from member list; requires Server Members access).
     */
    UPROPERTY(BlueprintAssignable, Category = "Discord|Events")
    FOnDiscordPresenceUpdated OnPresenceUpdated;

    /** Fired for each member returned by the guild member list poll. */
    UPROPERTY(BlueprintAssignable, Category = "Discord|Events")
    FOnDiscordMemberUpdated OnMemberUpdated;

    /** Fired after the first successful token-verification call to /users/@me. */
    UPROPERTY(BlueprintAssignable, Category = "Discord|Events")
    FOnDiscordConnected OnConnected;

private:
    // ---- HTTP helpers ------------------------------------------------------

    /** Build an authorized GET request for the given Discord API path. */
    TSharedRef<IHttpRequest, ESPMode::ThreadSafe> MakeRequest(const FString& Path) const;

    // ---- Token verification ------------------------------------------------
    void VerifyToken();
    void OnVerifyResponse(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bSucceeded);

    // ---- Poll cycle --------------------------------------------------------
    void Poll();
    void PollMessages();
    void PollMembers();

    void OnMessagesResponse(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bSucceeded);
    void OnMembersResponse(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bSucceeded);

    // ---- State -------------------------------------------------------------
    FTimerHandle PollTimerHandle;
    FString BotToken;
    FString LastMessageId;
    bool bPolling = false;
};
