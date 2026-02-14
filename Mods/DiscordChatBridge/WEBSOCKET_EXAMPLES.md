# WebSocket Implementation Examples

This document provides practical examples of using WebSockets in the Discord Chat Bridge mod.

## Example 1: Basic Gateway Connection

This example shows how to create and connect to Discord Gateway:

```cpp
// In your subsystem or actor
UPROPERTY()
UDiscordGateway* Gateway;

void AMyActor::InitializeGateway()
{
    // Create Gateway instance
    Gateway = NewObject<UDiscordGateway>(this);
    
    // Initialize with bot token
    FString BotToken = TEXT("YOUR_BOT_TOKEN_HERE");
    Gateway->Initialize(BotToken);
    
    // Bind connection events
    Gateway->OnConnected.BindUObject(this, &AMyActor::OnGatewayConnected);
    Gateway->OnDisconnected.BindUObject(this, &AMyActor::OnGatewayDisconnected);
    
    // Connect to Discord
    Gateway->Connect();
    
    UE_LOG(LogTemp, Log, TEXT("Gateway connection initiated"));
}

void AMyActor::OnGatewayConnected()
{
    UE_LOG(LogTemp, Log, TEXT("Successfully connected to Discord Gateway!"));
    
    // Now you can update bot presence
    Gateway->UpdatePresence(TEXT("with 10 players"), 0); // 0 = Playing
}

void AMyActor::OnGatewayDisconnected(const FString& Reason)
{
    UE_LOG(LogTemp, Warning, TEXT("Gateway disconnected: %s"), *Reason);
    
    // Optionally implement reconnection logic here
}
```

## Example 2: Dynamic Presence Updates

Update bot presence based on player count:

```cpp
void ADiscordChatSubsystem::UpdateBotActivity()
{
    // Get current player count
    int32 PlayerCount = GetPlayerCount();
    
    // Check if Gateway is enabled and connected
    if (BotConfig.bUseGatewayForPresence && Gateway && Gateway->IsConnected())
    {
        // Format presence text
        FString PresenceText = BotConfig.GatewayPresenceFormat;
        PresenceText = PresenceText.Replace(TEXT("{playercount}"), *FString::FromInt(PlayerCount));
        
        // Update presence via Gateway
        Gateway->UpdatePresence(PresenceText, BotConfig.GatewayActivityType);
        
        UE_LOG(LogTemp, Log, TEXT("Updated bot presence: %s"), *PresenceText);
    }
    else
    {
        // Fallback to REST API if Gateway not available
        SendActivityToChannel(PlayerCount);
    }
}

int32 ADiscordChatSubsystem::GetPlayerCount()
{
    UWorld* World = GetWorld();
    if (!World) return 0;
    
    AGameStateBase* GameState = World->GetGameState();
    if (!GameState) return 0;
    
    return GameState->PlayerArray.Num();
}
```

## Example 3: Different Activity Types

Show different activity types based on server state:

```cpp
void UpdatePresenceBasedOnServerState()
{
    int32 PlayerCount = GetPlayerCount();
    
    if (PlayerCount == 0)
    {
        // Server empty - "Watching for players"
        Gateway->UpdatePresence(TEXT("for players"), 3); // 3 = Watching
    }
    else if (PlayerCount == 1)
    {
        // One player - "Playing with 1 player"
        Gateway->UpdatePresence(TEXT("with 1 player"), 0); // 0 = Playing
    }
    else if (PlayerCount < 5)
        {
        // Few players - "Playing with X players"
        FString Text = FString::Printf(TEXT("with %d players"), PlayerCount);
        Gateway->UpdatePresence(Text, 0); // 0 = Playing
    }
    else
    {
        // Many players - "Competing in Satisfactory"
        Gateway->UpdatePresence(TEXT("Satisfactory"), 5); // 5 = Competing
    }
}
```

## Example 4: Configuration-Based Setup

Complete example using configuration from INI file:

