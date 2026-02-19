// Copyright (c) 2024 Yamahasxviper

#include "DiscordGatewayClientCustom.h"
#include "DiscordBotSubsystem.h"
#include "HttpModule.h"
#include "Interfaces/IHttpRequest.h"
#include "Interfaces/IHttpResponse.h"
#include "Json.h"
#include "JsonUtilities.h"
#include "TimerManager.h"
#include "Engine/World.h"
#include "Engine/GameInstance.h"

DEFINE_LOG_CATEGORY_STATIC(LogDiscordGatewayCustom, Log, All);

// Discord Gateway Opcodes
#define OPCODE_DISPATCH 0
#define OPCODE_HEARTBEAT 1
#define OPCODE_IDENTIFY 2
#define OPCODE_PRESENCE_UPDATE 3
#define OPCODE_HELLO 10
#define OPCODE_HEARTBEAT_ACK 11
#define OPCODE_RESUME 6

// Discord Intents
// Guilds Intent (1 << 0) = 1 - Basic guild events (required baseline)
// Server Members Intent (1 << 1) = 2 - PRIVILEGED
// Presence Intent (1 << 8) = 256 - PRIVILEGED
// Guild Messages Intent (1 << 9) = 512 - Required to receive MESSAGE_CREATE events
// Message Content Intent (1 << 15) = 32768 - PRIVILEGED
// Combined: 1 + 2 + 256 + 512 + 32768 = 33539
#define DISCORD_INTENTS_COMBINED 33539

ADiscordGatewayClientCustom::ADiscordGatewayClientCustom()
{
    // Base class ADiscordGatewayClient initializes:
    // PrimaryActorTick, GatewayURL, Intents, bIsConnected, HeartbeatInterval, SequenceNumber

    // Initialize custom-only default values
    bHeartbeatAckReceived = true;
    LastHeartbeatTime = 0.0f;
    HeartbeatCount = 0;
    ConnectionStartTime = 0.0f;
    ReconnectAttempts = 0;
    ReconnectDelay = 1.0f;
}

void ADiscordGatewayClientCustom::BeginPlay()
{
    Super::BeginPlay();
    
    UE_LOG(LogDiscordGatewayCustom, Log, TEXT("Discord Gateway Client (Custom WebSocket) initialized"));
    UE_LOG(LogDiscordGatewayCustom, Log, TEXT("Using platform-agnostic CustomWebSocket implementation"));
    
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

void ADiscordGatewayClientCustom::InitializeBot(const FString& InBotToken)
{
    BotToken = InBotToken;
    UE_LOG(LogDiscordGatewayCustom, Log, TEXT("Bot token set"));
}

void ADiscordGatewayClientCustom::Connect()
{
    if (BotToken.IsEmpty())
    {
        UE_LOG(LogDiscordGatewayCustom, Error, TEXT("Cannot connect: Bot token is empty"));
        return;
    }

    UE_LOG(LogDiscordGatewayCustom, Log, TEXT("Connecting to Discord Gateway..."));
    GetGatewayURL();
}

void ADiscordGatewayClientCustom::Disconnect()
{
    if (bIsConnected)
    {
        // Clear heartbeat timer
        if (GetWorld())
        {
            GetWorld()->GetTimerManager().ClearTimer(HeartbeatTimerHandle);
        }

        // Disconnect WebSocket
        if (WebSocket.IsValid())
        {
            WebSocket->Disconnect(1000, TEXT("Normal closure"));
        }

        bIsConnected = false;
        SessionId.Empty();
        SequenceNumber = -1;
        
        UE_LOG(LogDiscordGatewayCustom, Log, TEXT("Disconnected from Discord Gateway"));
    }
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
            GatewayURL = JsonObject->GetStringField(TEXT("url"));
            UE_LOG(LogDiscordGatewayCustom, Log, TEXT("Gateway URL: %s"), *GatewayURL);
            
            // Connect to WebSocket with the retrieved URL
            ConnectWebSocket();
        }
    }
}

