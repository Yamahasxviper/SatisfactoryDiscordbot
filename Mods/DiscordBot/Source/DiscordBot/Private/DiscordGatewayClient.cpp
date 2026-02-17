// Copyright (c) 2024 Yamahasxviper

#include "DiscordGatewayClient.h"
#include "HttpModule.h"
#include "Interfaces/IHttpRequest.h"
#include "Interfaces/IHttpResponse.h"
#include "Json.h"
#include "JsonUtilities.h"
#include "TimerManager.h"
#include "Engine/World.h"

DEFINE_LOG_CATEGORY_STATIC(LogDiscordGateway, Log, All);

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

ADiscordGatewayClient::ADiscordGatewayClient()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.bStartWithTickEnabled = true;

    // Initialize default values
    GatewayURL = TEXT("wss://gateway.discord.gg");
    Intents = DISCORD_INTENTS_COMBINED;
    bIsConnected = false;
    HeartbeatInterval = 41250; // Default Discord heartbeat interval
    SequenceNumber = -1;

    // Create SocketIO component
    SocketIOComponent = CreateDefaultSubobject<USocketIOClientComponent>(TEXT("SocketIOComponent"));
}

void ADiscordGatewayClient::BeginPlay()
{
    Super::BeginPlay();
    
    UE_LOG(LogDiscordGateway, Log, TEXT("Discord Gateway Client initialized"));
    
    // Get HTTP module
    HttpModule = &FHttpModule::Get();
}

void ADiscordGatewayClient::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    Disconnect();
    Super::EndPlay(EndPlayReason);
}

void ADiscordGatewayClient::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
}

void ADiscordGatewayClient::InitializeBot(const FString& InBotToken)
{
    BotToken = InBotToken;
    UE_LOG(LogDiscordGateway, Log, TEXT("Bot token set"));
}

void ADiscordGatewayClient::Connect()
{
    if (BotToken.IsEmpty())
    {
        UE_LOG(LogDiscordGateway, Error, TEXT("Cannot connect: Bot token is empty"));
        return;
    }

    UE_LOG(LogDiscordGateway, Log, TEXT("Connecting to Discord Gateway..."));
    GetGatewayURL();
}

void ADiscordGatewayClient::Disconnect()
{
    if (bIsConnected && SocketIOComponent)
    {
        // Clear heartbeat timer
        if (GetWorld())
        {
            GetWorld()->GetTimerManager().ClearTimer(HeartbeatTimerHandle);
        }

        bIsConnected = false;
        SessionId.Empty();
        SequenceNumber = -1;
        
        UE_LOG(LogDiscordGateway, Log, TEXT("Disconnected from Discord Gateway"));
    }
}

void ADiscordGatewayClient::GetGatewayURL()
{
    TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request = HttpModule->CreateRequest();
    Request->SetURL(TEXT("https://discord.com/api/v10/gateway/bot"));
    Request->SetVerb(TEXT("GET"));
    Request->SetHeader(TEXT("Authorization"), FString::Printf(TEXT("Bot %s"), *BotToken));
    Request->SetHeader(TEXT("Content-Type"), TEXT("application/json"));
    
    Request->OnProcessRequestComplete().BindUObject(this, &ADiscordGatewayClient::OnGetGatewayURLComplete);
    
    if (!Request->ProcessRequest())
    {
        UE_LOG(LogDiscordGateway, Error, TEXT("Failed to send gateway URL request"));
    }
}