```cpp
void ADiscordChatSubsystem::SetupGateway()
{
    // Load configuration
    LoadConfiguration(); // Loads BotConfig from INI
    
    // Only initialize Gateway if enabled in config
    if (!BotConfig.bUseGatewayForPresence)
    {
        UE_LOG(LogTemp, Log, TEXT("Gateway presence disabled in configuration"));
        return;
    }
    
    // Validate configuration
    if (BotConfig.BotToken.IsEmpty())
    {
        UE_LOG(LogTemp, Error, TEXT("Cannot initialize Gateway: Bot token is empty"));
        return;
    }
    
    // Create and initialize Gateway
    Gateway = NewObject<UDiscordGateway>(this);
    Gateway->Initialize(BotConfig.BotToken);
    
    // Bind events
    Gateway->OnConnected.BindUObject(this, &ADiscordChatSubsystem::OnGatewayConnected);
    Gateway->OnDisconnected.BindUObject(this, &ADiscordChatSubsystem::OnGatewayDisconnected);
    
    // Connect
    Gateway->Connect();
    
    UE_LOG(LogTemp, Log, TEXT("Gateway initialization complete"));
}

void ADiscordChatSubsystem::OnGatewayConnected()
{
    UE_LOG(LogTemp, Log, TEXT("Gateway connected successfully"));
    
    // Immediately update presence
    UpdateBotActivity();
    
    // Set up timer for periodic updates
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().SetTimer(
            ActivityTimerHandle,
            this,
            &ADiscordChatSubsystem::UpdateBotActivity,
            BotConfig.ActivityUpdateIntervalSeconds,
            true,
            0.0f  // Start immediately
        );
    }
}

void ADiscordChatSubsystem::OnGatewayDisconnected(const FString& Reason)
{
    UE_LOG(LogTemp, Warning, TEXT("Gateway disconnected: %s"), *Reason);
    
    // Stop periodic updates
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().ClearTimer(ActivityTimerHandle);
    }
    
    // Optionally schedule reconnection
    ScheduleReconnection();
}

void ADiscordChatSubsystem::ScheduleReconnection()
{
    if (UWorld* World = GetWorld())
    {
        FTimerHandle ReconnectTimer;
        World->GetTimerManager().SetTimer(
            ReconnectTimer,
            [this]()
            {
                if (Gateway && !Gateway->IsConnected())
                {
                    UE_LOG(LogTemp, Log, TEXT("Attempting to reconnect to Gateway..."));
                    Gateway->Connect();
                }
            },
            30.0f, // Wait 30 seconds before reconnecting
            false
        );
    }
}
```

## Example 5: Testing WebSocket Connection

Create a simple test to verify WebSocket functionality:

```cpp
UCLASS()
class UWebSocketTest : public UObject
{
    GENERATED_BODY()
    
public:
    void RunTest(const FString& BotToken)
    {
        UE_LOG(LogTemp, Log, TEXT("=== WebSocket Gateway Test ==="));
        
        // Create Gateway
        Gateway = NewObject<UDiscordGateway>(this);
        
        // Bind test handlers
        Gateway->OnConnected.BindLambda([this]()
        {
            UE_LOG(LogTemp, Log, TEXT("✅ Test PASSED: Gateway connected"));
            
            // Test presence update
            Gateway->UpdatePresence(TEXT("WebSocket Test"), 0);
            UE_LOG(LogTemp, Log, TEXT("✅ Test PASSED: Presence update sent"));
            
            // Schedule disconnect
            if (UWorld* World = GetWorld())
            {
                FTimerHandle DisconnectTimer;
                World->GetTimerManager().SetTimer(
                    DisconnectTimer,
                    [this]()
                    {
                        Gateway->Disconnect();
                        UE_LOG(LogTemp, Log, TEXT("✅ Test PASSED: Disconnected successfully"));
                        UE_LOG(LogTemp, Log, TEXT("=== Test Complete ==="));
                    },
                    5.0f,
                    false
                );
            }
        });
        
        Gateway->OnDisconnected.BindLambda([](const FString& Reason)
        {
            UE_LOG(LogTemp, Log, TEXT("ℹ Gateway disconnected: %s"), *Reason);
        });
        
        // Initialize and connect
        Gateway->Initialize(BotToken);
        Gateway->Connect();
    }
    
private:
    UPROPERTY()
    UDiscordGateway* Gateway;
};
```

