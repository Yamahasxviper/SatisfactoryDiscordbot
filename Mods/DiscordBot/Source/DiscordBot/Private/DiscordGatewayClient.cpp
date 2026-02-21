#include "DiscordGatewayClient.h"

#include "WebSocketsModule.h"
#include "IWebSocket.h"
#include "Dom/JsonObject.h"
#include "Dom/JsonValue.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"
#include "Serialization/JsonWriter.h"
#include "TimerManager.h"
#include "Engine/World.h"

DEFINE_LOG_CATEGORY_STATIC(LogDiscordBot, Log, All);

static const FString DISCORD_GATEWAY_URL = TEXT("wss://gateway.discord.gg/?v=10&encoding=json");

// ---- Helpers ---------------------------------------------------------------

static FString SerializeJson(const TSharedRef<FJsonObject>& Obj)
{
    FString Out;
    TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&Out);
    FJsonSerializer::Serialize(Obj, Writer);
    return Out;
}

static TSharedPtr<FJsonObject> ParseJson(const FString& Raw)
{
    TSharedPtr<FJsonObject> Obj;
    TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Raw);
    FJsonSerializer::Deserialize(Reader, Obj);
    return Obj;
}

// ---- UDiscordGatewayClient -------------------------------------------------

UDiscordGatewayClient::UDiscordGatewayClient()
{
}

void UDiscordGatewayClient::Connect(const FString& InBotToken, int32 InIntents)
{
    if (WebSocket.IsValid() && WebSocket->IsConnected())
    {
        UE_LOG(LogDiscordBot, Warning, TEXT("Already connected. Call Disconnect() first."));
        return;
    }

    BotToken  = InBotToken;
    Intents   = InIntents;
    bIdentified = false;
    LastSequenceNumber.Reset();

    if (!FModuleManager::Get().IsModuleLoaded(TEXT("WebSockets")))
    {
        FModuleManager::Get().LoadModule(TEXT("WebSockets"));
    }

    WebSocket = FWebSocketsModule::Get().CreateWebSocket(DISCORD_GATEWAY_URL, TEXT(""));

    WebSocket->OnConnected().AddUObject(this, &UDiscordGatewayClient::OnWebSocketConnected);
    WebSocket->OnConnectionError().AddUObject(this, &UDiscordGatewayClient::OnWebSocketConnectionError);
    WebSocket->OnClosed().AddUObject(this, &UDiscordGatewayClient::OnWebSocketClosed);
    WebSocket->OnMessage().AddUObject(this, &UDiscordGatewayClient::OnWebSocketMessage);

    UE_LOG(LogDiscordBot, Log, TEXT("Connecting to Discord Gateway: %s"), *DISCORD_GATEWAY_URL);
    WebSocket->Connect();
}

void UDiscordGatewayClient::Disconnect()
{
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().ClearTimer(HeartbeatTimerHandle);
    }

    if (WebSocket.IsValid())
    {
        WebSocket->Close();
        WebSocket.Reset();
    }

    bIdentified = false;
    UE_LOG(LogDiscordBot, Log, TEXT("Disconnected from Discord Gateway."));
}

// ---- WebSocket callbacks ---------------------------------------------------

void UDiscordGatewayClient::OnWebSocketConnected()
{
    UE_LOG(LogDiscordBot, Log, TEXT("WebSocket connected to Discord Gateway."));
    // The Hello payload with heartbeat_interval will arrive next; handled in OnWebSocketMessage.
}

void UDiscordGatewayClient::OnWebSocketConnectionError(const FString& Error)
{
    UE_LOG(LogDiscordBot, Error, TEXT("WebSocket connection error: %s"), *Error);
    OnConnected.Broadcast(false, Error);
}

void UDiscordGatewayClient::OnWebSocketClosed(int32 StatusCode, const FString& Reason, bool bWasClean)
{
    UE_LOG(LogDiscordBot, Log, TEXT("WebSocket closed — code %d, reason: %s, clean: %d"),
        StatusCode, *Reason, bWasClean ? 1 : 0);

    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().ClearTimer(HeartbeatTimerHandle);
    }
    bIdentified = false;
}

void UDiscordGatewayClient::OnWebSocketMessage(const FString& Message)
{
    TSharedPtr<FJsonObject> Payload = ParseJson(Message);
    if (!Payload.IsValid())
    {
        UE_LOG(LogDiscordBot, Warning, TEXT("Received non-JSON message from gateway."));
        return;
    }

    const int32 Op = Payload->GetIntegerField(TEXT("op"));

    // Update sequence number if present (the "s" field is null for non-Dispatch ops)
    double SeqNum = 0.0;
    if (Payload->TryGetNumberField(TEXT("s"), SeqNum))
    {
        LastSequenceNumber = static_cast<int32>(SeqNum);
    }

    switch (Op)
    {
        case EDiscordOpCode::Hello:
        {
            const TSharedPtr<FJsonObject> HelloData = Payload->GetObjectField(TEXT("d"));
            const double HeartbeatIntervalMs = HelloData->GetNumberField(TEXT("heartbeat_interval"));
            const float  HeartbeatIntervalSec = static_cast<float>(HeartbeatIntervalMs) / 1000.f;

            UE_LOG(LogDiscordBot, Log, TEXT("Received Hello — heartbeat interval %.2f s"), HeartbeatIntervalSec);

            if (UWorld* World = GetWorld())
            {
                World->GetTimerManager().SetTimer(
                    HeartbeatTimerHandle,
                    this,
                    &UDiscordGatewayClient::SendHeartbeat,
                    HeartbeatIntervalSec,
                    /*bLoop=*/true,
                    /*FirstDelay=*/HeartbeatIntervalSec
                );
            }

            SendIdentify();
            break;
        }

        case EDiscordOpCode::HeartbeatAck:
            UE_LOG(LogDiscordBot, Verbose, TEXT("Heartbeat ACK received."));
            break;

        case EDiscordOpCode::Heartbeat:
            // Discord requested an immediate heartbeat
            SendHeartbeat();
            break;

        case EDiscordOpCode::Dispatch:
        {
            const FString EventName = Payload->GetStringField(TEXT("t"));
            const TSharedPtr<FJsonObject> EventData = Payload->GetObjectField(TEXT("d"));
            HandleDispatch(EventName, EventData);
            break;
        }

        default:
            UE_LOG(LogDiscordBot, Verbose, TEXT("Unhandled gateway op-code %d"), Op);
            break;
    }
}

