// Copyright Yamahasxviper. All Rights Reserved.

#include "DiscordBotWebSocket.h"

#include "Dom/JsonObject.h"
#include "Dom/JsonValue.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonWriter.h"
#include "Serialization/JsonSerializer.h"
#include "Engine/World.h"
#include "TimerManager.h"

DEFINE_LOG_CATEGORY(LogDiscordBotWebSocket);

// Discord Gateway v10 URL
static const FString DiscordGatewayURL = TEXT("wss://gateway.discord.gg/?v=10&encoding=json");

// How often (seconds) we poll the inbound message queue from the game thread
static constexpr float PollInterval = 0.05f;   // 50 ms

// ---------------------------------------------------------------------------
// Construction / Destruction
// ---------------------------------------------------------------------------

UDiscordBotWebSocket::UDiscordBotWebSocket()
    : GatewayURL(DiscordGatewayURL)
{
}

UDiscordBotWebSocket::~UDiscordBotWebSocket()
{
    Disconnect();
}

// ---------------------------------------------------------------------------
// Blueprint factory
// ---------------------------------------------------------------------------

UDiscordBotWebSocket* UDiscordBotWebSocket::Create(UObject* WorldContextObject,
    const FString& BotToken, int32 Intents)
{
    UDiscordBotWebSocket* Instance = NewObject<UDiscordBotWebSocket>(WorldContextObject);
    Instance->BotToken = BotToken;
    Instance->Intents  = Intents;
    return Instance;
}

// ---------------------------------------------------------------------------
// Public API
// ---------------------------------------------------------------------------

void UDiscordBotWebSocket::Connect()
{
    if (ConnectionState != EDiscordWebSocketState::Disconnected)
    {
        UE_LOG(LogDiscordBotWebSocket, Warning,
            TEXT("Connect() called while not Disconnected (state=%d)"), (int32)ConnectionState);
        return;
    }

    const FString& URL = (ConnectionState == EDiscordWebSocketState::Resuming
                          && !ResumeGatewayURL.IsEmpty())
        ? ResumeGatewayURL
        : GatewayURL;

    WSClient = MakeUnique<FDiscordWebSocketClient>(URL);
    ConnectionState = EDiscordWebSocketState::Connecting;

    if (!WSClient->Connect())
    {
        UE_LOG(LogDiscordBotWebSocket, Error, TEXT("Failed to start WebSocket I/O thread"));
        WSClient.Reset();
        ConnectionState = EDiscordWebSocketState::Disconnected;
        OnError.Broadcast(TEXT("Failed to start WebSocket I/O thread"));
        return;
    }

    // Start polling the inbound queue on the game thread
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().SetTimer(
            PollTimerHandle,
            FTimerDelegate::CreateUObject(this, &UDiscordBotWebSocket::PollInboundMessages),
            PollInterval,
            true  /* loop */
        );
    }

    UE_LOG(LogDiscordBotWebSocket, Log, TEXT("Connecting to Discord Gateway: %s"), *URL);
}

void UDiscordBotWebSocket::Disconnect()
{
    // Cancel timers
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().ClearTimer(PollTimerHandle);
        World->GetTimerManager().ClearTimer(HeartbeatTimerHandle);
    }

    if (WSClient)
    {
        WSClient->Disconnect();  // waits for the thread to exit
        WSClient.Reset();
    }

    ConnectionState = EDiscordWebSocketState::Disconnected;
}

void UDiscordBotWebSocket::SendRawPayload(const FString& JsonPayload)
{
    if (!WSClient)
    {
        UE_LOG(LogDiscordBotWebSocket, Warning,
            TEXT("SendRawPayload: not connected"));
        return;
    }
    WSClient->SendText(JsonPayload);
}

