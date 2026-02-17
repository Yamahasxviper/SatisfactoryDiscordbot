// Copyright (c) 2024 Yamahasxviper
// Discord Gateway Client using Custom WebSocket

#include "DiscordGatewayClientCustom.h"
#include "HttpModule.h"
#include "Interfaces/IHttpRequest.h"
#include "Interfaces/IHttpResponse.h"
#include "Json.h"
#include "JsonUtilities.h"
#include "TimerManager.h"
#include "Engine/World.h"

DEFINE_LOG_CATEGORY_STATIC(LogDiscordGatewayCustom, Log, All);

// Discord Gateway Opcodes
#define OPCODE_DISPATCH 0
#define OPCODE_HEARTBEAT 1
#define OPCODE_IDENTIFY 2
#define OPCODE_RESUME 6
#define OPCODE_RECONNECT 7
#define OPCODE_INVALID_SESSION 9
#define OPCODE_HELLO 10
#define OPCODE_HEARTBEAT_ACK 11

// Discord Intents
// Presence Intent (1 << 8) = 256
// Server Members Intent (1 << 1) = 2
// Message Content Intent (1 << 15) = 32768
// Combined: 256 + 2 + 32768 = 33026
#define DISCORD_INTENTS_COMBINED 33026

ADiscordGatewayClientCustom::ADiscordGatewayClientCustom()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.bStartWithTickEnabled = true;

    // Initialize default values
    GatewayURL = TEXT("wss://gateway.discord.gg");
    Intents = DISCORD_INTENTS_COMBINED;
    bIsConnected = false;
    HeartbeatInterval = 41250; // Default Discord heartbeat interval
    SequenceNumber = -1;
    bHeartbeatAckReceived = true;
    LastHeartbeatTime = 0.0f;
    ReconnectAttempts = 0;
    ReconnectDelay = 1.0f;
}

void ADiscordGatewayClientCustom::BeginPlay()
{
    Super::BeginPlay();
    
    UE_LOG(LogDiscordGatewayCustom, Log, TEXT("Discord Gateway Client (Custom WebSocket) initialized"));
    UE_LOG(LogDiscordGatewayCustom, Log, TEXT("✅ Using platform-agnostic Custom WebSocket implementation"));
    
    // Get HTTP module
    HttpModule = &FHttpModule::Get();
}

void ADiscordGatewayClientCustom::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    Disconnect();
    Super::EndPlay(EndPlayReason);
}

void ADiscordGatewayClientCustom::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    // Tick the WebSocket for async operations
    if (WebSocket.IsValid())
    {
        WebSocket->Tick(DeltaTime);
    }
}

void ADiscordGatewayClientCustom::InitializeBot(const FString& BotTokenValue)
{
    BotToken = BotTokenValue;
    UE_LOG(LogDiscordGatewayCustom, Log, TEXT("Bot initialized with token"));
}

void ADiscordGatewayClientCustom::Connect()
{
    if (BotToken.IsEmpty())
    {
        UE_LOG(LogDiscordGatewayCustom, Error, TEXT("Cannot connect: Bot token is empty"));
        return;
    }

    // Get gateway URL from Discord API
    GetGatewayURL();
}

void ADiscordGatewayClientCustom::Disconnect()
{
    // Clear heartbeat timer
    if (GetWorld())
    {
        GetWorld()->GetTimerManager().ClearTimer(HeartbeatTimerHandle);
    }

    // Close WebSocket connection
    if (WebSocket.IsValid())
    {
        WebSocket->Disconnect(1000, TEXT("Client disconnecting"));
        WebSocket.Reset();
    }

    bIsConnected = false;
    SessionId.Empty();
    SequenceNumber = -1;
    
    UE_LOG(LogDiscordGatewayCustom, Log, TEXT("Disconnected from Discord Gateway"));
}

