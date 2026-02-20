// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystem/ModSubsystem.h"
#include "FGChatManager.h"
#include "Interfaces/IHttpRequest.h"
#include "Interfaces/IHttpResponse.h"
#include "DiscordBotSubsystem.generated.h"

/**
 * Server-only subsystem that bridges Satisfactory in-game chat with a Discord
 * channel using the Discord HTTP REST API.
 *
 * Game → Discord:
 *   Every CMT_PlayerMessage that appears in the ChatManager is forwarded to the
 *   configured Discord channel via a POST to the Discord messages endpoint.
 *
 * Discord → Game:
 *   A timer polls the Discord channel every PollingIntervalSeconds seconds.
 *   New messages (those with a snowflake ID greater than the last one seen) are
 *   broadcast to all connected clients through AFGChatManager::BroadcastChatMessage.
 *
 * Configure in <GameDir>/Configs/DiscordBot.ini (or via Project Settings):
 *   BotToken          – Discord bot token (required).
 *   ChannelId         – Target channel snowflake ID (required).
 *   InGameDisplayName – Label shown in game for Discord messages (default: "Discord").
 *   PollingIntervalSeconds – Poll frequency in seconds (default: 5, min: 2).
 */
UCLASS()
class DISCORDBOT_API ADiscordBotSubsystem : public AModSubsystem
{
	GENERATED_BODY()

public:
	ADiscordBotSubsystem();

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
	/** Fires whenever the ChatManager records a new message locally (server-side). */
	UFUNCTION()
	void OnChatMessageAdded();

	/** Timer callback: poll Discord for messages posted after LastSeenMessageId. */
	void PollDiscordMessages();

	/**
	 * HTTP response handler for Discord polling requests.
	 * Parses the JSON message array and calls DisplayInGame() for each new message.
	 */
	void HandlePollResponse(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful);

	/**
	 * POST a player's chat message to the configured Discord channel.
	 * @param SenderName  In-game display name of the player.
	 * @param MessageText Text typed by the player.
	 */
	void ForwardToDiscord(const FString& SenderName, const FString& MessageText);

	/**
	 * Broadcast a Discord message to all in-game clients via AFGChatManager.
	 * @param AuthorName Discord username of the message author.
	 * @param Content    Message text received from Discord.
	 */
	void DisplayInGame(const FString& AuthorName, const FString& Content);

	/** Returns the "Bot <token>" Authorization header value. */
	FString GetAuthHeader() const;

	// -------------------------------------------------------------------------
	// State
	// -------------------------------------------------------------------------

	/** Repeating timer for Discord polling. */
	FTimerHandle PollingTimerHandle;

	/**
	 * Snowflake ID of the most recent Discord message we have already displayed.
	 * Empty string means "not yet initialised" – the first poll will populate it
	 * without showing any messages (to avoid replaying old history on startup).
	 */
	FString LastSeenMessageId;

	/**
	 * Number of entries in the ChatManager's received-messages array the last time
	 * OnChatMessageAdded ran.  Used to find newly appended messages efficiently.
	 */
	int32 LastProcessedMessageCount;

	/**
	 * Set to true while we are injecting a Discord message into in-game chat so
	 * that OnChatMessageAdded does not echo it back to Discord.
	 */
	bool bInjecting;
};
