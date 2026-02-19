// Copyright (c) 2024 Yamahasxviper

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Http.h"
#include "DiscordGatewayClient.generated.h"

/**
 * Discord Gateway Client for Satisfactory
 * Connects to Discord Gateway with required intents:
 * - Presence Intent (1 << 8)
 * - Server Members Intent (1 << 1) 
 * - Message Content Intent (1 << 15)
 */
UCLASS(Blueprintable)
class DISCORDBOT_API ADiscordGatewayClient : public AActor
{
    GENERATED_BODY()

public:
    ADiscordGatewayClient();

    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
    virtual void Tick(float DeltaTime) override;

    /** Initialize the Discord bot with token */
    UFUNCTION(BlueprintCallable, Category = "Discord")
    virtual void InitializeBot(const FString& BotToken);

    /** Connect to Discord Gateway */
    UFUNCTION(BlueprintCallable, Category = "Discord")
    virtual void Connect();

    /** Disconnect from Discord Gateway */
    UFUNCTION(BlueprintCallable, Category = "Discord")
    virtual void Disconnect();

    /** Send a message to a Discord channel */
    UFUNCTION(BlueprintCallable, Category = "Discord")
    virtual void SendMessage(const FString& ChannelId, const FString& Message);

    /** Check if the bot is connected */
    UFUNCTION(BlueprintPure, Category = "Discord")
    bool IsConnected() const { return bIsConnected; }

    /** Update bot presence/status */
    UFUNCTION(BlueprintCallable, Category = "Discord")
    virtual void UpdatePresence(const FString& StatusMessage, int32 ActivityType = 0);

    /** Set the Gateway URL (used as fallback if Discord API lookup fails) */
    UFUNCTION(BlueprintCallable, Category = "Discord")
    void SetGatewayURL(const FString& InGatewayURL) { GatewayURL = InGatewayURL; }

protected:
    /** Discord bot token */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Discord")
    FString BotToken;

    /** Discord Gateway URL */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Discord")
    FString GatewayURL;

    /** Gateway intents value */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Discord")
    int32 Intents;

    /** Connection status */
    UPROPERTY(BlueprintReadOnly, Category = "Discord")
    bool bIsConnected;

    /** Heartbeat interval in milliseconds */
    UPROPERTY(BlueprintReadOnly, Category = "Discord")
    int32 HeartbeatInterval;

    /** Sequence number for gateway messages */
    UPROPERTY(BlueprintReadOnly, Category = "Discord")
    int32 SequenceNumber;

    /** Session ID */
    UPROPERTY(BlueprintReadOnly, Category = "Discord")
    FString SessionId;

private:
    /** Timer handle for heartbeat */
    FTimerHandle HeartbeatTimerHandle;

    /** HTTP module reference */
    FHttpModule* HttpModule;

    /** Get Gateway URL from Discord API */
    void GetGatewayURL();

    /** Handle Gateway URL response */
    void OnGetGatewayURLComplete(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful);

    /** Connect to WebSocket */
    void ConnectWebSocket();

    /** Send identify payload */
    void SendIdentify();

    /** Send heartbeat */
    void SendHeartbeat();

    /** Handle incoming Gateway messages */
    void OnMessageReceived(const FString& Message);

    /** Handle Gateway events */
    void HandleGatewayEvent(int32 OpCode, const TSharedPtr<FJsonObject>& Data);

    /** Send message via HTTP API */
    void SendMessageHTTP(const FString& ChannelId, const FString& MessageContent);
};