void UDiscordBotWebSocket::SendPresenceUpdate(const FString& Status,
    const FString& ActivityName, int32 ActivityType)
{
    TSharedPtr<FJsonObject> Activity = MakeShared<FJsonObject>();
    Activity->SetStringField(TEXT("name"), ActivityName);
    Activity->SetNumberField(TEXT("type"), ActivityType);

    TArray<TSharedPtr<FJsonValue>> Activities;
    Activities.Add(MakeShared<FJsonValueObject>(Activity));

    TSharedPtr<FJsonObject> Data = MakeShared<FJsonObject>();
    Data->SetNumberField(TEXT("since"), 0);
    Data->SetArrayField(TEXT("activities"), Activities);
    Data->SetStringField(TEXT("status"), Status);
    Data->SetBoolField(TEXT("afk"), false);

    TSharedPtr<FJsonObject> Payload = MakeShared<FJsonObject>();
    Payload->SetNumberField(TEXT("op"), (int32)EDiscordGatewayOpcode::PresenceUpdate);
    Payload->SetObjectField(TEXT("d"), Data);

    FString PayloadString;
    TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&PayloadString);
    FJsonSerializer::Serialize(Payload.ToSharedRef(), Writer);

    SendRawPayload(PayloadString);
}

// ---------------------------------------------------------------------------
// Game-thread poll
// ---------------------------------------------------------------------------

void UDiscordBotWebSocket::PollInboundMessages()
{
    if (!WSClient)
        return;

    FWSInboundMessage Msg;
    while (WSClient->InboundMessages.Dequeue(Msg))
    {
        switch (Msg.Kind)
        {
        case EWSMessageKind::Connected:
            UE_LOG(LogDiscordBotWebSocket, Log, TEXT("WebSocket TCP/TLS connected; waiting for HELLO"));
            break;

        case EWSMessageKind::TextReceived:
            HandleGatewayMessage(Msg.Payload);
            break;

        case EWSMessageKind::Error:
            UE_LOG(LogDiscordBotWebSocket, Error,
                TEXT("WebSocket error: %s"), *Msg.Payload);
            ConnectionState = EDiscordWebSocketState::Disconnected;
            if (UWorld* World = GetWorld())
            {
                World->GetTimerManager().ClearTimer(HeartbeatTimerHandle);
                World->GetTimerManager().ClearTimer(PollTimerHandle);
            }
            WSClient.Reset();
            OnError.Broadcast(Msg.Payload);
            break;

        case EWSMessageKind::Closed:
            UE_LOG(LogDiscordBotWebSocket, Log,
                TEXT("WebSocket closed: code=%d reason=%s"), Msg.CloseCode, *Msg.Payload);
            ConnectionState = EDiscordWebSocketState::Disconnected;
            if (UWorld* World = GetWorld())
            {
                World->GetTimerManager().ClearTimer(HeartbeatTimerHandle);
                World->GetTimerManager().ClearTimer(PollTimerHandle);
            }
            WSClient.Reset();
            OnClosed.Broadcast(Msg.CloseCode, Msg.Payload);
            break;

        default:
            break;
        }
    }
}

// ---------------------------------------------------------------------------
// Discord Gateway protocol
// ---------------------------------------------------------------------------

void UDiscordBotWebSocket::HandleGatewayMessage(const FString& JsonText)
{
    TSharedPtr<FJsonObject> JsonPayload;
    TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(JsonText);
    if (!FJsonSerializer::Deserialize(Reader, JsonPayload) || !JsonPayload.IsValid())
    {
        UE_LOG(LogDiscordBotWebSocket, Warning,
            TEXT("Failed to parse Gateway message: %s"), *JsonText);
        return;
    }

    const int32 Opcode = (int32)JsonPayload->GetNumberField(TEXT("op"));

    switch ((EDiscordGatewayOpcode)Opcode)
    {
    case EDiscordGatewayOpcode::Hello:
    {
        const TSharedPtr<FJsonObject>* Data = nullptr;
        if (JsonPayload->TryGetObjectField(TEXT("d"), Data) && Data)
            HandleHello(*Data);
        break;
    }
    case EDiscordGatewayOpcode::Dispatch:
    {
        FString EventType;
        JsonPayload->TryGetStringField(TEXT("t"), EventType);
        int32 Seq = 0;
        JsonPayload->TryGetNumberField(TEXT("s"), Seq);
        HandleDispatch(JsonPayload, EventType, Seq);
        break;
    }
    case EDiscordGatewayOpcode::HeartbeatAck:
        bHeartbeatAcknowledged = true;
        UE_LOG(LogDiscordBotWebSocket, Verbose, TEXT("Heartbeat ACK received"));
        break;

    case EDiscordGatewayOpcode::Heartbeat:
        // Server requested an immediate heartbeat
        SendHeartbeat();
        break;

    case EDiscordGatewayOpcode::Reconnect:
        HandleReconnect();
        break;

    case EDiscordGatewayOpcode::InvalidSession:
    {
        bool bResumable = false;
        const TSharedPtr<FJsonValue>* DataVal = JsonPayload->Values.Find(TEXT("d"));
        if (DataVal && (*DataVal)->Type == EJson::Boolean)
            bResumable = (*DataVal)->AsBool();
        HandleInvalidSession(bResumable);
        break;
    }
    default:
        UE_LOG(LogDiscordBotWebSocket, Verbose,
            TEXT("Unhandled Gateway opcode %d"), Opcode);
        break;
    }
}

