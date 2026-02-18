// Copyright (c) 2024 Yamahasxviper
// Discord Gateway Client using Custom WebSocket

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CustomWebSocket.h"
#include "Http.h"
#include "DiscordGatewayClientCustom.generated.h"

/**
 * Discord Gateway Client using Custom WebSocket Implementation
 * 
 * This client uses a fully custom WebSocket that:
 * - Works on ALL platforms (Win64, Linux, Mac, Dedicated Servers)
 * - No dependency on Unreal's WebSocket module
 * - Only requires Sockets and OpenSSL (always available)
 * - Full Discord Gateway protocol support
 * - RFC 6455 compliant
 * 
 * Features:
 * - All Discord Gateway opcodes
 * - Automatic heartbeat management
 * - Session resumption
 * - Reconnection with exponential backoff
 * - All required intents (Presence, Server Members, Message Content)
 * - Event handling
 * - Platform agnostic
 * 
 * Use this implementation when:
 * - Native WebSocket module not available
 * - Need guaranteed cross-platform support
 * - Want full control over WebSocket behavior
 * - Need fallback option
 */
UCLASS(Blueprintable)
class DISCORDBOT_API ADiscordGatewayClientCustom : public AActor
{
    GENERATED_BODY()

public:
    ADiscordGatewayClientCustom();

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

    /** Set custom intents */
    UFUNCTION(BlueprintCallable, Category = "Discord")
    void SetIntents(int32 CustomIntents) { Intents = CustomIntents; }

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

    /** Resume Gateway URL */
    UPROPERTY(BlueprintReadOnly, Category = "Discord")
    FString ResumeGatewayURL;

private:
    /** Custom WebSocket connection */
    TSharedPtr<FCustomWebSocket> WebSocket;

    /** Timer handle for heartbeat */
    FTimerHandle HeartbeatTimerHandle;

    /** HTTP module reference */
    FHttpModule* HttpModule;

    /** Heartbeat tracking */
    bool bHeartbeatAckReceived;
    float LastHeartbeatTime;

    /** Reconnection */
    int32 ReconnectAttempts;
    float ReconnectDelay;

    /** Get Gateway URL from Discord API */
    void GetGatewayURL();

    /** Handle Gateway URL response */
    void OnGetGatewayURLComplete(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful);

    /** Connect to WebSocket */
    void ConnectWebSocket();

    /** Send identify payload */
    void SendIdentify();

    /** Send resume payload */
    void SendResume();

    /** Send heartbeat */
    void SendHeartbeat();

    /** WebSocket event handlers */
    void OnWebSocketConnected(bool bSuccess);
    void OnWebSocketMessage(const FString& Message, bool bIsText);
    void OnWebSocketClosed(int32 StatusCode, const FString& Reason, bool bWasClean);
    void OnWebSocketError(const FString& Error);

    /** Handle Gateway events */
    void HandleGatewayEvent(int32 OpCode, const TSharedPtr<FJsonObject>& Data, const FString& EventType);

    /** Send message via HTTP API */
    void SendMessageHTTP(const FString& ChannelId, const FString& MessageContent);

    /** Reconnection logic */
    void AttemptReconnect();
};
