// Copyright (c) 2024 Yamahasxviper

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "DiscordGatewayClient.h"
#include "DiscordChatRelay.h"
#include "DiscordBotSubsystem.generated.h"

/**
 * Game Instance Subsystem for managing Discord bot
 * This allows easy Blueprint and C++ access to the Discord bot
 */
UCLASS()
class DISCORDBOT_API UDiscordBotSubsystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    /** Called when the subsystem is initialized */
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;

    /** Called when the subsystem is deinitialized */
    virtual void Deinitialize() override;

    /** Get the Discord Gateway Client */
    UFUNCTION(BlueprintPure, Category = "Discord Bot")
    ADiscordGatewayClient* GetGatewayClient() const { return GatewayClient; }

    /** Initialize and connect the Discord bot */
    UFUNCTION(BlueprintCallable, Category = "Discord Bot")
    void InitializeAndConnect(const FString& BotToken);

    /** Disconnect the Discord bot */
    UFUNCTION(BlueprintCallable, Category = "Discord Bot")
    void DisconnectBot();

    /** Send a message to a Discord channel */
    UFUNCTION(BlueprintCallable, Category = "Discord Bot")
    void SendDiscordMessage(const FString& ChannelId, const FString& Message);

    /** Check if the bot is connected */
    UFUNCTION(BlueprintPure, Category = "Discord Bot")
    bool IsBotConnected() const;

    /** Handle Discord message received - forwards to in-game chat */
    void OnDiscordMessageReceived(const FString& ChannelId, const FString& Username, const FString& Message);

    /** Handle in-game chat message - forwards to Discord */
    void OnGameChatMessage(const FString& PlayerName, const FString& Message);

    /** Get list of configured Discord channel IDs */
    const TArray<FString>& GetChatChannelIds() const { return ChatChannelIds; }

    /** Check if two-way chat is enabled */
    bool IsTwoWayChatEnabled() const { return bTwoWayChatEnabled; }

private:
    /** The Discord Gateway Client instance */
    UPROPERTY()
    ADiscordGatewayClient* GatewayClient;

    /** Chat relay for forwarding game messages to Discord */
    UPROPERTY()
    UDiscordChatRelay* ChatRelay;

    /** Load bot token from config */
    FString LoadBotTokenFromConfig();

    /** Load two-way chat configuration from config */
    void LoadTwoWayChatConfig();

    /** Load server notification configuration from config */
    void LoadServerNotificationConfig();

    /** Send server start notification */
    void SendServerStartNotification();

    /** Send server stop notification */
    void SendServerStopNotification();

    /** Format Discord username for in-game display */
    FString FormatDiscordSender(const FString& Username) const;

    /** Format in-game player name for Discord */
    FString FormatGameSender(const FString& PlayerName) const;

    /** Two-way chat enabled flag */
    bool bTwoWayChatEnabled;

    /** List of Discord channel IDs to relay chat to/from */
    TArray<FString> ChatChannelIds;

    /** Format string for Discord sender names in-game */
    FString DiscordSenderFormat;

    /** Format string for game sender names in Discord */
    FString GameSenderFormat;

    /** Server notification enabled flag */
    bool bServerNotificationsEnabled;

    /** Notification channel ID for server start/stop events */
    FString NotificationChannelId;

    /** Custom message for server start */
    FString ServerStartMessage;

    /** Custom message for server stop */
    FString ServerStopMessage;

    /** Custom bot presence message */
    FString BotPresenceMessage;

    /** Player count update interval in seconds */
    float PlayerCountUpdateInterval;

    /** Whether to show player count in bot presence */
    bool bShowPlayerCount;

    /** Whether to show player names instead of just count */
    bool bShowPlayerNames;

    /** Maximum number of player names to show (0 = show all) */
    int32 MaxPlayerNamesToShow;

    /** Format string for player names display */
    FString PlayerNamesFormat;

    /** Timer handle for player count updates */
    FTimerHandle PlayerCountUpdateTimerHandle;

    /** Get current number of players on the server */
    int32 GetCurrentPlayerCount() const;

    /** Get list of current player names on the server */
    TArray<FString> GetCurrentPlayerNames() const;

    /** Format player names into a string for display */
    FString FormatPlayerNames(const TArray<FString>& PlayerNames) const;

    /** Update bot presence with current player count */
    void UpdateBotPresenceWithPlayerCount();