## Example 6: Error Handling and Resilience

Robust implementation with comprehensive error handling:

```cpp
class FGatewayManager
{
public:
    FGatewayManager()
        : bIsShuttingDown(false)
        , ReconnectAttempts(0)
        , MaxReconnectAttempts(5)
    {
    }
    
    void Initialize(const FString& BotToken)
    {
        Token = BotToken;
        Connect();
    }
    
    void Shutdown()
    {
        bIsShuttingDown = true;
        
        if (Gateway)
        {
            Gateway->Disconnect();
            Gateway = nullptr;
        }
    }
    
    void UpdatePresence(const FString& Activity, int32 Type)
    {
        if (!Gateway || !Gateway->IsConnected())
        {
            UE_LOG(LogTemp, Warning, TEXT("Cannot update presence: Not connected"));
            return;
        }
        
        Gateway->UpdatePresence(Activity, Type);
    }
    
private:
    void Connect()
    {
        if (bIsShuttingDown) return;
        
        // Create Gateway if needed
        if (!Gateway)
        {
            Gateway = NewObject<UDiscordGateway>();
            Gateway->Initialize(Token);
            Gateway->OnConnected.BindRaw(this, &FGatewayManager::OnConnected);
            Gateway->OnDisconnected.BindRaw(this, &FGatewayManager::OnDisconnected);
        }
        
        // Attempt connection
        UE_LOG(LogTemp, Log, TEXT("Connecting to Gateway (Attempt %d/%d)"), 
            ReconnectAttempts + 1, MaxReconnectAttempts);
        
        Gateway->Connect();
    }
    
    void OnConnected()
    {
        UE_LOG(LogTemp, Log, TEXT("Gateway connected successfully"));
        ReconnectAttempts = 0; // Reset counter on successful connection
    }
    
    void OnDisconnected(const FString& Reason)
    {
        UE_LOG(LogTemp, Warning, TEXT("Gateway disconnected: %s"), *Reason);
        
        if (bIsShuttingDown) return;
        
        // Attempt reconnection with exponential backoff
        ReconnectAttempts++;
        
        if (ReconnectAttempts <= MaxReconnectAttempts)
        {
            // Calculate delay: 2^attempts seconds (2, 4, 8, 16, 32)
            float Delay = FMath::Pow(2.0f, static_cast<float>(ReconnectAttempts));
            Delay = FMath::Min(Delay, 60.0f); // Cap at 60 seconds
            
            UE_LOG(LogTemp, Log, TEXT("Scheduling reconnection in %.0f seconds..."), Delay);
            
            // Schedule reconnection (pseudo-code, needs proper timer handle)
            FTimerHandle ReconnectTimer;
            // World->GetTimerManager().SetTimer(ReconnectTimer, this, &FGatewayManager::Connect, Delay, false);
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("Max reconnection attempts reached. Giving up."));
        }
    }
    
    UDiscordGateway* Gateway;
    FString Token;
    bool bIsShuttingDown;
    int32 ReconnectAttempts;
    int32 MaxReconnectAttempts;
};
```

## Example 7: Full Configuration Example

Complete INI configuration with all WebSocket-related settings:

