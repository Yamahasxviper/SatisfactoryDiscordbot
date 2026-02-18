// Copyright (c) 2024 Yamahasxviper

#include "DiscordGatewayClientNative.h"
#include "HttpModule.h"
#include "Interfaces/IHttpRequest.h"
#include "Interfaces/IHttpResponse.h"
#include "Json.h"
#include "JsonUtilities.h"
#include "TimerManager.h"
#include "Engine/World.h"
#include "Modules/ModuleManager.h"

// Check if WebSocket headers are available at compile time
#if __has_include("IWebSocket.h")
    #include "IWebSocket.h"
    #include "WebSocketsModule.h"
    #define WEBSOCKETS_AVAILABLE 1
#else
    #define WEBSOCKETS_AVAILABLE 0
    #pragma message("WARNING: WebSocket headers not found! Discord bot will not work.")
#endif

DEFINE_LOG_CATEGORY_STATIC(LogDiscordGatewayNative, Log, All);

// Discord Gateway Opcodes
#define OPCODE_DISPATCH 0
#define OPCODE_HEARTBEAT 1
#define OPCODE_IDENTIFY 2
#define OPCODE_HELLO 10
#define OPCODE_HEARTBEAT_ACK 11

// Discord Intents
// Guilds Intent (1 << 0) = 1 - Basic guild events (required baseline)
// Server Members Intent (1 << 1) = 2 - PRIVILEGED
// Presence Intent (1 << 8) = 256 - PRIVILEGED
// Message Content Intent (1 << 15) = 32768 - PRIVILEGED
// Combined: 1 + 2 + 256 + 32768 = 33027
#define DISCORD_INTENTS_COMBINED 33027

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
    
#if !WEBSOCKETS_AVAILABLE
    UE_LOG(LogDiscordGatewayNative, Error, TEXT("========================================"));
    UE_LOG(LogDiscordGatewayNative, Error, TEXT("❌ CRITICAL: WebSocket headers NOT found!"));
    UE_LOG(LogDiscordGatewayNative, Error, TEXT("   The WebSockets module is not available in your engine build."));
    UE_LOG(LogDiscordGatewayNative, Error, TEXT("   Discord bot will NOT work!"));
    UE_LOG(LogDiscordGatewayNative, Error, TEXT("   See WEBSOCKET_TROUBLESHOOTING.md for help"));
    UE_LOG(LogDiscordGatewayNative, Error, TEXT("========================================"));
#else
    // Check if module is available at runtime
    if (!FModuleManager::Get().IsModuleLoaded("WebSockets"))
    {
        TArray<FModuleStatus> Modules;
        FModuleManager::Get().QueryModules(Modules);
        
        bool bFound = false;
        for (const FModuleStatus& Module : Modules)
        {
            if (Module.Name == TEXT("WebSockets"))
            {
                bFound = true;
                break;
            }
        }
        
        if (!bFound)
        {
            UE_LOG(LogDiscordGatewayNative, Error, TEXT("========================================"));
            UE_LOG(LogDiscordGatewayNative, Error, TEXT("❌ WARNING: WebSockets module not registered!"));
            UE_LOG(LogDiscordGatewayNative, Error, TEXT("   The module may not be available in your engine."));
            UE_LOG(LogDiscordGatewayNative, Error, TEXT("   Use WebSocketModuleVerifier to diagnose."));
            UE_LOG(LogDiscordGatewayNative, Error, TEXT("   See WEBSOCKET_TROUBLESHOOTING.md for help"));
            UE_LOG(LogDiscordGatewayNative, Error, TEXT("========================================"));
        }
        else
        {
            UE_LOG(LogDiscordGatewayNative, Log, TEXT("WebSockets module found (not loaded yet)"));
        }
    }
    else
    {
        UE_LOG(LogDiscordGatewayNative, Log, TEXT("WebSockets module already loaded"));
    }
#endif
    
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
#if !WEBSOCKETS_AVAILABLE
    UE_LOG(LogDiscordGatewayNative, Error, TEXT("Cannot connect: WebSocket headers not available at compile time"));
    UE_LOG(LogDiscordGatewayNative, Error, TEXT("Your engine build does not include the WebSockets module"));
    UE_LOG(LogDiscordGatewayNative, Error, TEXT("See WEBSOCKET_TROUBLESHOOTING.md for help"));
    return;
