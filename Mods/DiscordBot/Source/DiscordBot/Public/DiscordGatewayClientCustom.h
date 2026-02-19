// Copyright (c) 2024 Yamahasxviper
// Discord Gateway Client using Custom WebSocket

#pragma once

#include "CoreMinimal.h"
#include "DiscordGatewayClient.h"
#include "CustomWebSocket.h"
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
class DISCORDBOT_API ADiscordGatewayClientCustom : public ADiscordGatewayClient
{
    GENERATED_BODY()

public:
    ADiscordGatewayClientCustom();

    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
    virtual void Tick(float DeltaTime) override;

    /** Initialize the Discord bot with token */
    virtual void InitializeBot(const FString& BotToken) override;

    /** Connect to Discord Gateway */
    virtual void Connect() override;

    /** Disconnect from Discord Gateway */
    virtual void Disconnect() override;

    /** Send a message to a Discord channel */
    virtual void SendMessage(const FString& ChannelId, const FString& Message) override;

    /** Set custom intents */
    UFUNCTION(BlueprintCallable, Category = "Discord")
    void SetIntents(int32 CustomIntents) { Intents = CustomIntents; }

    /** Update bot presence/status */
    virtual void UpdatePresence(const FString& StatusMessage, int32 ActivityType = 0) override;

protected:
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
    int32 HeartbeatCount;
    float ConnectionStartTime;

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