// ---- Gateway logic ---------------------------------------------------------

void UDiscordGatewayClient::SendHeartbeat()
{
    if (!WebSocket.IsValid() || !WebSocket->IsConnected()) return;

    TSharedRef<FJsonObject> Payload = MakeShared<FJsonObject>();
    Payload->SetNumberField(TEXT("op"), EDiscordOpCode::Heartbeat);

    if (LastSequenceNumber.IsSet())
    {
        Payload->SetNumberField(TEXT("d"), LastSequenceNumber.GetValue());
    }
    else
    {
        Payload->SetField(TEXT("d"), MakeShared<FJsonValueNull>());
    }

    WebSocket->Send(SerializeJson(Payload));
    UE_LOG(LogDiscordBot, Verbose, TEXT("Heartbeat sent."));
}

void UDiscordGatewayClient::SendIdentify()
{
    if (!WebSocket.IsValid() || !WebSocket->IsConnected()) return;

    // Build the IDENTIFY payload
    // Intents value enables the three requested privileged intents:
    //   GuildMembers  (Server Members Intent) : 1 << 1  =     2
    //   GuildPresences (Presence Intent)      : 1 << 8  =   256
    //   MessageContent (Message Content Intent): 1 << 15 = 32768
    //   Combined                                         = 33026

    TSharedRef<FJsonObject> Properties = MakeShared<FJsonObject>();
    Properties->SetStringField(TEXT("os"),      TEXT("windows"));
    Properties->SetStringField(TEXT("browser"), TEXT("satisfactory-discord-bot"));
    Properties->SetStringField(TEXT("device"),  TEXT("satisfactory-discord-bot"));

    TSharedRef<FJsonObject> IdentifyData = MakeShared<FJsonObject>();
    IdentifyData->SetStringField(TEXT("token"),   BotToken);
    IdentifyData->SetNumberField(TEXT("intents"), Intents);
    IdentifyData->SetObjectField(TEXT("properties"), Properties);

    TSharedRef<FJsonObject> Payload = MakeShared<FJsonObject>();
    Payload->SetNumberField(TEXT("op"), EDiscordOpCode::Identify);
    Payload->SetObjectField(TEXT("d"),  IdentifyData);

    WebSocket->Send(SerializeJson(Payload));
    bIdentified = true;

    UE_LOG(LogDiscordBot, Log,
        TEXT("IDENTIFY sent — intents: %d (GuildMembers=%d | GuildPresences=%d | MessageContent=%d)"),
        Intents,
        static_cast<int32>(EDiscordGatewayIntent::GuildMembers),
        static_cast<int32>(EDiscordGatewayIntent::GuildPresences),
        static_cast<int32>(EDiscordGatewayIntent::MessageContent));

    OnConnected.Broadcast(true, TEXT(""));
}

void UDiscordGatewayClient::HandleDispatch(const FString& EventName,
                                            const TSharedPtr<FJsonObject>& Data)
{
    if (EventName == TEXT("READY"))
    {
        UE_LOG(LogDiscordBot, Log, TEXT("Bot is READY."));
    }
    else if (EventName == TEXT("MESSAGE_CREATE"))
    {
        // MESSAGE_CREATE is delivered only when MESSAGE_CONTENT intent is active
        const FString Content = Data.IsValid() ? Data->GetStringField(TEXT("content")) : TEXT("");
        UE_LOG(LogDiscordBot, Log, TEXT("MESSAGE_CREATE: %s"), *Content);
        OnMessageReceived.Broadcast(Content);
    }
    else if (EventName == TEXT("PRESENCE_UPDATE"))
    {
        // PRESENCE_UPDATE requires GUILD_PRESENCES intent
        const TSharedPtr<FJsonObject> User = Data.IsValid() ? Data->GetObjectField(TEXT("user")) : nullptr;
        const FString UserId = (User.IsValid()) ? User->GetStringField(TEXT("id")) : TEXT("");
        UE_LOG(LogDiscordBot, Log, TEXT("PRESENCE_UPDATE for user: %s"), *UserId);
        OnPresenceUpdated.Broadcast(UserId);
    }
    else if (EventName == TEXT("GUILD_MEMBER_UPDATE") ||
             EventName == TEXT("GUILD_MEMBER_ADD")    ||
             EventName == TEXT("GUILD_MEMBER_REMOVE"))
    {
        // These events require GUILD_MEMBERS intent
        const TSharedPtr<FJsonObject> User = Data.IsValid() ? Data->GetObjectField(TEXT("user")) : nullptr;
        const FString UserId = (User.IsValid()) ? User->GetStringField(TEXT("id")) : TEXT("");
        UE_LOG(LogDiscordBot, Log, TEXT("%s for user: %s"), *EventName, *UserId);
        OnMemberUpdated.Broadcast(UserId);
    }
    else
    {
        UE_LOG(LogDiscordBot, Verbose, TEXT("Unhandled dispatch event: %s"), *EventName);
    }
}