void ADiscordGatewayClientCustom::GetGatewayURL()
{
    TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request = HttpModule->CreateRequest();
    Request->SetURL(TEXT("https://discord.com/api/v10/gateway/bot"));
    Request->SetVerb(TEXT("GET"));
    Request->SetHeader(TEXT("Authorization"), FString::Printf(TEXT("Bot %s"), *BotToken));
    Request->SetHeader(TEXT("Content-Type"), TEXT("application/json"));
    
    Request->OnProcessRequestComplete().BindUObject(this, &ADiscordGatewayClientCustom::OnGetGatewayURLComplete);
    
    if (!Request->ProcessRequest())
    {
        UE_LOG(LogDiscordGatewayCustom, Error, TEXT("Failed to send gateway URL request"));
    }
}

void ADiscordGatewayClientCustom::OnGetGatewayURLComplete(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
{
    if (!bWasSuccessful || !Response.IsValid())
    {
        UE_LOG(LogDiscordGatewayCustom, Error, TEXT("Failed to get gateway URL"));
        return;
    }

    const FString ResponseStr = Response->GetContentAsString();
    UE_LOG(LogDiscordGatewayCustom, Log, TEXT("Gateway response: %s"), *ResponseStr);

    TSharedPtr<FJsonObject> JsonObject;
    TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(ResponseStr);
    
    if (FJsonSerializer::Deserialize(Reader, JsonObject) && JsonObject.IsValid())
    {
        if (JsonObject->HasField(TEXT("url")))
        {
            FString BaseURL = JsonObject->GetStringField(TEXT("url"));
            // Append Discord Gateway v10 parameters
            GatewayURL = FString::Printf(TEXT("%s/?v=10&encoding=json"), *BaseURL);
            
            UE_LOG(LogDiscordGatewayCustom, Log, TEXT("Gateway URL: %s"), *GatewayURL);
            
            // Store resume gateway URL if available
            if (JsonObject->HasField(TEXT("resume_gateway_url")))
            {
                ResumeGatewayURL = JsonObject->GetStringField(TEXT("resume_gateway_url"));
            }
            
            // Connect to WebSocket
            ConnectWebSocket();
        }
        else
        {
            UE_LOG(LogDiscordGatewayCustom, Error, TEXT("Gateway response missing 'url' field"));
        }
    }
    else
    {
        UE_LOG(LogDiscordGatewayCustom, Error, TEXT("Failed to parse gateway response"));
    }
}

void ADiscordGatewayClientCustom::ConnectWebSocket()
{
    // Create custom WebSocket
    UE_LOG(LogDiscordGatewayCustom, Log, TEXT("Creating Custom WebSocket for URL: %s"), *GatewayURL);
    WebSocket = MakeShared<FCustomWebSocket>();
    
    if (!WebSocket.IsValid())
    {
        UE_LOG(LogDiscordGatewayCustom, Error, TEXT("Failed to create Custom WebSocket"));
        return;
    }

    // Bind WebSocket events
    WebSocket->OnConnected.BindUObject(this, &ADiscordGatewayClientCustom::OnWebSocketConnected);
    WebSocket->OnMessage.BindUObject(this, &ADiscordGatewayClientCustom::OnWebSocketMessage);
    WebSocket->OnClosed.BindUObject(this, &ADiscordGatewayClientCustom::OnWebSocketClosed);
    WebSocket->OnError.BindUObject(this, &ADiscordGatewayClientCustom::OnWebSocketError);

    // Connect
    UE_LOG(LogDiscordGatewayCustom, Log, TEXT("Connecting to WebSocket: %s"), *GatewayURL);
    if (!WebSocket->Connect(GatewayURL))
    {
        UE_LOG(LogDiscordGatewayCustom, Error, TEXT("Failed to initiate WebSocket connection"));
        OnWebSocketError(TEXT("Connection initiation failed"));
    }
}

void ADiscordGatewayClientCustom::OnWebSocketConnected(bool bSuccess)
{
    if (bSuccess)
    {
        UE_LOG(LogDiscordGatewayCustom, Log, TEXT("✅ Custom WebSocket connected successfully"));
        bIsConnected = true;
        ReconnectAttempts = 0;
        ReconnectDelay = 1.0f;
    }
    else
    {
        UE_LOG(LogDiscordGatewayCustom, Error, TEXT("❌ Custom WebSocket connection failed"));
        OnWebSocketError(TEXT("Connection failed"));
    }
}

void ADiscordGatewayClientCustom::OnWebSocketConnectionError(const FString& Error)
{
    UE_LOG(LogDiscordGatewayCustom, Error, TEXT("WebSocket connection error: %s"), *Error);
    bIsConnected = false;
}

void ADiscordGatewayClientCustom::OnWebSocketClosed(int32 StatusCode, const FString& Reason, bool bWasClean)
{
    UE_LOG(LogDiscordGatewayCustom, Warning, TEXT("WebSocket closed - Code: %d, Reason: %s, Clean: %s"), 
        StatusCode, *Reason, bWasClean ? TEXT("Yes") : TEXT("No"));
    
    bIsConnected = false;
    
    // Clear heartbeat timer
    if (GetWorld())
    {
        GetWorld()->GetTimerManager().ClearTimer(HeartbeatTimerHandle);
    }
    
    // Attempt reconnection for certain close codes
    if (StatusCode == 4000 || StatusCode == 4001 || StatusCode == 4002 || 
        StatusCode == 4003 || StatusCode == 4005 || StatusCode == 4007 ||
        StatusCode == 4008 || StatusCode == 4009)
    {
        UE_LOG(LogDiscordGatewayCustom, Log, TEXT("Attempting to reconnect..."));
        AttemptReconnect();
    }
}

void ADiscordGatewayClientCustom::OnWebSocketMessage(const FString& Message, bool bIsText)
{
    if (!bIsText)
    {
        UE_LOG(LogDiscordGatewayCustom, Warning, TEXT("Received binary message (not supported)"));
        return;
    }

    UE_LOG(LogDiscordGatewayCustom, Verbose, TEXT("Received message: %s"), *Message);

    TSharedPtr<FJsonObject> JsonObject;
    TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Message);
    
    if (FJsonSerializer::Deserialize(Reader, JsonObject) && JsonObject.IsValid())
    {
        int32 OpCode = JsonObject->GetIntegerField(TEXT("op"));
        
        // Update sequence number if present
        if (JsonObject->HasField(TEXT("s")) && !JsonObject->GetField(TEXT("s"))->IsNull())
        {
            SequenceNumber = JsonObject->GetIntegerField(TEXT("s"));
        }
        
        // Get event type for dispatch events
        FString EventType;
        if (OpCode == OPCODE_DISPATCH && JsonObject->HasField(TEXT("t")))
        {
            EventType = JsonObject->GetStringField(TEXT("t"));
        }
        
        // Get data payload
        TSharedPtr<FJsonObject> Data;
        if (JsonObject->HasField(TEXT("d")))
        {
            Data = JsonObject->GetObjectField(TEXT("d"));
        }
        
        HandleGatewayEvent(OpCode, Data, EventType);
    }
    else
    {
        UE_LOG(LogDiscordGatewayCustom, Error, TEXT("Failed to parse WebSocket message"));
    }
}

