// Copyright Yamahasxviper. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "DiscordWebSocketClient.h"
#include "DiscordBotWebSocket.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogDiscordBotWebSocket, Log, All);

/** Discord Gateway opcodes */
UENUM(BlueprintType)
enum class EDiscordGatewayOpcode : uint8
{
    Dispatch          = 0  UMETA(DisplayName = "Dispatch"),
    Heartbeat         = 1  UMETA(DisplayName = "Heartbeat"),
    Identify          = 2  UMETA(DisplayName = "Identify"),
    PresenceUpdate    = 3  UMETA(DisplayName = "Presence Update"),
    VoiceStateUpdate  = 4  UMETA(DisplayName = "Voice State Update"),
    Resume            = 6  UMETA(DisplayName = "Resume"),
    Reconnect         = 7  UMETA(DisplayName = "Reconnect"),
    RequestMembers    = 8  UMETA(DisplayName = "Request Guild Members"),
    InvalidSession    = 9  UMETA(DisplayName = "Invalid Session"),
    Hello             = 10 UMETA(DisplayName = "Hello"),
    HeartbeatAck      = 11 UMETA(DisplayName = "Heartbeat ACK"),
};

/** Connection state of the WebSocket */
UENUM(BlueprintType)
enum class EDiscordWebSocketState : uint8
{
    Disconnected  UMETA(DisplayName = "Disconnected"),
    Connecting    UMETA(DisplayName = "Connecting"),
    Connected     UMETA(DisplayName = "Connected"),
    Resuming      UMETA(DisplayName = "Resuming"),
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnDiscordConnected);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDiscordMessage, const FString&, EventName);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDiscordError, const FString&, ErrorMessage);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnDiscordClosed, int32, StatusCode, const FString&, Reason);

/**
 * UDiscordBotWebSocket
 *
 * Implements the Discord Gateway v10 protocol over a custom WebSocket client
 * built from Unreal Engine's Sockets + SSL modules (no WebSockets plugin needed).
 *
 * Usage:
 *   1. Create via UDiscordBotWebSocket::Create(BotToken, Intents).
 *   2. Bind to OnConnected, OnDiscordEvent, OnError, OnClosed delegates.
 *   3. Call Connect() to open the connection.
 *   4. Call Disconnect() when done.
 */
UCLASS(BlueprintType, Blueprintable)
class DISCORDBOT_API UDiscordBotWebSocket : public UObject
{
    GENERATED_BODY()

public:
    UDiscordBotWebSocket();
    virtual ~UDiscordBotWebSocket() override;

    /**
     * Create a new UDiscordBotWebSocket instance (not yet connected).
     * @param WorldContextObject  Used to resolve the outer world for timers.
     * @param BotToken            Discord bot token (without the "Bot " prefix).
     * @param Intents             Gateway intents bitmask (default: all standard intents).
     */
    UFUNCTION(BlueprintCallable, Category = "Discord|WebSocket", meta = (WorldContext = "WorldContextObject"))
    static UDiscordBotWebSocket* Create(UObject* WorldContextObject, const FString& BotToken, int32 Intents = 3276799);

    /** Open the WebSocket connection to the Discord Gateway. */
    UFUNCTION(BlueprintCallable, Category = "Discord|WebSocket")
    void Connect();

    /** Close the WebSocket connection gracefully. */
    UFUNCTION(BlueprintCallable, Category = "Discord|WebSocket")
    void Disconnect();

    /**
     * Send a raw JSON payload string to the Discord Gateway.
     * Prefer the typed helpers (SendPresenceUpdate, etc.) where possible.
     */
    UFUNCTION(BlueprintCallable, Category = "Discord|WebSocket")
    void SendRawPayload(const FString& JsonPayload);

    /** Update the bot's presence / status. */
    UFUNCTION(BlueprintCallable, Category = "Discord|WebSocket")
    void SendPresenceUpdate(const FString& Status, const FString& ActivityName, int32 ActivityType = 0);

    /** Returns the current connection state. */
    UFUNCTION(BlueprintPure, Category = "Discord|WebSocket")
    EDiscordWebSocketState GetConnectionState() const { return ConnectionState; }

    /** Fired when the WebSocket successfully connects and the bot is identified. */
    UPROPERTY(BlueprintAssignable, Category = "Discord|WebSocket")
    FOnDiscordConnected OnConnected;

    /**
     * Fired whenever a DISPATCH event arrives from Discord.
     * EventName is the "t" field of the payload (e.g. "MESSAGE_CREATE").
     */
    UPROPERTY(BlueprintAssignable, Category = "Discord|WebSocket")
    FOnDiscordMessage OnDiscordEvent;

    /** Fired when a connection or protocol error occurs. */
    UPROPERTY(BlueprintAssignable, Category = "Discord|WebSocket")
    FOnDiscordError OnError;

    /** Fired when the WebSocket connection closes. */
    UPROPERTY(BlueprintAssignable, Category = "Discord|WebSocket")
    FOnDiscordClosed OnClosed;

protected:
    // --- Called on game thread via poll timer ---
    void PollInboundMessages();

    // --- Discord Gateway protocol ---
    void HandleGatewayMessage(const FString& JsonText);
    void HandleHello(const TSharedPtr<FJsonObject>& Data);
    void HandleDispatch(const TSharedPtr<FJsonObject>& Payload, const FString& EventType, int32 Seq);
    void HandleReconnect();
    void HandleInvalidSession(bool bResumable);
    void SendHeartbeat();
    void SendIdentify();
    void SendResume();
    void ScheduleHeartbeat(float IntervalSeconds);

private:
    /** Low-level WebSocket client (manages its own I/O thread) */
    TUniquePtr<FDiscordWebSocketClient> WSClient;

    /** Discord bot token */
    FString BotToken;

    /** Discord Gateway intents bitmask */
    int32 Intents = 3276799;

    /** Discord Gateway URL */
    FString GatewayURL;

    /** Current logical connection state */
    EDiscordWebSocketState ConnectionState = EDiscordWebSocketState::Disconnected;

    /** Last received sequence number */
    int32 LastSequenceNumber = -1;

    /** Session ID received in READY (needed for RESUME) */
    FString SessionId;

    /** Resume URL received in READY */
    FString ResumeGatewayURL;

    /** Timer that fires periodically to drain inbound message queue */
    FTimerHandle PollTimerHandle;

    /** Timer that fires to send a heartbeat */
    FTimerHandle HeartbeatTimerHandle;

    /** Heartbeat interval in seconds */
    float HeartbeatInterval = 45.0f;

    /** True after we received the HeartbeatAck for the last heartbeat */
    bool bHeartbeatAcknowledged = true;
};

