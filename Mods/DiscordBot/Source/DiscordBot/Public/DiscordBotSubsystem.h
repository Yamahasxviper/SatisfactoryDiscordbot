#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "DiscordGatewayClient.h"
#include "DiscordBotSubsystem.generated.h"

/**
 * GameInstance subsystem that owns the Discord Gateway client.
 *
 * Access from Blueprints via the "Get Discord Bot Subsystem" node,
 * or in C++ via UGameInstance::GetSubsystem<UDiscordBotSubsystem>().
 *
 * Configuration (Config/DiscordBot.ini):
 *   [/Script/DiscordBot.DiscordBotSubsystem]
 *   BotToken=Bot YOUR_TOKEN_HERE
 *   GuildId=YOUR_GUILD_ID
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
     * When true the subsystem automatically calls Connect() on Initialize()
     * using the token from the config file.
     */
    UPROPERTY(Config, BlueprintReadOnly, Category = "Discord|Config")
    bool bAutoConnect = false;

    // ---- Runtime -----------------------------------------------------------

    /**
     * Connect to the Discord Gateway.
     * If InBotToken is empty, uses the token from the config file.
     * InIntents defaults to 33026:
     *   GuildMembers (2) | GuildPresences (256) | MessageContent (32768).
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