void ADiscordGatewayClientCustom::OnWebSocketError(const FString& Error)
{
    UE_LOG(LogDiscordGatewayCustom, Error, TEXT("WebSocket error: %s"), *Error);
    bIsConnected = false;
}

void ADiscordGatewayClientCustom::HandleGatewayEvent(int32 OpCode, const TSharedPtr<FJsonObject>& Data, const FString& EventType)
{
    switch (OpCode)
    {
    case OPCODE_HELLO:
        if (Data.IsValid() && Data->HasField(TEXT("heartbeat_interval")))
        {
            HeartbeatInterval = Data->GetIntegerField(TEXT("heartbeat_interval"));
            UE_LOG(LogDiscordGatewayCustom, Log, TEXT("Received HELLO - Heartbeat interval: %d ms"), HeartbeatInterval);
            
            // Send identify or resume
            if (!SessionId.IsEmpty())
            {
                SendResume();
            }
            else
            {
                SendIdentify();
            }
            
            // Start heartbeat timer
            if (GetWorld())
            {
                float HeartbeatIntervalSeconds = HeartbeatInterval / 1000.0f;
                GetWorld()->GetTimerManager().SetTimer(
                    HeartbeatTimerHandle,
                    this,
                    &ADiscordGatewayClientCustom::SendHeartbeat,
                    HeartbeatIntervalSeconds,
                    true
                );
                
                bHeartbeatAckReceived = true;
            }
        }
        break;
        
    case OPCODE_HEARTBEAT_ACK:
        UE_LOG(LogDiscordGatewayCustom, Verbose, TEXT("Received HEARTBEAT_ACK"));
        bHeartbeatAckReceived = true;
        break;
        
    case OPCODE_DISPATCH:
        UE_LOG(LogDiscordGatewayCustom, Log, TEXT("Received DISPATCH event: %s"), *EventType);
        
        if (EventType == TEXT("READY"))
        {
            if (Data.IsValid())
            {
                if (Data->HasField(TEXT("session_id")))
                {
                    SessionId = Data->GetStringField(TEXT("session_id"));
                    UE_LOG(LogDiscordGatewayCustom, Log, TEXT("✅ Bot ready! Session ID: %s"), *SessionId);
                }
                
                if (Data->HasField(TEXT("resume_gateway_url")))
                {
                    ResumeGatewayURL = Data->GetStringField(TEXT("resume_gateway_url"));
                }
            }
        }
        break;
        
    case OPCODE_RECONNECT:
        UE_LOG(LogDiscordGatewayCustom, Warning, TEXT("Server requested reconnect"));
        AttemptReconnect();
        break;
        
    case OPCODE_INVALID_SESSION:
        {
            bool bCanResume = Data.IsValid() && Data->HasField(TEXT("d")) && Data->GetBoolField(TEXT("d"));
            UE_LOG(LogDiscordGatewayCustom, Warning, TEXT("Invalid session - Can resume: %s"), 
                bCanResume ? TEXT("Yes") : TEXT("No"));
            
            if (!bCanResume)
            {
                SessionId.Empty();
                SequenceNumber = -1;
            }
            
            // Wait a bit before reconnecting
            FTimerHandle ReconnectTimer;
            if (GetWorld())
            {
                GetWorld()->GetTimerManager().SetTimer(ReconnectTimer, [this]()
                {
                    AttemptReconnect();
                }, 5.0f, false);
            }
        }
        break;
        
    default:
        UE_LOG(LogDiscordGatewayCustom, Verbose, TEXT("Unknown opcode: %d"), OpCode);
        break;
    }
}

