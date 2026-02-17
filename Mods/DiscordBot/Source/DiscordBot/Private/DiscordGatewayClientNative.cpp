// Copyright (c) 2024 Yamahasxviper

#include "DiscordGatewayClientNative.h"
#include "WebSocketsModule.h"
#include "HttpModule.h"
#include "Interfaces/IHttpRequest.h"
#include "Interfaces/IHttpResponse.h"
#include "Json.h"
#include "JsonUtilities.h"
#include "TimerManager.h"
#include "Engine/World.h"

DEFINE_LOG_CATEGORY_STATIC(LogDiscordGatewayNative, Log, All);

// Discord Gateway Opcodes
#define OPCODE_DISPATCH 0
#define OPCODE_HEARTBEAT 1
#define OPCODE_IDENTIFY 2
#define OPCODE_HELLO 10
#define OPCODE_HEARTBEAT_ACK 11

// Discord Intents
// Presence Intent (1 << 8) = 256
// Server Members Intent (1 << 1) = 2
// Message Content Intent (1 << 15) = 32768
// Combined: 256 + 2 + 32768 = 33026
#define DISCORD_INTENTS_COMBINED 33026

ADiscordGatewayClientNative::ADiscordGatewayClientNative()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.bStartWithTickEnabled = true;

    // Initialize default values
    GatewayURL = TEXT("wss://gateway.discord.gg");
    Intents = DISCORD_INTENTS_COMBINED;
    bIsConnected = false;
    HeartbeatInterval = 41250; // Default Discord heartbeat interval
    SequenceNumber = -1;
}

void ADiscordGatewayClientNative::BeginPlay()
{
    Super::BeginPlay();
    
    UE_LOG(LogDiscordGatewayNative, Log, TEXT("Discord Gateway Client (Native WebSocket) initialized"));
    
    // Get HTTP module
    HttpModule = &FHttpModule::Get();
}

void ADiscordGatewayClientNative::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    Disconnect();
    Super::EndPlay(EndPlayReason);
}

void ADiscordGatewayClientNative::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
}

void ADiscordGatewayClientNative::InitializeBot(const FString& InBotToken)
{
    BotToken = InBotToken;
    UE_LOG(LogDiscordGatewayNative, Log, TEXT("Bot token set"));
}

void ADiscordGatewayClientNative::Connect()
{
    if (BotToken.IsEmpty())
    {
        UE_LOG(LogDiscordGatewayNative, Error, TEXT("Cannot connect: Bot token is empty"));
        return;
    }

    UE_LOG(LogDiscordGatewayNative, Log, TEXT("Connecting to Discord Gateway..."));
    GetGatewayURL();
}

void ADiscordGatewayClientNative::Disconnect()
{
    if (bIsConnected && WebSocket.IsValid())
    {
        // Clear heartbeat timer
        if (GetWorld())
        {
            GetWorld()->GetTimerManager().ClearTimer(HeartbeatTimerHandle);
        }

        // Close WebSocket
        WebSocket->Close(1000, TEXT("Client disconnect"));
        WebSocket.Reset();

        bIsConnected = false;
        SessionId.Empty();
        SequenceNumber = -1;
        
        UE_LOG(LogDiscordGatewayNative, Log, TEXT("Disconnected from Discord Gateway"));
    }
}

void ADiscordGatewayClientNative::GetGatewayURL()
{
    TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request = HttpModule->CreateRequest();
    Request->SetURL(TEXT("https://discord.com/api/v10/gateway/bot"));
    Request->SetVerb(TEXT("GET"));
    Request->SetHeader(TEXT("Authorization"), FString::Printf(TEXT("Bot %s"), *BotToken));
    Request->SetHeader(TEXT("Content-Type"), TEXT("application/json"));
    
    Request->OnProcessRequestComplete().BindUObject(this, &ADiscordGatewayClientNative::OnGetGatewayURLComplete);
    
    if (!Request->ProcessRequest())
    {
        UE_LOG(LogDiscordGatewayNative, Error, TEXT("Failed to send gateway URL request"));
    }
}