void UDiscordBotWebSocket::HandleHello(const TSharedPtr<FJsonObject>& Data)
{
    double Interval = 0.0;
    if (Data->TryGetNumberField(TEXT("heartbeat_interval"), Interval))
        HeartbeatInterval = (float)(Interval / 1000.0);

    UE_LOG(LogDiscordBotWebSocket, Log,
        TEXT("HELLO received; heartbeat_interval=%.2fs"), HeartbeatInterval);

    // Jittered first heartbeat as recommended by Discord
    const float JitteredDelay = HeartbeatInterval * FMath::FRandRange(0.0f, 1.0f);
    ScheduleHeartbeat(JitteredDelay);

    // IDENTIFY or RESUME
    if (!SessionId.IsEmpty() && ConnectionState == EDiscordWebSocketState::Resuming)
        SendResume();
    else
        SendIdentify();
}

void UDiscordBotWebSocket::HandleDispatch(const TSharedPtr<FJsonObject>& Payload,
    const FString& EventType, int32 Seq)
{
    LastSequenceNumber = Seq;

    if (EventType == TEXT("READY"))
    {
        const TSharedPtr<FJsonObject>* Data = nullptr;
        if (Payload->TryGetObjectField(TEXT("d"), Data) && Data)
        {
            (*Data)->TryGetStringField(TEXT("session_id"), SessionId);
            (*Data)->TryGetStringField(TEXT("resume_gateway_url"), ResumeGatewayURL);
        }
        ConnectionState = EDiscordWebSocketState::Connected;
        UE_LOG(LogDiscordBotWebSocket, Log,
            TEXT("Bot is READY (session_id=%s)"), *SessionId);
        OnConnected.Broadcast();
    }
    else if (EventType == TEXT("RESUMED"))
    {
        ConnectionState = EDiscordWebSocketState::Connected;
        UE_LOG(LogDiscordBotWebSocket, Log, TEXT("Session RESUMED"));
        OnConnected.Broadcast();
    }

    OnDiscordEvent.Broadcast(EventType);
}

void UDiscordBotWebSocket::HandleReconnect()
{
    UE_LOG(LogDiscordBotWebSocket, Log,
        TEXT("Discord requested RECONNECT; will resume session"));
    ConnectionState = EDiscordWebSocketState::Resuming;
    Disconnect();
    Connect();
}

void UDiscordBotWebSocket::HandleInvalidSession(bool bResumable)
{
    UE_LOG(LogDiscordBotWebSocket, Warning,
        TEXT("Invalid session (resumable=%d); reconnecting"), bResumable);

    if (!bResumable)
    {
        SessionId.Empty();
        ResumeGatewayURL.Empty();
        LastSequenceNumber = -1;
        ConnectionState    = EDiscordWebSocketState::Disconnected;
    }
    else
    {
        ConnectionState = EDiscordWebSocketState::Resuming;
    }

    // Discord recommends a small random delay before reconnecting
    if (UWorld* World = GetWorld())
    {
        FTimerHandle RetryHandle;
        const float  Delay = FMath::FRandRange(1.0f, 5.0f);
        World->GetTimerManager().SetTimer(
            RetryHandle,
            FTimerDelegate::CreateUObject(this, &UDiscordBotWebSocket::Connect),
            Delay, false);
    }
}