void ADiscordGatewayClientCustom::ConnectWebSocket()
{
    // Create WebSocket instance if needed
    if (!WebSocket.IsValid())
    {
        WebSocket = MakeShareable(new FCustomWebSocket());
        
        // Bind event handlers
        WebSocket->OnConnected.BindUObject(this, &ADiscordGatewayClientCustom::OnWebSocketConnected);
        WebSocket->OnMessage.BindUObject(this, &ADiscordGatewayClientCustom::OnWebSocketMessage);
        WebSocket->OnClosed.BindUObject(this, &ADiscordGatewayClientCustom::OnWebSocketClosed);
        WebSocket->OnError.BindUObject(this, &ADiscordGatewayClientCustom::OnWebSocketError);
    }
    
    // Append WebSocket query parameters
    FString FullURL = GatewayURL + TEXT("/?v=10&encoding=json");
    
    UE_LOG(LogDiscordGatewayCustom, Log, TEXT("========================================"));
    UE_LOG(LogDiscordGatewayCustom, Log, TEXT("Discord Bot: Attempting to connect to Discord Gateway"));
    UE_LOG(LogDiscordGatewayCustom, Log, TEXT("Discord Bot: WebSocket URL: %s"), *FullURL);
    UE_LOG(LogDiscordGatewayCustom, Log, TEXT("Discord Bot: Using CustomWebSocket implementation"));
    UE_LOG(LogDiscordGatewayCustom, Log, TEXT("========================================"));
    
    if (!WebSocket->Connect(FullURL))
    {
        UE_LOG(LogDiscordGatewayCustom, Error, TEXT("Failed to connect to WebSocket"));
        AttemptReconnect();
    }
}

void ADiscordGatewayClientCustom::OnWebSocketConnected(bool bSuccess)
{
    if (bSuccess)
    {
        UE_LOG(LogDiscordGatewayCustom, Log, TEXT("========================================"));
        UE_LOG(LogDiscordGatewayCustom, Log, TEXT("Discord Bot: CustomWebSocket connection established!"));
        UE_LOG(LogDiscordGatewayCustom, Log, TEXT("Discord Bot: Waiting for Discord Gateway HELLO message..."));
        UE_LOG(LogDiscordGatewayCustom, Log, TEXT("========================================"));
        ReconnectAttempts = 0;
        ReconnectDelay = 1.0f;
        // Wait for HELLO message before setting bIsConnected = true
    }
    else
    {
        UE_LOG(LogDiscordGatewayCustom, Error, TEXT("WebSocket connection failed"));
        AttemptReconnect();
    }
}

void ADiscordGatewayClientCustom::OnWebSocketMessage(const FString& Message, bool bIsText)
{
    if (!bIsText)
    {
        UE_LOG(LogDiscordGatewayCustom, Warning, TEXT("Received binary message, expected text"));
        return;
    }
    
    UE_LOG(LogDiscordGatewayCustom, Verbose, TEXT("Received message: %s"), *Message);
    
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
        
        // Get event type for DISPATCH events
        FString EventType;
        if (OpCode == OPCODE_DISPATCH && JsonObject->HasField(TEXT("t")))
        {
            EventType = JsonObject->GetStringField(TEXT("t"));
        }
        
        // Get data payload
        TSharedPtr<FJsonObject> Data;
        if (JsonObject->HasField(TEXT("d")) && !JsonObject->GetField<EJson::None>(TEXT("d"))->IsNull())
        {
            Data = JsonObject->GetObjectField(TEXT("d"));
        }
        
        HandleGatewayEvent(OpCode, Data, EventType);
    }
}

void ADiscordGatewayClientCustom::OnWebSocketClosed(int32 StatusCode, const FString& Reason, bool bWasClean)
{
    UE_LOG(LogDiscordGatewayCustom, Warning, TEXT("WebSocket closed: %d - %s (clean: %s)"), 
        StatusCode, *Reason, bWasClean ? TEXT("yes") : TEXT("no"));
    
    bIsConnected = false;
    
    // Clear heartbeat timer
    if (GetWorld())
    {
        GetWorld()->GetTimerManager().ClearTimer(HeartbeatTimerHandle);
    }
    
    // Attempt reconnect for non-normal closures
    if (StatusCode != 1000)
    {
        AttemptReconnect();
    }
}

void ADiscordGatewayClientCustom::OnWebSocketError(const FString& Error)
{
    UE_LOG(LogDiscordGatewayCustom, Error, TEXT("WebSocket error: %s"), *Error);
}

