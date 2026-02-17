// Copyright (c) 2024 Yamahasxviper

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "DiscordGatewayClientCustom.h"
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
    ADiscordGatewayClientCustom* GetGatewayClient() const { return GatewayClient; }

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

private:
    /** The Discord Gateway Client instance */
    UPROPERTY()
    ADiscordGatewayClientCustom* GatewayClient;

    /** Load bot token from config */
    FString LoadBotTokenFromConfig();
};
