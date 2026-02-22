// Copyright Coffee Stain Studios. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Containers/Ticker.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Interfaces/IHttpRequest.h"
#include "Interfaces/IHttpResponse.h"
#include "DiscordBridgeConfig.h"
#include "DiscordBridgeSubsystem.generated.h"

class AFGChatManager;

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
 * Fired on the game thread once the first REST API poll succeeds and the bridge
 * is confirmed active (bot token is valid, channel is reachable).
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FDiscordConnectedDelegate);

/**
 * Fired on the game thread when polling is stopped (Disconnect() called or
 * a terminal API error is encountered).
 *
 * @param Reason  Human-readable description of why polling stopped.
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FDiscordDisconnectedDelegate,
                                            const FString&, Reason);

/**
 * UDiscordBridgeSubsystem
 *
 * A GameInstance-level subsystem that bridges Satisfactory in-game chat with
 * a Discord text channel using the Discord REST API only.
 *
 * How it works
 * ────────────
 *  • On startup the subsystem fetches the bot's own user-ID via
 *    GET /users/@me so self-sent messages can be filtered out.
 *  • It then performs a one-shot GET /channels/{id}/messages?limit=1 to
 *    establish a "baseline" snowflake – the ID of the newest message that
 *    already existed before the server started.  No old messages are relayed.
 *  • A repeating FTSTicker fires every PollIntervalSeconds and calls
 *    GET /channels/{id}/messages?after={last_id}&limit=100.
 *    Any new messages are broadcast through OnDiscordMessageReceived and
 *    automatically relayed to Satisfactory in-game chat.
 *  • Game → Discord: SendGameMessageToDiscord() posts via
 *    POST /channels/{id}/messages (Discord REST API v10).
 *
 * Discord bot requirements
 * ────────────────────────
 *  • Message Content Intent (GUILD_MESSAGES / MESSAGE_CONTENT) MUST be enabled
 *    in the Discord Developer Portal → Bot → Privileged Gateway Intents so that
 *    the REST API returns non-empty content fields.
 *  • Presence Intent and Server Members Intent may still be enabled on the bot
 *    account; they are harmless in REST-only mode and allow future Gateway use.
 *  • The bot must have "Send Messages" and "Read Message History" permissions
 *    in the target channel.
 *
 * Setup
 * ─────
 *  1. Create a Discord application and bot in the Discord Developer Portal.
 *  2. Enable Message Content Intent (Bot → Privileged Gateway Intents).
 *  3. Invite the bot to your server with "Send Messages" + "Read Message History".
 *  4. Fill in BotToken and ChannelId in Saved/Config/DiscordBridge.ini
 *     (auto-created on first server start) and restart the server.
 *  5. Optionally tune PollIntervalSeconds, GameToDiscordFormat, DiscordToGameFormat.
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

	/** Fired once the first successful REST poll confirms the bot token is valid. */
	UPROPERTY(BlueprintAssignable, Category="Discord Bridge")
	FDiscordConnectedDelegate OnDiscordConnected;

	/** Fired when polling is stopped (Disconnect() called or terminal error). */
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
	 * Start polling the Discord REST API for new messages.
	 * Called automatically during Initialize() when BotToken and ChannelId are set.
	 */
	UFUNCTION(BlueprintCallable, Category="Discord Bridge")
	void Connect();

	/**
	 * Stop polling and fire OnDiscordDisconnected.
	 * Called automatically during Deinitialize().
	 */
	UFUNCTION(BlueprintCallable, Category="Discord Bridge")
	void Disconnect();

	/**
	 * Returns true while the poll ticker is active and the first REST poll has
	 * confirmed the bot token is valid.
	 */
	UFUNCTION(BlueprintPure, Category="Discord Bridge")
	bool IsConnected() const { return bPollingActive; }

private:
	// ── REST poll helpers ─────────────────────────────────────────────────────

	/**
	 * Step 1 (called once from Connect).
	 * GET /users/@me to learn the bot's own snowflake ID so we can skip
	 * messages the bot itself posted.
	 */
	void FetchBotUserId();

	/**
	 * Step 2 (called from FetchBotUserId callback, or directly if the fetch
	 * failed).  GET /channels/{id}/messages?limit=1 to establish the baseline
	 * message ID.  No messages seen before server start are relayed.
	 */
	void FetchBaselineMessageId();

	/**
	 * Repeating ticker body – calls PollNewMessages() every PollIntervalSeconds.
	 * Returns true to keep ticking.
	 */
	bool PollTick(float DeltaTime);

	/**
	 * GET /channels/{id}/messages?after={LastMessageId}&limit=100.
	 * Processes each new message and updates LastMessageId.
	 */
	void PollNewMessages();

	/**
	 * Process the JSON array returned by the messages endpoint.
	 * Fires OnDiscordMessageReceived for each qualifying message.
	 * Returns the snowflake ID of the newest message seen (empty if none).
	 */
	FString ProcessMessageArray(const TArray<TSharedPtr<FJsonValue>>& Messages);

	// ── REST API sending ──────────────────────────────────────────────────────

	/** POST a plain text message to the configured Discord channel. */
	void SendStatusMessageToDiscord(const FString& Message);

	/** Build a common Bot-authorised HTTP request (verb + auth header). */
	TSharedRef<IHttpRequest, ESPMode::ThreadSafe> MakeApiRequest(
		const FString& Verb, const FString& Url) const;

	// ── Chat-manager binding ──────────────────────────────────────────────────

	/**
	 * Automatically relays an incoming Discord message to the Satisfactory
	 * in-game chat via AFGChatManager::BroadcastChatMessage.
	 * Bound to OnDiscordMessageReceived in Initialize().
	 */
	UFUNCTION()
	void RelayDiscordMessageToGame(const FString& Username, const FString& Message);

	/**
	 * Called by AFGChatManager::OnChatMessageAdded each time a new in-game chat
	 * message arrives.  Forwards CMT_PlayerMessage entries to Discord.
	 */
	UFUNCTION()
	void OnNewChatMessage();

	/** Ticker that polls for AFGChatManager every second until it is available. */
	FTSTicker::FDelegateHandle ChatManagerBindTickerHandle;

	/** Cached pointer to the ChatManager we have bound OnNewChatMessage to. */
	UPROPERTY()
	AFGChatManager* BoundChatManager{nullptr};

	/** Number of chat messages already inspected (prevents replay on rebind). */
	int32 NumSeenChatMessages{0};

	// ── Polling state ─────────────────────────────────────────────────────────

	/** Repeating ticker that drives PollNewMessages(). */
	FTSTicker::FDelegateHandle PollTickerHandle;

	/**
	 * Snowflake ID of the newest Discord message seen so far.
	 * Passed as the `after` parameter on every poll so only new messages are
	 * returned.  Empty until the baseline fetch completes.
	 */
	FString LastMessageId;

	/**
	 * True once the first REST poll has returned HTTP 200, confirming the bot
	 * token is valid and the channel is reachable.
	 */
	bool bPollingActive{false};

	/**
	 * True once the baseline fetch has finished and regular polling has started.
	 * Guards against ProcessMessageArray being called before the baseline is set.
	 */
	bool bBaselineEstablished{false};

	// ── Shared state ──────────────────────────────────────────────────────────

	/** Loaded configuration (populated in Initialize()). */
	FDiscordBridgeConfig Config;

	/**
	 * Snowflake ID of this bot's own user account.
	 * Populated by FetchBotUserId(); used to suppress echo of the bot's own posts.
	 */
	FString BotUserId;
};