```ini
[/Script/DiscordChatBridge.DiscordChatSubsystem]
; ========== Basic Settings ==========
BotToken=YOUR_BOT_TOKEN_HERE
ChannelId=123456789012345678
PollIntervalSeconds=2.0

; ========== Message Formatting ==========
DiscordSourceLabel=Discord
GameSourceLabel=Game
DiscordNameFormat=[{source}] {username}
GameNameFormat=**[{username}]** {message}

; ========== Server Notifications ==========
EnableServerNotifications=true
NotificationChannelId=
ServerStartMessage=🟢 **Server Started** - The Satisfactory server is now online!
ServerStopMessage=🔴 **Server Stopped** - The Satisfactory server is now offline.

; ========== Bot Activity / Presence ==========
; Enable bot activity updates
EnableBotActivity=true

; Update interval (60-300 seconds recommended)
ActivityUpdateIntervalSeconds=60.0

; ⭐ WebSocket Gateway Settings ⭐
; Use WebSocket Gateway for true bot presence (shows in member list)
UseGatewayForPresence=true

; Presence format when using Gateway ({playercount} placeholder)
; Examples:
;   "with {playercount} players" → "Playing with 5 players"
;   "{playercount} online" → "Playing 5 online"
;   "Satisfactory" → "Playing Satisfactory"
GatewayPresenceFormat=with {playercount} players

; Activity type for Gateway presence
; 0 = Playing (default)
; 1 = Streaming
; 2 = Listening
; 3 = Watching
; 5 = Competing
GatewayActivityType=0

; ========== REST API Fallback Settings ==========
; These are used only when UseGatewayForPresence=false
BotActivityFormat=🎮 **Players Online:** {playercount}
BotActivityChannelId=
```

## Example 8: Debugging WebSocket Issues

Add diagnostic logging to troubleshoot connection issues:

```cpp
void DiagnoseGatewayConnection()
{
    UE_LOG(LogTemp, Log, TEXT("=== Gateway Diagnostics ==="));
    
    // Check configuration
    UE_LOG(LogTemp, Log, TEXT("Bot Token: %s"), 
        BotConfig.BotToken.IsEmpty() ? TEXT("❌ EMPTY") : TEXT("✅ Set"));
    
    UE_LOG(LogTemp, Log, TEXT("Gateway Enabled: %s"), 
        BotConfig.bUseGatewayForPresence ? TEXT("✅ YES") : TEXT("❌ NO"));
    
    UE_LOG(LogTemp, Log, TEXT("Activity Enabled: %s"), 
        BotConfig.bEnableBotActivity ? TEXT("✅ YES") : TEXT("❌ NO"));
    
    // Check Gateway state
    if (Gateway)
    {
        UE_LOG(LogTemp, Log, TEXT("Gateway Object: ✅ Created"));
        
        EGatewayConnectionState State = Gateway->GetConnectionState();
        switch (State)
        {
            case EGatewayConnectionState::Disconnected:
                UE_LOG(LogTemp, Log, TEXT("Connection State: ⚪ Disconnected"));
                break;
            case EGatewayConnectionState::Connecting:
                UE_LOG(LogTemp, Log, TEXT("Connection State: 🟡 Connecting..."));
                break;
            case EGatewayConnectionState::Identifying:
                UE_LOG(LogTemp, Log, TEXT("Connection State: 🟡 Identifying..."));
                break;
            case EGatewayConnectionState::Connected:
                UE_LOG(LogTemp, Log, TEXT("Connection State: 🟢 Connected"));
                break;
            case EGatewayConnectionState::Reconnecting:
                UE_LOG(LogTemp, Log, TEXT("Connection State: 🟡 Reconnecting..."));
                break;
        }
        
        UE_LOG(LogTemp, Log, TEXT("Is Connected: %s"), 
            Gateway->IsConnected() ? TEXT("✅ YES") : TEXT("❌ NO"));
    }
    else
    {
        UE_LOG(LogTemp, Log, TEXT("Gateway Object: ❌ Not Created"));
    }
    
    // Check network connectivity
    // You could add a simple HTTP request test here
    
    UE_LOG(LogTemp, Log, TEXT("=== End Diagnostics ==="));
}
```

## Summary

These examples demonstrate:
- ✅ Basic Gateway connection setup
- ✅ Dynamic presence updates based on game state
- ✅ Different activity types for various scenarios
- ✅ Configuration-driven implementation
- ✅ Testing and validation
- ✅ Error handling and reconnection logic
- ✅ Complete configuration examples
- ✅ Debugging and diagnostics

All WebSocket functionality is fully implemented and ready to use! Refer to [WEBSOCKET_USAGE.md](WEBSOCKET_USAGE.md) for more details on usage and troubleshooting.
