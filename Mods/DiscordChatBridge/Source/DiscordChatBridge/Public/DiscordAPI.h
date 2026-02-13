// Copyright (c) 2024 Discord Chat Bridge Contributors

#pragma once

#include "CoreMinimal.h"
#include "Http.h"
#include "DiscordAPI.generated.h"

DECLARE_DELEGATE_OneParam(FOnDiscordMessageReceived, const FString& /* Message */);
DECLARE_DELEGATE_TwoParams(FOnDiscordMessageReceived_Full, const FString& /* Username */, const FString& /* Message */);

/**
 * Configuration structure for Discord bot settings
 */
USTRUCT(BlueprintType)
struct DISCORDCHATBRIDGE_API FDiscordBotConfig
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, Category = "Discord")
	FString BotToken;

	UPROPERTY(BlueprintReadWrite, Category = "Discord")
	FString ChannelId;

	UPROPERTY(BlueprintReadWrite, Category = "Discord")
	float PollIntervalSeconds = 2.0f;

	UPROPERTY(BlueprintReadWrite, Category = "Discord")
	bool bEnableGameToDiscord = true;

	UPROPERTY(BlueprintReadWrite, Category = "Discord")
	bool bEnableDiscordToGame = true;

	UPROPERTY(BlueprintReadWrite, Category = "Discord")
	FString GameMessageFormat;

	UPROPERTY(BlueprintReadWrite, Category = "Discord")
	FString DiscordMessageFormat;

	UPROPERTY(BlueprintReadWrite, Category = "Discord")
	FLinearColor DiscordMessageColor;

	UPROPERTY(BlueprintReadWrite, Category = "Discord")
	int32 LogVerbosity = 3;

	FDiscordBotConfig()
		: BotToken(TEXT(""))
		, ChannelId(TEXT(""))
		, PollIntervalSeconds(2.0f)
		, bEnableGameToDiscord(true)
		, bEnableDiscordToGame(true)
		, GameMessageFormat(TEXT("**[{username}]** {message}"))
		, DiscordMessageFormat(TEXT("[Discord] {username}"))
		, DiscordMessageColor(FLinearColor(0.447f, 0.627f, 0.957f))
		, LogVerbosity(3)
	{}
};

/**
 * Wrapper class for Discord API communication
 */
UCLASS()
class DISCORDCHATBRIDGE_API UDiscordAPI : public UObject
{
	GENERATED_BODY()

public:
	UDiscordAPI();

	/** Initialize the Discord API with configuration */
	void Initialize(const FDiscordBotConfig& Config);

	/** Send a message to the configured Discord channel */
	void SendMessage(const FString& Username, const FString& Message);

	/** Start polling for new messages from Discord */
	void StartPolling();

	/** Stop polling for messages */
	void StopPolling();

	/** Check if the API is initialized and ready */
	bool IsInitialized() const { return bIsInitialized; }

	/** Delegate called when a new message is received from Discord */
	FOnDiscordMessageReceived_Full OnMessageReceived;

private:
	/** Poll Discord for new messages */
	void PollMessages();

	/** Handle response from sending a message */
	void OnSendMessageResponse(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful);

	/** Handle response from polling messages */
	void OnPollMessagesResponse(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful);

	/** Discord bot configuration */
	FDiscordBotConfig BotConfig;

	/** Timer handle for message polling */
	FTimerHandle PollTimerHandle;

	/** ID of the last message we've seen */
	FString LastMessageId;

	/** Whether the API is initialized */
	bool bIsInitialized;

	/** Whether we're currently polling */
	bool bIsPolling;
};