#else
    // Try to load module if not already loaded
    if (!FModuleManager::Get().IsModuleLoaded("WebSockets"))
    {
        UE_LOG(LogDiscordGatewayNative, Log, TEXT("Loading WebSockets module..."));
        
        // Check if module exists before trying to load
        TArray<FModuleStatus> Modules;
        FModuleManager::Get().QueryModules(Modules);
        
        bool bModuleExists = false;
        for (const FModuleStatus& Module : Modules)
        {
            if (Module.Name == TEXT("WebSockets"))
            {
                bModuleExists = true;
                break;
            }
        }
        
        if (!bModuleExists)
        {
            UE_LOG(LogDiscordGatewayNative, Error, TEXT("========================================"));
            UE_LOG(LogDiscordGatewayNative, Error, TEXT("❌ CRITICAL: WebSockets module NOT found!"));
            UE_LOG(LogDiscordGatewayNative, Error, TEXT("   Module is not registered in this engine build"));
            UE_LOG(LogDiscordGatewayNative, Error, TEXT("   "));
            UE_LOG(LogDiscordGatewayNative, Error, TEXT("   Run WebSocketModuleVerifier to diagnose:"));
            UE_LOG(LogDiscordGatewayNative, Error, TEXT("   1. Spawn WebSocketModuleVerifier actor"));
            UE_LOG(LogDiscordGatewayNative, Error, TEXT("   2. Call RunFullVerification()"));
            UE_LOG(LogDiscordGatewayNative, Error, TEXT("   3. Check log for detailed report"));
            UE_LOG(LogDiscordGatewayNative, Error, TEXT("   "));
            UE_LOG(LogDiscordGatewayNative, Error, TEXT("   See WEBSOCKET_TROUBLESHOOTING.md for solutions"));
            UE_LOG(LogDiscordGatewayNative, Error, TEXT("========================================"));
            return;
        }
        
        // Try to load the module
        FModuleManager::Get().LoadModule("WebSockets");
        
        if (!FModuleManager::Get().IsModuleLoaded("WebSockets"))
        {
            UE_LOG(LogDiscordGatewayNative, Error, TEXT("========================================"));
            UE_LOG(LogDiscordGatewayNative, Error, TEXT("❌ CRITICAL: Failed to load WebSockets module!"));
            UE_LOG(LogDiscordGatewayNative, Error, TEXT("   Module exists but failed to load"));
            UE_LOG(LogDiscordGatewayNative, Error, TEXT("   This may indicate:"));
            UE_LOG(LogDiscordGatewayNative, Error, TEXT("   - Missing dependencies"));
            UE_LOG(LogDiscordGatewayNative, Error, TEXT("   - Corrupted module files"));
            UE_LOG(LogDiscordGatewayNative, Error, TEXT("   - Platform incompatibility"));
            UE_LOG(LogDiscordGatewayNative, Error, TEXT("   "));
            UE_LOG(LogDiscordGatewayNative, Error, TEXT("   See WEBSOCKET_TROUBLESHOOTING.md for solutions"));
            UE_LOG(LogDiscordGatewayNative, Error, TEXT("========================================"));
            return;
        }
        
        UE_LOG(LogDiscordGatewayNative, Log, TEXT("✅ WebSockets module loaded successfully"));
    }

    // Create native WebSocket using Unreal's built-in module
    UE_LOG(LogDiscordGatewayNative, Log, TEXT("Creating WebSocket for URL: %s"), *GatewayURL);
    WebSocket = FWebSocketsModule::Get().CreateWebSocket(GatewayURL);
    
    if (!WebSocket.IsValid())
    {
        UE_LOG(LogDiscordGatewayNative, Error, TEXT("========================================"));
        UE_LOG(LogDiscordGatewayNative, Error, TEXT("❌ Failed to create WebSocket"));
        UE_LOG(LogDiscordGatewayNative, Error, TEXT("   URL: %s"), *GatewayURL);
        UE_LOG(LogDiscordGatewayNative, Error, TEXT("   This may indicate:"));
        UE_LOG(LogDiscordGatewayNative, Error, TEXT("   - Invalid URL format"));
        UE_LOG(LogDiscordGatewayNative, Error, TEXT("   - WebSocket factory failure"));
        UE_LOG(LogDiscordGatewayNative, Error, TEXT("   - Platform-specific issue"));
        UE_LOG(LogDiscordGatewayNative, Error, TEXT("========================================"));
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
#endif
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