void UDiscordBotWebSocket::SendHeartbeat()
{
    if (!bHeartbeatAcknowledged)
    {
        // Zombie connection — reconnect
        UE_LOG(LogDiscordBotWebSocket, Warning,
            TEXT("No HeartbeatAck received; reconnecting"));
        ConnectionState = EDiscordWebSocketState::Resuming;
        Disconnect();
        Connect();
        return;
    }

    TSharedPtr<FJsonObject> Payload = MakeShared<FJsonObject>();
    Payload->SetNumberField(TEXT("op"), (int32)EDiscordGatewayOpcode::Heartbeat);
    if (LastSequenceNumber >= 0)
        Payload->SetNumberField(TEXT("d"), LastSequenceNumber);
    else
        Payload->SetField(TEXT("d"), MakeShared<FJsonValueNull>());

    FString PayloadStr;
    TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&PayloadStr);
    FJsonSerializer::Serialize(Payload.ToSharedRef(), Writer);

    bHeartbeatAcknowledged = false;
    SendRawPayload(PayloadStr);
    UE_LOG(LogDiscordBotWebSocket, Verbose,
        TEXT("Sent heartbeat (seq=%d)"), LastSequenceNumber);

    ScheduleHeartbeat(HeartbeatInterval);
}

void UDiscordBotWebSocket::SendIdentify()
{
    TSharedPtr<FJsonObject> Props = MakeShared<FJsonObject>();
    Props->SetStringField(TEXT("os"),      TEXT("windows"));
    Props->SetStringField(TEXT("browser"), TEXT("SatisfactoryDiscordBot"));
    Props->SetStringField(TEXT("device"),  TEXT("SatisfactoryDiscordBot"));

    TSharedPtr<FJsonObject> Data = MakeShared<FJsonObject>();
    Data->SetStringField(TEXT("token"),      FString::Printf(TEXT("Bot %s"), *BotToken));
    Data->SetNumberField(TEXT("intents"),    Intents);
    Data->SetObjectField(TEXT("properties"), Props);

    TSharedPtr<FJsonObject> Payload = MakeShared<FJsonObject>();
    Payload->SetNumberField(TEXT("op"), (int32)EDiscordGatewayOpcode::Identify);
    Payload->SetObjectField(TEXT("d"),  Data);

    FString PayloadStr;
    TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&PayloadStr);
    FJsonSerializer::Serialize(Payload.ToSharedRef(), Writer);

    SendRawPayload(PayloadStr);
    UE_LOG(LogDiscordBotWebSocket, Log, TEXT("Sent IDENTIFY"));
}

void UDiscordBotWebSocket::SendResume()
{
    TSharedPtr<FJsonObject> Data = MakeShared<FJsonObject>();
    Data->SetStringField(TEXT("token"),      FString::Printf(TEXT("Bot %s"), *BotToken));
    Data->SetStringField(TEXT("session_id"), SessionId);
    Data->SetNumberField(TEXT("seq"),        LastSequenceNumber);

    TSharedPtr<FJsonObject> Payload = MakeShared<FJsonObject>();
    Payload->SetNumberField(TEXT("op"), (int32)EDiscordGatewayOpcode::Resume);
    Payload->SetObjectField(TEXT("d"),  Data);

    FString PayloadStr;
    TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&PayloadStr);
    FJsonSerializer::Serialize(Payload.ToSharedRef(), Writer);

    SendRawPayload(PayloadStr);
    UE_LOG(LogDiscordBotWebSocket, Log,
        TEXT("Sent RESUME (session=%s seq=%d)"), *SessionId, LastSequenceNumber);
}

void UDiscordBotWebSocket::ScheduleHeartbeat(float IntervalSeconds)
{
    UWorld* World = GetWorld();
    if (!World)
    {
        UE_LOG(LogDiscordBotWebSocket, Warning,
            TEXT("ScheduleHeartbeat: no World — timer not set"));
        return;
    }
    World->GetTimerManager().SetTimer(
        HeartbeatTimerHandle,
        FTimerDelegate::CreateUObject(this, &UDiscordBotWebSocket::SendHeartbeat),
        IntervalSeconds, false);
}