void ADiscordGatewayClientCustom::HandleGatewayEvent(int32 OpCode, const TSharedPtr<FJsonObject>& Data, const FString& EventType)
{
    switch (OpCode)
    {
    case OPCODE_HELLO:
        if (Data.IsValid() && Data->HasField(TEXT("heartbeat_interval")))
        {
            HeartbeatInterval = Data->GetIntegerField(TEXT("heartbeat_interval"));
            UE_LOG(LogDiscordGatewayCustom, Log, TEXT("Heartbeat interval: %d ms"), HeartbeatInterval);
            
            // Start heartbeat timer
            if (GetWorld())
            {
                GetWorld()->GetTimerManager().SetTimer(
                    HeartbeatTimerHandle,
                    this,
                    &ADiscordGatewayClientCustom::SendHeartbeat,
                    HeartbeatInterval / 1000.0f,
                    true
                );
            }
            
            // Send identify or resume
            if (!SessionId.IsEmpty() && !ResumeGatewayURL.IsEmpty())
            {
                SendResume();
            }
            else
            {
                SendIdentify();
            }
        }
        break;
        
    case OPCODE_HEARTBEAT_ACK:
        UE_LOG(LogDiscordGatewayCustom, Verbose, TEXT("Heartbeat acknowledged"));
        bHeartbeatAckReceived = true;
        break;
        
    case OPCODE_DISPATCH:
        if (EventType == TEXT("READY"))
        {
            bIsConnected = true;
            ConnectionStartTime = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;
            HeartbeatCount = 0;
            if (Data.IsValid() && Data->HasField(TEXT("session_id")))
            {
                SessionId = Data->GetStringField(TEXT("session_id"));
                UE_LOG(LogDiscordGatewayCustom, Log, TEXT("****************************************"));
                UE_LOG(LogDiscordGatewayCustom, Log, TEXT("*** DISCORD BOT FULLY CONNECTED AND READY! ***"));
                UE_LOG(LogDiscordGatewayCustom, Log, TEXT("*** CustomWebSocket successfully connected to Discord Gateway ***"));
                UE_LOG(LogDiscordGatewayCustom, Log, TEXT("*** Session ID: %s ***"), *SessionId);
                UE_LOG(LogDiscordGatewayCustom, Log, TEXT("*** Internet connection: ACTIVE ***"));
                UE_LOG(LogDiscordGatewayCustom, Log, TEXT("****************************************"));
            }
            if (Data.IsValid() && Data->HasField(TEXT("resume_gateway_url")))
            {
                ResumeGatewayURL = Data->GetStringField(TEXT("resume_gateway_url"));
            }
        }
        else if (EventType == TEXT("MESSAGE_CREATE"))
        {
            // Handle message creation events - relay to subsystem
            if (Data.IsValid())
            {
                // Get channel ID
                FString ChannelId;
                if (Data->HasField(TEXT("channel_id")))
                {
                    ChannelId = Data->GetStringField(TEXT("channel_id"));
                }
                
                // Get message content
                FString Content;
                if (Data->HasField(TEXT("content")))
                {
                    Content = Data->GetStringField(TEXT("content"));
                }
                
                // Get author info
                FString Username;
                bool bIsBot = false;
                if (Data->HasField(TEXT("author")))
                {
                    TSharedPtr<FJsonObject> Author = Data->GetObjectField(TEXT("author"));
                    if (Author.IsValid())
                    {
                        if (Author->HasField(TEXT("username")))
                        {
                            Username = Author->GetStringField(TEXT("username"));
                        }
                        if (Author->HasField(TEXT("bot")))
                        {
                            bIsBot = Author->GetBoolField(TEXT("bot"));
                        }
                    }
                }
                
                // Don't relay bot messages
                if (!bIsBot && !Content.IsEmpty())
                {
                    // Get subsystem and relay message
                    if (GetWorld() && GetWorld()->GetGameInstance())
                    {
                        UDiscordBotSubsystem* Subsystem = GetWorld()->GetGameInstance()->GetSubsystem<UDiscordBotSubsystem>();
                        if (Subsystem)
                        {
                            Subsystem->OnDiscordMessageReceived(ChannelId, Username, Content);
                        }
                    }
                }
            }
        }
        UE_LOG(LogDiscordGatewayCustom, Verbose, TEXT("Dispatch event: %s"), *EventType);
        break;
        
    case OPCODE_HEARTBEAT:
        // Server requesting immediate heartbeat
        SendHeartbeat();
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
    Properties->SetStringField(TEXT("os"), TEXT("windows"));
    Properties->SetStringField(TEXT("browser"), TEXT("satisfactory-mod"));
    Properties->SetStringField(TEXT("device"), TEXT("satisfactory-mod"));
    Data->SetObjectField(TEXT("properties"), Properties);
    
    IdentifyPayload->SetObjectField(TEXT("d"), Data);
    
    FString OutputString;
    TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&OutputString);
    FJsonSerializer::Serialize(IdentifyPayload.ToSharedRef(), Writer);
    
    if (WebSocket.IsValid())
    {
        WebSocket->SendText(OutputString);
        UE_LOG(LogDiscordGatewayCustom, Log, TEXT("Sent IDENTIFY payload"));
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
    
    if (WebSocket.IsValid())
    {
        WebSocket->SendText(OutputString);
        UE_LOG(LogDiscordGatewayCustom, Log, TEXT("Sent RESUME payload"));
    }
}

void ADiscordGatewayClientCustom::SendHeartbeat()
{
    if (!bHeartbeatAckReceived)
    {
        UE_LOG(LogDiscordGatewayCustom, Warning, TEXT("Heartbeat ACK not received, connection may be zombie"));
        // Could disconnect and reconnect here
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
    
    if (WebSocket.IsValid())
    {
        WebSocket->SendText(OutputString);
        bHeartbeatAckReceived = false;
        LastHeartbeatTime = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;
        HeartbeatCount++;
        
        UE_LOG(LogDiscordGatewayCustom, Verbose, TEXT("Heartbeat sent"));
        
        // Log connection status every 10 heartbeats (approximately every 7 minutes)
        if (HeartbeatCount % 10 == 0)
        {
            float CurrentTime = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;
            float UptimeMinutes = (CurrentTime - ConnectionStartTime) / 60.0f;
            
            UE_LOG(LogDiscordGatewayCustom, Log, TEXT("========================================"));
            UE_LOG(LogDiscordGatewayCustom, Log, TEXT("Connection Status Report"));
            UE_LOG(LogDiscordGatewayCustom, Log, TEXT("Status: CONNECTED"));
            UE_LOG(LogDiscordGatewayCustom, Log, TEXT("WebSocket: ACTIVE"));
            UE_LOG(LogDiscordGatewayCustom, Log, TEXT("Internet Connection: ACTIVE"));
            UE_LOG(LogDiscordGatewayCustom, Log, TEXT("Uptime: %.1f minutes"), UptimeMinutes);
            UE_LOG(LogDiscordGatewayCustom, Log, TEXT("Heartbeats sent: %d"), HeartbeatCount);
            UE_LOG(LogDiscordGatewayCustom, Log, TEXT("Session ID: %s"), *SessionId);
            UE_LOG(LogDiscordGatewayCustom, Log, TEXT("========================================"));
        }
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

void ADiscordGatewayClientCustom::UpdatePresence(const FString& StatusMessage, int32 ActivityType)
{
    if (!bIsConnected)
    {
        UE_LOG(LogDiscordGatewayCustom, Warning, TEXT("Cannot update presence: Bot not connected"));
        return;
    }
    
    UE_LOG(LogDiscordGatewayCustom, Log, TEXT("Updating bot presence: %s (Type: %d)"), *StatusMessage, ActivityType);
    
    // Create presence update payload
    TSharedPtr<FJsonObject> PresencePayload = MakeShareable(new FJsonObject());
    PresencePayload->SetNumberField(TEXT("op"), OPCODE_PRESENCE_UPDATE);
    
    TSharedPtr<FJsonObject> Data = MakeShareable(new FJsonObject());
    
    // Set status (online, idle, dnd, invisible)
    Data->SetStringField(TEXT("status"), TEXT("online"));
    
    // Set activities array with one activity
    TArray<TSharedPtr<FJsonValue>> Activities;
    TSharedPtr<FJsonObject> Activity = MakeShareable(new FJsonObject());
    Activity->SetStringField(TEXT("name"), StatusMessage);
    Activity->SetNumberField(TEXT("type"), ActivityType); // 0 = Playing, 1 = Streaming, 2 = Listening, 3 = Watching, 5 = Competing
    Activities.Add(MakeShareable(new FJsonValueObject(Activity)));
    Data->SetArrayField(TEXT("activities"), Activities);
    
    // AFK and since fields
    // 'since' must be null when the bot is not idle (per Discord Gateway spec)
    Data->SetBoolField(TEXT("afk"), false);
    Data->SetField(TEXT("since"), MakeShareable(new FJsonValueNull()));
    
    PresencePayload->SetObjectField(TEXT("d"), Data);
    
    FString OutputString;
    TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&OutputString);
    FJsonSerializer::Serialize(PresencePayload.ToSharedRef(), Writer);
    
    if (WebSocket.IsValid())
    {
        WebSocket->SendText(OutputString);
        UE_LOG(LogDiscordGatewayCustom, Log, TEXT("Presence update sent"));
    }
}

void ADiscordGatewayClientCustom::AttemptReconnect()
{
    ReconnectAttempts++;
    
    if (ReconnectAttempts > 5)
    {
        UE_LOG(LogDiscordGatewayCustom, Error, TEXT("Max reconnect attempts reached"));
        return;
    }
    
    // Exponential backoff
    ReconnectDelay = FMath::Min(ReconnectDelay * 2.0f, 60.0f);
    
    UE_LOG(LogDiscordGatewayCustom, Warning, TEXT("Attempting reconnect in %.1f seconds (attempt %d)"), ReconnectDelay, ReconnectAttempts);
    
    // Schedule reconnect
    if (GetWorld())
    {
        FTimerHandle ReconnectTimer;
        GetWorld()->GetTimerManager().SetTimer(ReconnectTimer, [this]()
        {
            ConnectWebSocket();
        }, ReconnectDelay, false);
    }
}