void ADiscordGatewayClient::OnGetGatewayURLComplete(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
{
    if (!bWasSuccessful || !Response.IsValid())
    {
        UE_LOG(LogDiscordGateway, Error, TEXT("Failed to get gateway URL"));
        return;
    }

    const FString ResponseStr = Response->GetContentAsString();
    UE_LOG(LogDiscordGateway, Log, TEXT("Gateway response: %s"), *ResponseStr);

    TSharedPtr<FJsonObject> JsonObject;
    TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(ResponseStr);
    
    if (FJsonSerializer::Deserialize(Reader, JsonObject) && JsonObject.IsValid())
    {
        if (JsonObject->HasField(TEXT("url")))
        {
            GatewayURL = JsonObject->GetStringField(TEXT("url"));
            UE_LOG(LogDiscordGateway, Log, TEXT("Gateway URL: %s"), *GatewayURL);
            
            // Connect to WebSocket with the retrieved URL
            ConnectWebSocket();
        }
    }
}

void ADiscordGatewayClient::ConnectWebSocket()
{
    if (!SocketIOComponent)
    {
        UE_LOG(LogDiscordGateway, Error, TEXT("SocketIO component is null"));
        return;
    }

    // Note: Discord Gateway uses WebSocket protocol, not Socket.IO
    // For a production implementation, you would need a native WebSocket client
    // This is a simplified example showing the structure
    
    UE_LOG(LogDiscordGateway, Log, TEXT("WebSocket connection would be established here"));
    UE_LOG(LogDiscordGateway, Log, TEXT("Gateway URL: %s"), *GatewayURL);
    UE_LOG(LogDiscordGateway, Log, TEXT("Intents: %d (Presence: 256, Server Members: 2, Message Content: 32768)"), Intents);
    
    // In a real implementation, you would:
    // 1. Connect to the WebSocket
    // 2. Receive HELLO (opcode 10) with heartbeat_interval
    // 3. Send IDENTIFY (opcode 2) with token and intents
    // 4. Start heartbeat timer
    // 5. Handle incoming events
    
    bIsConnected = true;
}

void ADiscordGatewayClient::SendIdentify()
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
    
    UE_LOG(LogDiscordGateway, Log, TEXT("Identify payload: %s"), *OutputString);
}

void ADiscordGatewayClient::SendHeartbeat()
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
    
    UE_LOG(LogDiscordGateway, Verbose, TEXT("Heartbeat sent: %s"), *OutputString);
}

void ADiscordGatewayClient::SendMessage(const FString& ChannelId, const FString& Message)
{
    if (!bIsConnected)
    {
        UE_LOG(LogDiscordGateway, Warning, TEXT("Cannot send message: Not connected"));
        return;
    }

    SendMessageHTTP(ChannelId, Message);
}

void ADiscordGatewayClient::SendMessageHTTP(const FString& ChannelId, const FString& MessageContent)
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
            UE_LOG(LogDiscordGateway, Log, TEXT("Message sent successfully"));
        }
        else
        {
            UE_LOG(LogDiscordGateway, Error, TEXT("Failed to send message"));
        }
    });
    
    if (!Request->ProcessRequest())
    {
        UE_LOG(LogDiscordGateway, Error, TEXT("Failed to send message request"));
    }
}

void ADiscordGatewayClient::OnMessageReceived(const FString& Message)
{
    UE_LOG(LogDiscordGateway, Log, TEXT("Received message: %s"), *Message);
    
    TSharedPtr<FJsonObject> JsonObject;
    TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Message);
    
    if (FJsonSerializer::Deserialize(Reader, JsonObject) && JsonObject.IsValid())
    {
        int32 OpCode = JsonObject->GetIntegerField(TEXT("op"));
        TSharedPtr<FJsonObject> Data = JsonObject->GetObjectField(TEXT("d"));
        
        if (JsonObject->HasField(TEXT("s")) && !JsonObject->GetField<EJson::None>(TEXT("s"))->IsNull())
        {
            SequenceNumber = JsonObject->GetIntegerField(TEXT("s"));
        }
        
        HandleGatewayEvent(OpCode, Data);
    }
}

void ADiscordGatewayClient::HandleGatewayEvent(int32 OpCode, const TSharedPtr<FJsonObject>& Data)
{
    switch (OpCode)
    {
    case OPCODE_HELLO:
        if (Data.IsValid() && Data->HasField(TEXT("heartbeat_interval")))
        {
            HeartbeatInterval = Data->GetIntegerField(TEXT("heartbeat_interval"));
            UE_LOG(LogDiscordGateway, Log, TEXT("Heartbeat interval: %d ms"), HeartbeatInterval);
            
            // Start heartbeat timer
            if (GetWorld())
            {
                GetWorld()->GetTimerManager().SetTimer(
                    HeartbeatTimerHandle,
                    this,
                    &ADiscordGatewayClient::SendHeartbeat,
                    HeartbeatInterval / 1000.0f,
                    true
                );
            }
            
            // Send identify
            SendIdentify();
        }
        break;
        
    case OPCODE_HEARTBEAT_ACK:
        UE_LOG(LogDiscordGateway, Verbose, TEXT("Heartbeat acknowledged"));
        break;
        
    case OPCODE_DISPATCH:
        // Handle various Discord events here
        UE_LOG(LogDiscordGateway, Log, TEXT("Dispatch event received"));
        break;
    }
}
