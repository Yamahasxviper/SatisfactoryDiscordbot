// Copyright (c) 2024 Yamahasxviper

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "IWebSocket.h"
#include "Http.h"
#include "DiscordGatewayClientNative.generated.h"

/**
 * Discord Gateway Client using Native Unreal WebSocket
 * This implementation uses Unreal's built-in WebSocket module which is:
 * - Guaranteed compatible with CSS Unreal Engine 5.3.2
 * - Implements proper WebSocket protocol (not Socket.IO)
 * - Lighter weight and better maintained
 * 
 * Connects to Discord Gateway with required intents:
 * - Presence Intent (1 << 8)
 * - Server Members Intent (1 << 1) 
 * - Message Content Intent (1 << 15)
 */
UCLASS(Blueprintable)
class DISCORDBOT_API ADiscordGatewayClientNative : public AActor
{
    GENERATED_BODY()

public:
    ADiscordGatewayClientNative();

    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
    virtual void Tick(float DeltaTime) override;

    /** Initialize the Discord bot with token */
    UFUNCTION(BlueprintCallable, Category = "Discord")
    void InitializeBot(const FString& BotToken);

    /** Connect to Discord Gateway */
    UFUNCTION(BlueprintCallable, Category = "Discord")
    void Connect();

    /** Disconnect from Discord Gateway */
    UFUNCTION(BlueprintCallable, Category = "Discord")
    void Disconnect();

    /** Send a message to a Discord channel */
    UFUNCTION(BlueprintCallable, Category = "Discord")
    void SendMessage(const FString& ChannelId, const FString& Message);

    /** Check if the bot is connected */
    UFUNCTION(BlueprintPure, Category = "Discord")
    bool IsConnected() const { return bIsConnected; }

    /** Update bot presence/status */
    UFUNCTION(BlueprintCallable, Category = "Discord")
    void UpdatePresence(const FString& StatusMessage, int32 ActivityType = 0);

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

    /** Bot User ID - set after READY event */
    UPROPERTY(BlueprintReadOnly, Category = "Discord")
    FString BotUserId;

private:
    /** Native WebSocket connection */
    TSharedPtr<IWebSocket> WebSocket;

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

    /** WebSocket event handlers */
    void OnWebSocketConnected();
    void OnWebSocketConnectionError(const FString& Error);
    void OnWebSocketClosed(int32 StatusCode, const FString& Reason, bool bWasClean);
    void OnWebSocketMessage(const FString& Message);

    /** Handle Gateway events */
    void HandleGatewayEvent(int32 OpCode, const TSharedPtr<FJsonObject>& Data, const TSharedPtr<FJsonObject>& RootJsonObject);

    /** Handle MESSAGE_CREATE event */
    void HandleMessageCreate(const TSharedPtr<FJsonObject>& Data);

    /** Send message via HTTP API */
    void SendMessageHTTP(const FString& ChannelId, const FString& MessageContent);
};