void ADiscordGatewayClientCustom::SendIdentify()
{
    TSharedPtr<FJsonObject> IdentifyPayload = MakeShareable(new FJsonObject());
    IdentifyPayload->SetNumberField(TEXT("op"), OPCODE_IDENTIFY);
    
    TSharedPtr<FJsonObject> Data = MakeShareable(new FJsonObject());
    Data->SetStringField(TEXT("token"), BotToken);
    Data->SetNumberField(TEXT("intents"), Intents);
    
    TSharedPtr<FJsonObject> Properties = MakeShareable(new FJsonObject());
    Properties->SetStringField(TEXT("os"), TEXT("satisfactory-server"));
    Properties->SetStringField(TEXT("browser"), TEXT("satisfactory-mod"));
    Properties->SetStringField(TEXT("device"), TEXT("satisfactory-mod"));
    Data->SetObjectField(TEXT("properties"), Properties);
    
    IdentifyPayload->SetObjectField(TEXT("d"), Data);
    
    FString OutputString;
    TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&OutputString);
    FJsonSerializer::Serialize(IdentifyPayload.ToSharedRef(), Writer);
    
    UE_LOG(LogDiscordGatewayCustom, Log, TEXT("Sending IDENTIFY"));
    
    if (WebSocket.IsValid() && bIsConnected)
    {
        WebSocket->SendText(OutputString);
    }
}

