#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "DiscordGatewayClient.h"
#include "DiscordBotSubsystem.generated.h"

/**
 * GameInstance subsystem that owns the Discord HTTP polling client.
 *
 * Access from Blueprints via the "Get Discord Bot Subsystem" node,
 * or in C++ via UGameInstance::GetSubsystem<UDiscordBotSubsystem>().
 *
 * Configuration (Config/DefaultDiscordBot.ini):
 *   [/Script/DiscordBot.DiscordBotSubsystem]
 *   BotToken=Bot YOUR_TOKEN_HERE
 *   GuildId=YOUR_GUILD_ID
 *   ChannelId=YOUR_CHANNEL_ID
 *   bAutoConnect=true
 */
UCLASS(Config = DiscordBot)
class DISCORDBOT_API UDiscordBotSubsystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    // UGameInstanceSubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // ---- Configuration (read from DiscordBot.ini) --------------------------

    /** Bot token including the "Bot " prefix. Set in Config/DiscordBot.ini. */
    UPROPERTY(Config, BlueprintReadOnly, Category = "Discord|Config")
    FString BotToken;

    /** Target guild (server) ID for this bot instance. */
    UPROPERTY(Config, BlueprintReadOnly, Category = "Discord|Config")
    FString GuildId;

    /**
     * Discord channel ID to poll for new messages.
     * Find it by right-clicking a channel in Discord (with Developer Mode enabled)
     * and choosing "Copy Channel ID".
     */
    UPROPERTY(Config, BlueprintReadOnly, Category = "Discord|Config")
    FString ChannelId;

    /**
     * When true the subsystem automatically calls Connect() on Initialize()
     * using the token from the config file.
     */
    UPROPERTY(Config, BlueprintReadOnly, Category = "Discord|Config")
    bool bAutoConnect = false;

    // ---- Runtime -----------------------------------------------------------

    /**
     * Connect to the Discord REST API and start polling.
     * If InBotToken is empty, uses the token from the config file.
     * InIntents is accepted for source compatibility but has no effect
     * in HTTP polling mode (access is controlled by bot permissions).
     */
    UFUNCTION(BlueprintCallable, Category = "Discord|Bot")
    void Connect(const FString& InBotToken = TEXT(""), int32 InIntents = 33026);

    /** Disconnect from the Discord Gateway. */
    UFUNCTION(BlueprintCallable, Category = "Discord|Bot")
    void Disconnect();

    /** Returns the underlying gateway client (may be null before Connect() is called). */
    UFUNCTION(BlueprintPure, Category = "Discord|Bot")
    UDiscordGatewayClient* GetGatewayClient() const { return GatewayClient; }

private:
    UPROPERTY()
    UDiscordGatewayClient* GatewayClient = nullptr;
};