void ADiscordGatewayClientNative::OnGetGatewayURLComplete(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
{
    if (!bWasSuccessful || !Response.IsValid())
    {
        UE_LOG(LogDiscordGatewayNative, Error, TEXT("Failed to get gateway URL"));
        return;
    }

    const FString ResponseStr = Response->GetContentAsString();
    UE_LOG(LogDiscordGatewayNative, Log, TEXT("Gateway response: %s"), *ResponseStr);

    TSharedPtr<FJsonObject> JsonObject;
    TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(ResponseStr);
    
    if (FJsonSerializer::Deserialize(Reader, JsonObject) && JsonObject.IsValid())
    {
        if (JsonObject->HasField(TEXT("url")))
        {
            GatewayURL = JsonObject->GetStringField(TEXT("url"));
            // Append required query parameters for Discord Gateway v10
            GatewayURL += TEXT("/?v=10&encoding=json");
            UE_LOG(LogDiscordGatewayNative, Log, TEXT("Gateway URL: %s"), *GatewayURL);
            
            // Connect to WebSocket with the retrieved URL
            ConnectWebSocket();
        }
    }
}

void ADiscordGatewayClientNative::ConnectWebSocket()
{
    if (!FModuleManager::Get().IsModuleLoaded("WebSockets"))
    {
        FModuleManager::Get().LoadModule("WebSockets");
    }

    // Create native WebSocket using Unreal's built-in module
    WebSocket = FWebSocketsModule::Get().CreateWebSocket(GatewayURL);
    
    if (!WebSocket.IsValid())
    {
        UE_LOG(LogDiscordGatewayNative, Error, TEXT("Failed to create WebSocket"));
        return;
    }

    // Bind WebSocket events
    WebSocket->OnConnected().AddUObject(this, &ADiscordGatewayClientNative::OnWebSocketConnected);
    WebSocket->OnConnectionError().AddUObject(this, &ADiscordGatewayClientNative::OnWebSocketConnectionError);
    WebSocket->OnClosed().AddUObject(this, &ADiscordGatewayClientNative::OnWebSocketClosed);
    WebSocket->OnMessage().AddUObject(this, &ADiscordGatewayClientNative::OnWebSocketMessage);

    // Connect
    UE_LOG(LogDiscordGatewayNative, Log, TEXT("Connecting to WebSocket: %s"), *GatewayURL);
    WebSocket->Connect();
}

void ADiscordGatewayClientNative::OnWebSocketConnected()
{
    UE_LOG(LogDiscordGatewayNative, Log, TEXT("WebSocket connected successfully"));
    bIsConnected = true;
}

void ADiscordGatewayClientNative::OnWebSocketConnectionError(const FString& Error)
{
    UE_LOG(LogDiscordGatewayNative, Error, TEXT("WebSocket connection error: %s"), *Error);
    bIsConnected = false;
}

void ADiscordGatewayClientNative::OnWebSocketClosed(int32 StatusCode, const FString& Reason, bool bWasClean)
{
    UE_LOG(LogDiscordGatewayNative, Warning, TEXT("WebSocket closed: Code=%d, Reason=%s, Clean=%s"), 
        StatusCode, *Reason, bWasClean ? TEXT("Yes") : TEXT("No"));
    
    bIsConnected = false;
    
    // Clear heartbeat timer
    if (GetWorld())
    {
        GetWorld()->GetTimerManager().ClearTimer(HeartbeatTimerHandle);
    }
}

void ADiscordGatewayClientNative::OnWebSocketMessage(const FString& Message)
{
    UE_LOG(LogDiscordGatewayNative, Verbose, TEXT("Received message: %s"), *Message);
    
    TSharedPtr<FJsonObject> JsonObject;
    TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Message);
    
    if (FJsonSerializer::Deserialize(Reader, JsonObject) && JsonObject.IsValid())
    {
        int32 OpCode = JsonObject->GetIntegerField(TEXT("op"));
        
        // Update sequence number if present
        if (JsonObject->HasField(TEXT("s")) && !JsonObject->GetField<EJson::None>(TEXT("s"))->IsNull())
        {
            SequenceNumber = JsonObject->GetIntegerField(TEXT("s"));
        }
        
        // Get event data
        TSharedPtr<FJsonObject> Data;
        if (JsonObject->HasField(TEXT("d")))
        {
            const TSharedPtr<FJsonValue> DataValue = JsonObject->GetField<EJson::Object>(TEXT("d"));
            if (DataValue.IsValid() && DataValue->Type == EJson::Object)
            {
                Data = DataValue->AsObject();
            }
        }
        
        HandleGatewayEvent(OpCode, Data);
    }
}

void ADiscordGatewayClientNative::HandleGatewayEvent(int32 OpCode, const TSharedPtr<FJsonObject>& Data)
{
    switch (OpCode)
    {
    case OPCODE_HELLO:
        if (Data.IsValid() && Data->HasField(TEXT("heartbeat_interval")))
        {
            HeartbeatInterval = Data->GetIntegerField(TEXT("heartbeat_interval"));
            UE_LOG(LogDiscordGatewayNative, Log, TEXT("Received HELLO: Heartbeat interval=%d ms"), HeartbeatInterval);
            
            // Start heartbeat timer
            if (GetWorld())
            {
                GetWorld()->GetTimerManager().SetTimer(
                    HeartbeatTimerHandle,
                    this,
                    &ADiscordGatewayClientNative::SendHeartbeat,
                    HeartbeatInterval / 1000.0f,
                    true
                );
            }
            
            // Send identify
            SendIdentify();
        }
        break;
        
    case OPCODE_HEARTBEAT_ACK:
        UE_LOG(LogDiscordGatewayNative, Verbose, TEXT("Heartbeat acknowledged"));
        break;
        
    case OPCODE_DISPATCH:
        // Handle various Discord events here
        if (Data.IsValid())
        {
            UE_LOG(LogDiscordGatewayNative, Log, TEXT("Dispatch event received (sequence: %d)"), SequenceNumber);
            
            // Get event type
            FString EventType;
            if (JsonObject->HasField(TEXT("t")))
            {
                EventType = JsonObject->GetStringField(TEXT("t"));
                UE_LOG(LogDiscordGatewayNative, Log, TEXT("Event type: %s"), *EventType);
                
                // Handle READY event to get session ID
                if (EventType == TEXT("READY") && Data->HasField(TEXT("session_id")))
                {
                    SessionId = Data->GetStringField(TEXT("session_id"));
                    UE_LOG(LogDiscordGatewayNative, Log, TEXT("Bot ready! Session ID: %s"), *SessionId);
                }
            }
        }
        break;
        
    default:
        UE_LOG(LogDiscordGatewayNative, Verbose, TEXT("Unknown opcode: %d"), OpCode);
        break;
    }
}

