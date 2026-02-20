// Copyright Yamahasxviper. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "DiscordBotWebSocket.h"
#include "DiscordBotSubsystem.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogDiscordBotSubsystem, Log, All);

/**
 * UDiscordBotSubsystem
 *
 * GameInstance subsystem that owns the Discord bot WebSocket connection for the
 * lifetime of the game session. Configure the bot token and intents in Project
 * Settings → Discord Bot, or call InitializeBot() at runtime.
 *
 * This subsystem is accessible from C++ and Blueprint via
 *   GetGameInstance()->GetSubsystem<UDiscordBotSubsystem>()
 * or the Blueprint node "Get Discord Bot Subsystem".
 */
UCLASS(BlueprintType)
class DISCORDBOT_API UDiscordBotSubsystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    /**
     * Initialize and connect the Discord bot with the given credentials.
     * Call this once after the game instance is ready.
     *
     * @param BotToken  Discord bot token (without the "Bot " prefix).
     * @param Intents   Discord Gateway intents bitmask (default: all privileged + non-privileged).
     */
    UFUNCTION(BlueprintCallable, Category = "Discord|Bot")
    void InitializeBot(const FString& BotToken, int32 Intents = 3276799);

    /** Disconnect the bot from Discord. */
    UFUNCTION(BlueprintCallable, Category = "Discord|Bot")
    void DisconnectBot();

    /** Returns the active WebSocket wrapper, or nullptr if not yet initialized. */
    UFUNCTION(BlueprintPure, Category = "Discord|Bot")
    UDiscordBotWebSocket* GetWebSocket() const { return DiscordWebSocket; }

    /** Returns true if the WebSocket is connected and the bot is identified. */
    UFUNCTION(BlueprintPure, Category = "Discord|Bot")
    bool IsConnected() const;

private:
    UPROPERTY()
    UDiscordBotWebSocket* DiscordWebSocket;
};