void ADiscordGatewayClientCustom::SendResume()
{
    TSharedPtr<FJsonObject> ResumePayload = MakeShareable(new FJsonObject());
    ResumePayload->SetNumberField(TEXT("op"), OPCODE_RESUME);
    
    TSharedPtr<FJsonObject> Data = MakeShareable(new FJsonObject());
    Data->SetStringField(TEXT("token"), BotToken);
    Data->SetStringField(TEXT("session_id"), SessionId);
    Data->SetNumberField(TEXT("seq"), SequenceNumber);
    
    ResumePayload->SetObjectField(TEXT("d"), Data);
    
    FString OutputString;
    TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&OutputString);
    FJsonSerializer::Serialize(ResumePayload.ToSharedRef(), Writer);
    
    UE_LOG(LogDiscordGatewayCustom, Log, TEXT("Sending RESUME"));
    
    if (WebSocket.IsValid() && bIsConnected)
    {
        WebSocket->SendText(OutputString);
    }
}

void ADiscordGatewayClientCustom::SendHeartbeat()
{
    // Check if we received ACK for previous heartbeat
    if (!bHeartbeatAckReceived)
    {
        UE_LOG(LogDiscordGatewayCustom, Warning, TEXT("No heartbeat ACK received - connection may be dead"));
        // Reconnect
        Disconnect();
        AttemptReconnect();
        return;
    }
    
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
    
    UE_LOG(LogDiscordGatewayCustom, Verbose, TEXT("Sending HEARTBEAT (seq: %d)"), SequenceNumber);
    
    if (WebSocket.IsValid() && bIsConnected)
    {
        bHeartbeatAckReceived = false;
        WebSocket->SendText(OutputString);
    }
}

void ADiscordGatewayClientCustom::SendMessage(const FString& ChannelId, const FString& Message)
{
    if (!bIsConnected)
    {
        UE_LOG(LogDiscordGatewayCustom, Warning, TEXT("Cannot send message: Not connected"));
        return;
    }

    SendMessageHTTP(ChannelId, Message);
}

void ADiscordGatewayClientCustom::SendMessageHTTP(const FString& ChannelId, const FString& MessageContent)
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
            UE_LOG(LogDiscordGatewayCustom, Log, TEXT("Message sent successfully"));
        }
        else
        {
            UE_LOG(LogDiscordGatewayCustom, Error, TEXT("Failed to send message"));
        }
    });
    
    if (!Request->ProcessRequest())
    {
        UE_LOG(LogDiscordGatewayCustom, Error, TEXT("Failed to send message request"));
    }
}

void ADiscordGatewayClientCustom::AttemptReconnect()
{
    if (ReconnectAttempts >= 5)
    {
        UE_LOG(LogDiscordGatewayCustom, Error, TEXT("Max reconnection attempts reached. Giving up."));
        return;
    }
    
    ReconnectAttempts++;
    float Delay = ReconnectDelay * FMath::Pow(2.0f, ReconnectAttempts - 1); // Exponential backoff
    Delay = FMath::Min(Delay, 60.0f); // Cap at 60 seconds
    
    UE_LOG(LogDiscordGatewayCustom, Log, TEXT("Reconnecting in %.1f seconds (attempt %d/5)..."), Delay, ReconnectAttempts);
    
    if (GetWorld())
    {
        FTimerHandle ReconnectTimer;
        GetWorld()->GetTimerManager().SetTimer(ReconnectTimer, [this]()
        {
            if (!ResumeGatewayURL.IsEmpty())
            {
                GatewayURL = ResumeGatewayURL + TEXT("/?v=10&encoding=json");
            }
            ConnectWebSocket();
        }, Delay, false);
    }
}
