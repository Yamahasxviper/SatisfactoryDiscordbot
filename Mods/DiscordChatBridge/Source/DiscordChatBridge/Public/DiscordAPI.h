// Copyright (c) 2024 Discord Chat Bridge Contributors

#pragma once

#include "CoreMinimal.h"
#include "Http.h"
#include "DiscordAPI.generated.h"

// Forward declarations
class UDiscordGateway;

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
	FString DiscordNameFormat;

	UPROPERTY(BlueprintReadWrite, Category = "Discord")
	FString GameNameFormat;

	UPROPERTY(BlueprintReadWrite, Category = "Discord")
	FString DiscordSourceLabel;

	UPROPERTY(BlueprintReadWrite, Category = "Discord")
	FString GameSourceLabel;

	UPROPERTY(BlueprintReadWrite, Category = "Discord")
	bool bEnableServerNotifications = false;

	UPROPERTY(BlueprintReadWrite, Category = "Discord")
	FString NotificationChannelId;

	UPROPERTY(BlueprintReadWrite, Category = "Discord")
	FString ServerStartMessage;

	UPROPERTY(BlueprintReadWrite, Category = "Discord")
	FString ServerStopMessage;

	UPROPERTY(BlueprintReadWrite, Category = "Discord")
	bool bEnableBotActivity = false;

	UPROPERTY(BlueprintReadWrite, Category = "Discord")
	FString BotActivityFormat;

	UPROPERTY(BlueprintReadWrite, Category = "Discord")
	float ActivityUpdateIntervalSeconds = 60.0f;

	UPROPERTY(BlueprintReadWrite, Category = "Discord")
	FString BotActivityChannelId;

	UPROPERTY(BlueprintReadWrite, Category = "Discord")
	bool bUseGatewayForPresence = false;

	UPROPERTY(BlueprintReadWrite, Category = "Discord")
	FString GatewayPresenceFormat;

	UPROPERTY(BlueprintReadWrite, Category = "Discord")
	int32 GatewayActivityType = 0;

	FDiscordBotConfig()
		: BotToken(TEXT(""))
		, ChannelId(TEXT(""))
		, PollIntervalSeconds(2.0f)
		, DiscordNameFormat(TEXT("[{source}] {username}"))
		, GameNameFormat(TEXT("**[{username}]** {message}"))
		, DiscordSourceLabel(TEXT("Discord"))
		, GameSourceLabel(TEXT("Game"))
		, bEnableServerNotifications(false)
		, NotificationChannelId(TEXT(""))
		, ServerStartMessage(TEXT("🟢 **Server Started** - The Satisfactory server is now online!"))
		, ServerStopMessage(TEXT("🔴 **Server Stopped** - The Satisfactory server is now offline."))
		, bEnableBotActivity(false)
		, BotActivityFormat(TEXT("🎮 **Players Online:** {playercount}"))
		, ActivityUpdateIntervalSeconds(60.0f)
		, BotActivityChannelId(TEXT(""))
		, bUseGatewayForPresence(false)
		, GatewayPresenceFormat(TEXT("with {playercount} players"))
		, GatewayActivityType(0)
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

	/** Send a notification message to the notification channel (or chat channel if not configured) */
	void SendNotification(const FString& Message);

	/** Start polling for new messages from Discord */
	void StartPolling();

	/** Stop polling for messages */
	void StopPolling();

	/** Start updating bot activity status */
	void StartActivityUpdates();

	/** Stop updating bot activity status */
	void StopActivityUpdates();

	/** Update bot activity status with player count */
	void UpdateBotActivity(int32 PlayerCount);

	/** Check if the API is initialized and ready */
	bool IsInitialized() const { return bIsInitialized; }

	/** Delegate called when a new message is received from Discord */
	FOnDiscordMessageReceived_Full OnMessageReceived;

private:
	/** Poll Discord for new messages */
	void PollMessages();

	/** Handle response from sending a message */
	void OnSendMessageResponse(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful);

	/** Handle response from sending a notification */
	void OnSendNotificationResponse(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful);

	/** Handle response from polling messages */
	void OnPollMessagesResponse(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful);

	/** Handle response from updating bot activity */
	void OnUpdateActivityResponse(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful);

	/** Gateway event handlers */
	void OnGatewayConnected();
	void OnGatewayDisconnected(const FString& Reason);

	/** Discord bot configuration */
	FDiscordBotConfig BotConfig;

	/** Discord Gateway connection (for bot presence) */
	UPROPERTY()
	UDiscordGateway* Gateway;

	/** Timer handle for message polling */
	FTimerHandle PollTimerHandle;

	/** Timer handle for activity updates */
	FTimerHandle ActivityUpdateTimerHandle;

	/** ID of the last message we've seen */
	FString LastMessageId;

	/** Whether the API is initialized */
	bool bIsInitialized;

	/** Whether we're currently polling */
	bool bIsPolling;

	/** Whether we're currently updating activity */
	bool bIsUpdatingActivity;
};