void ADiscordGatewayClientNative::SendIdentify()
{
    TSharedPtr<FJsonObject> IdentifyPayload = MakeShareable(new FJsonObject());
    IdentifyPayload->SetNumberField(TEXT("op"), OPCODE_IDENTIFY);
    
    TSharedPtr<FJsonObject> Data = MakeShareable(new FJsonObject());
    Data->SetStringField(TEXT("token"), BotToken);
    Data->SetNumberField(TEXT("intents"), Intents);
    
    TSharedPtr<FJsonObject> Properties = MakeShareable(new FJsonObject());
    Properties->SetStringField(TEXT("os"), TEXT("windows"));
    Properties->SetStringField(TEXT("browser"), TEXT("satisfactory-mod"));
    Properties->SetStringField(TEXT("device"), TEXT("satisfactory-mod"));
    Data->SetObjectField(TEXT("properties"), Properties);
    
    IdentifyPayload->SetObjectField(TEXT("d"), Data);
    
    FString OutputString;
    TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&OutputString);
    FJsonSerializer::Serialize(IdentifyPayload.ToSharedRef(), Writer);
    
    UE_LOG(LogDiscordGatewayNative, Log, TEXT("Sending IDENTIFY"));
    
    if (WebSocket.IsValid() && bIsConnected)
    {
        WebSocket->Send(OutputString);
    }
}

void ADiscordGatewayClientNative::SendHeartbeat()
{
    TSharedPtr<FJsonObject> HeartbeatPayload = MakeShareable(new FJsonObject());
    HeartbeatPayload->SetNumberField(TEXT("op"), OPCODE_HEARTBEAT);
    
    if (SequenceNumber >= 0)
    {
        HeartbeatPayload->SetNumberField(TEXT("d"), SequenceNumber);
    }
    else
    {
        HeartbeatPayload->SetField(TEXT("d"), MakeShareable(new FJsonValueNull()));
    }
    
    FString OutputString;
    TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&OutputString);
    FJsonSerializer::Serialize(HeartbeatPayload.ToSharedRef(), Writer);
    
    UE_LOG(LogDiscordGatewayNative, Verbose, TEXT("Sending HEARTBEAT (seq: %d)"), SequenceNumber);
    
    if (WebSocket.IsValid() && bIsConnected)
    {
        WebSocket->Send(OutputString);
    }
}

void ADiscordGatewayClientNative::SendMessage(const FString& ChannelId, const FString& Message)
{
    if (!bIsConnected)
    {
        UE_LOG(LogDiscordGatewayNative, Warning, TEXT("Cannot send message: Not connected"));
        return;
    }

    SendMessageHTTP(ChannelId, Message);
}

void ADiscordGatewayClientNative::SendMessageHTTP(const FString& ChannelId, const FString& MessageContent)
{
    TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request = HttpModule->CreateRequest();
    Request->SetURL(FString::Printf(TEXT("https://discord.com/api/v10/channels/%s/messages"), *ChannelId));
    Request->SetVerb(TEXT("POST"));
    Request->SetHeader(TEXT("Authorization"), FString::Printf(TEXT("Bot %s"), *BotToken));
    Request->SetHeader(TEXT("Content-Type"), TEXT("application/json"));
    
    TSharedPtr<FJsonObject> JsonObject = MakeShareable(new FJsonObject());
    JsonObject->SetStringField(TEXT("content"), MessageContent);
    
    FString OutputString;
    TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&OutputString);
    FJsonSerializer::Serialize(JsonObject.ToSharedRef(), Writer);
    
    Request->SetContentAsString(OutputString);
    
    Request->OnProcessRequestComplete().BindLambda([](FHttpRequestPtr Req, FHttpResponsePtr Res, bool bSuccess)
    {
        if (bSuccess && Res.IsValid())
        {
            UE_LOG(LogDiscordGatewayNative, Log, TEXT("Message sent successfully"));
        }
        else
        {
            UE_LOG(LogDiscordGatewayNative, Error, TEXT("Failed to send message"));
        }
    });
    
    if (!Request->ProcessRequest())
    {
        UE_LOG(LogDiscordGatewayNative, Error, TEXT("Failed to send message request"));
    }
}
