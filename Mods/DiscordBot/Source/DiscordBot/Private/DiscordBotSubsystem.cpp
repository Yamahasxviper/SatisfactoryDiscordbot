// Copyright (c) 2024 Yamahasxviper

#include "DiscordBotSubsystem.h"
#include "Engine/World.h"
#include "Misc/ConfigCacheIni.h"
#include "Logging/LogMacros.h"
#include "FGChatManager.h"
#include "EngineUtils.h"

DEFINE_LOG_CATEGORY_STATIC(LogDiscordBotSubsystem, Log, All);

void UDiscordBotSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogDiscordBotSubsystem, Log, TEXT("Discord Bot Subsystem initialized"));
    
    // Load two-way chat configuration
    LoadTwoWayChatConfig();
    
    // Load server notification configuration
    LoadServerNotificationConfig();
    
    // Try to load config and auto-connect if enabled
    bool bEnabled = false;
    if (GConfig)
    {
        GConfig->GetBool(TEXT("DiscordBot"), TEXT("bEnabled"), bEnabled, GGameIni);
    }
    
    if (bEnabled)
    {
        FString BotToken = LoadBotTokenFromConfig();
        if (!BotToken.IsEmpty() && BotToken != TEXT("YOUR_BOT_TOKEN_HERE"))
        {
            // Delay initialization to ensure world is ready
            FTimerHandle InitTimerHandle;
            GetWorld()->GetTimerManager().SetTimerForNextTick([this, BotToken]()
            {
                InitializeAndConnect(BotToken);
                
                // Initialize chat relay if two-way chat is enabled
                if (bTwoWayChatEnabled)
                {
                    if (AFGChatManager* ChatManager = AFGChatManager::Get(GetWorld()))
                    {
                        ChatRelay = NewObject<UDiscordChatRelay>(this);
                        ChatRelay->Initialize(ChatManager);
                    }
                    else
                    {
                        UE_LOG(LogDiscordBotSubsystem, Warning, TEXT("Cannot initialize chat relay: ChatManager not found"));
                    }
                }
                
                // Send server start notification after bot is ready
                // Add a small delay to ensure bot is fully connected
                FTimerHandle NotificationTimerHandle;
                GetWorld()->GetTimerManager().SetTimer(NotificationTimerHandle, [this]()
                {
                    SendServerStartNotification();
                }, 2.0f, false); // 2 second delay
            });
        }
        else
        {
            UE_LOG(LogDiscordBotSubsystem, Warning, TEXT("Discord bot enabled but no valid token configured"));
        }
    }
}

void UDiscordBotSubsystem::Deinitialize()
{
    // Clear player count update timer
    if (GetWorld())
    {
        GetWorld()->GetTimerManager().ClearTimer(PlayerCountUpdateTimerHandle);
    }
    
    // Send server stop notification before disconnecting
    SendServerStopNotification();
    
    // Deinitialize chat relay
    if (ChatRelay)
    {
        ChatRelay->Deinitialize();
        ChatRelay = nullptr;
    }
    
    DisconnectBot();
    Super::Deinitialize();
    
    UE_LOG(LogDiscordBotSubsystem, Log, TEXT("Discord Bot Subsystem deinitialized"));
}

void UDiscordBotSubsystem::InitializeAndConnect(const FString& BotToken)
{
    if (!GetWorld())
    {
        UE_LOG(LogDiscordBotSubsystem, Error, TEXT("Cannot initialize bot: World is null"));
        return;
    }

    // Spawn the gateway client if it doesn't exist
    if (!GatewayClient)
    {
        FActorSpawnParameters SpawnParams;
        SpawnParams.Name = FName(TEXT("DiscordGatewayClient"));
        SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
        
        GatewayClient = GetWorld()->SpawnActor<ADiscordGatewayClient>(ADiscordGatewayClient::StaticClass(), SpawnParams);
        
        if (GatewayClient)
        {
            UE_LOG(LogDiscordBotSubsystem, Log, TEXT("Discord Gateway Client spawned"));
        }
        else
        {
            UE_LOG(LogDiscordBotSubsystem, Error, TEXT("Failed to spawn Discord Gateway Client"));
            return;
        }
    }

    // Initialize and connect
    GatewayClient->InitializeBot(BotToken);
    GatewayClient->Connect();
}

void UDiscordBotSubsystem::DisconnectBot()
{
    if (GatewayClient)
    {
        GatewayClient->Disconnect();
        
        if (GetWorld())
        {
            GetWorld()->DestroyActor(GatewayClient);
        }
        
        GatewayClient = nullptr;
    }
}

void UDiscordBotSubsystem::SendDiscordMessage(const FString& ChannelId, const FString& Message)
{
    if (GatewayClient)
    {
        GatewayClient->SendMessage(ChannelId, Message);
    }
    else
    {
        UE_LOG(LogDiscordBotSubsystem, Warning, TEXT("Cannot send message: Gateway client is null"));
    }
}

bool UDiscordBotSubsystem::IsBotConnected() const
{
    return GatewayClient && GatewayClient->IsConnected();
}

FString UDiscordBotSubsystem::LoadBotTokenFromConfig()
{
    FString BotToken;
    
    if (GConfig)
    {
        GConfig->GetString(TEXT("DiscordBot"), TEXT("BotToken"), BotToken, GGameIni);
    }
    
    return BotToken;
}

void UDiscordBotSubsystem::LoadTwoWayChatConfig()
{
    bTwoWayChatEnabled = false;
    ChatChannelIds.Empty();
    DiscordSenderFormat = TEXT("[Discord] {username}");
    GameSenderFormat = TEXT("{playername}");
    
    if (GConfig)
    {
        // Load two-way chat enabled flag
        GConfig->GetBool(TEXT("DiscordBot"), TEXT("bEnableTwoWayChat"), bTwoWayChatEnabled, GGameIni);
        
        // Load Discord channel IDs
        // Support both comma-separated format (ChatChannelId=123,456,789) and array format (+ChatChannelId=123)
        FString CommaSeparatedChannels;
        if (GConfig->GetString(TEXT("DiscordBot"), TEXT("ChatChannelId"), CommaSeparatedChannels, GGameIni))
        {
            // Parse comma-separated channel IDs
            TArray<FString> ParsedChannels;
            CommaSeparatedChannels.ParseIntoArray(ParsedChannels, TEXT(","), true);
            
            for (FString& ChannelId : ParsedChannels)
            {
                // Trim whitespace from each channel ID
                ChannelId.TrimStartAndEndInline();
                if (!ChannelId.IsEmpty())
                {
                    ChatChannelIds.Add(ChannelId);
                }
            }
        }
        
        // Also try loading array format for backward compatibility
        TArray<FString> ArrayChannels;
        GConfig->GetArray(TEXT("DiscordBot"), TEXT("ChatChannelId"), ArrayChannels, GGameIni);
        
        // Add array channels if not already present (avoid duplicates)
        for (const FString& ChannelId : ArrayChannels)
        {
            if (!ChatChannelIds.Contains(ChannelId) && !ChannelId.IsEmpty())
            {
                ChatChannelIds.Add(ChannelId);
            }
        }
        
        // Load sender format strings
        GConfig->GetString(TEXT("DiscordBot"), TEXT("DiscordSenderFormat"), DiscordSenderFormat, GGameIni);
        GConfig->GetString(TEXT("DiscordBot"), TEXT("GameSenderFormat"), GameSenderFormat, GGameIni);
        
        if (bTwoWayChatEnabled)
        {
            UE_LOG(LogDiscordBotSubsystem, Log, TEXT("Two-way chat enabled with %d channel(s)"), ChatChannelIds.Num());
            for (const FString& ChannelId : ChatChannelIds)
            {
                UE_LOG(LogDiscordBotSubsystem, Log, TEXT("  - Channel ID: %s"), *ChannelId);
            }
        }
    }
}

void UDiscordBotSubsystem::OnDiscordMessageReceived(const FString& ChannelId, const FString& Username, const FString& Message)
{
    if (!bTwoWayChatEnabled)
    {
        return;
    }
    
    // Check if this channel is in our configured list
    if (!ChatChannelIds.Contains(ChannelId))
    {
        return;
    }
    
    // Format the sender name
    FString FormattedSender = FormatDiscordSender(Username);
    
    // Get the chat manager and broadcast the message
    if (UWorld* World = GetWorld())
    {
        AFGChatManager* ChatManager = AFGChatManager::Get(World);
        if (ChatManager)
        {
            FChatMessageStruct ChatMessage;
            ChatMessage.MessageType = EFGChatMessageType::CMT_CustomMessage;
            ChatMessage.MessageSender = FText::FromString(FormattedSender);
            ChatMessage.MessageText = FText::FromString(Message);
            ChatMessage.MessageSenderColor = FLinearColor(0.4f, 0.6f, 1.0f); // Light blue for Discord messages
            
            ChatManager->BroadcastChatMessage(ChatMessage);
            
            UE_LOG(LogDiscordBotSubsystem, Log, TEXT("Discord message relayed to game: [%s] %s"), *FormattedSender, *Message);
        }
        else
        {
            UE_LOG(LogDiscordBotSubsystem, Warning, TEXT("Cannot relay Discord message: ChatManager not found"));
        }
    }
}

void UDiscordBotSubsystem::OnGameChatMessage(const FString& PlayerName, const FString& Message)
{
    if (!bTwoWayChatEnabled || !IsBotConnected())
    {
        return;
    }
    
    // Format the message for Discord
    FString FormattedSender = FormatGameSender(PlayerName);
    FString DiscordMessage = FString::Printf(TEXT("**%s**: %s"), *FormattedSender, *Message);
    
    // Send to all configured channels
    for (const FString& ChannelId : ChatChannelIds)
    {
        SendDiscordMessage(ChannelId, DiscordMessage);
    }
    
    UE_LOG(LogDiscordBotSubsystem, Log, TEXT("Game message relayed to Discord: %s"), *DiscordMessage);
}

void UDiscordBotSubsystem::LoadServerNotificationConfig()
{
    bServerNotificationsEnabled = true;
    NotificationChannelId.Empty();
    ServerStartMessage = TEXT("🟢 Satisfactory Server is now ONLINE!");
    ServerStopMessage = TEXT("🔴 Satisfactory Server is now OFFLINE!");
    BotPresenceMessage = TEXT("Satisfactory Server");
    bShowPlayerCount = true;
    PlayerCountUpdateInterval = 30.0f; // Default to 30 seconds
    
    if (GConfig)
    {
        // Load server notification enabled flag
        GConfig->GetBool(TEXT("DiscordBot"), TEXT("bEnableServerNotifications"), bServerNotificationsEnabled, GGameIni);
        
        // Load notification channel ID
        GConfig->GetString(TEXT("DiscordBot"), TEXT("NotificationChannelId"), NotificationChannelId, GGameIni);
        
        // Load custom messages
        FString CustomStartMessage;
        if (GConfig->GetString(TEXT("DiscordBot"), TEXT("ServerStartMessage"), CustomStartMessage, GGameIni))
        {
            if (!CustomStartMessage.IsEmpty())
            {
                ServerStartMessage = CustomStartMessage;
            }
        }
        
        FString CustomStopMessage;
        if (GConfig->GetString(TEXT("DiscordBot"), TEXT("ServerStopMessage"), CustomStopMessage, GGameIni))
        {
            if (!CustomStopMessage.IsEmpty())
            {
                ServerStopMessage = CustomStopMessage;
            }
        }
        
        // Load bot presence message
        FString CustomPresenceMessage;
        if (GConfig->GetString(TEXT("DiscordBot"), TEXT("BotPresenceMessage"), CustomPresenceMessage, GGameIni))
        {
            if (!CustomPresenceMessage.IsEmpty())
            {
                BotPresenceMessage = CustomPresenceMessage;
            }
        }
        
        // Load player count settings
        GConfig->GetBool(TEXT("DiscordBot"), TEXT("bShowPlayerCount"), bShowPlayerCount, GGameIni);
        GConfig->GetFloat(TEXT("DiscordBot"), TEXT("PlayerCountUpdateInterval"), PlayerCountUpdateInterval, GGameIni);
        
        if (bServerNotificationsEnabled)
        {
            UE_LOG(LogDiscordBotSubsystem, Log, TEXT("Server notifications enabled"));
            if (!NotificationChannelId.IsEmpty() && NotificationChannelId != TEXT("YOUR_NOTIFICATION_CHANNEL_ID_HERE"))
            {
                UE_LOG(LogDiscordBotSubsystem, Log, TEXT("  - Notification Channel ID: %s"), *NotificationChannelId);
            }
            else
            {
                UE_LOG(LogDiscordBotSubsystem, Warning, TEXT("  - No valid notification channel ID configured"));
            }
            
            if (bShowPlayerCount)
            {
                UE_LOG(LogDiscordBotSubsystem, Log, TEXT("  - Player count display enabled (update interval: %.1fs)"), PlayerCountUpdateInterval);
            }
        }
    }
}

void UDiscordBotSubsystem::SendServerStartNotification()
{
    if (!bServerNotificationsEnabled)
    {
        return;
    }
    
    if (NotificationChannelId.IsEmpty() || NotificationChannelId == TEXT("YOUR_NOTIFICATION_CHANNEL_ID_HERE"))
    {
        UE_LOG(LogDiscordBotSubsystem, Warning, TEXT("Cannot send server start notification: No valid channel ID configured"));
        return;
    }
    
    if (!IsBotConnected())
    {
        UE_LOG(LogDiscordBotSubsystem, Warning, TEXT("Cannot send server start notification: Bot not connected"));
        return;
    }
    
    SendDiscordMessage(NotificationChannelId, ServerStartMessage);
    UE_LOG(LogDiscordBotSubsystem, Log, TEXT("Server start notification sent: %s"), *ServerStartMessage);
    
    // Update bot presence/status with initial player count
    UpdateBotPresenceWithPlayerCount();
    
    // Start periodic player count updates if enabled
    if (bShowPlayerCount && GetWorld())
    {
        GetWorld()->GetTimerManager().SetTimer(
            PlayerCountUpdateTimerHandle,
            this,
            &UDiscordBotSubsystem::UpdateBotPresenceWithPlayerCount,
            PlayerCountUpdateInterval,
            true // Loop
        );
        UE_LOG(LogDiscordBotSubsystem, Log, TEXT("Player count update timer started (interval: %.1fs)"), PlayerCountUpdateInterval);
    }
}

void UDiscordBotSubsystem::SendServerStopNotification()
{
    if (!bServerNotificationsEnabled)
    {
        return;
    }
    
    if (NotificationChannelId.IsEmpty() || NotificationChannelId == TEXT("YOUR_NOTIFICATION_CHANNEL_ID_HERE"))
    {
        return;
    }
    
    if (!IsBotConnected())
    {
        return;
    }
    
    SendDiscordMessage(NotificationChannelId, ServerStopMessage);
    UE_LOG(LogDiscordBotSubsystem, Log, TEXT("Server stop notification sent: %s"), *ServerStopMessage);
}

int32 UDiscordBotSubsystem::GetCurrentPlayerCount() const
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return 0;
    }
    
    AGameStateBase* GameState = World->GetGameState();
    if (!GameState)
    {
        return 0;
    }
    
    // Count players in the PlayerArray
    return GameState->PlayerArray.Num();
}

void UDiscordBotSubsystem::UpdateBotPresenceWithPlayerCount()
{
    if (!IsBotConnected() || !GatewayClient)
    {
        return;
    }
    
    FString PresenceMessage = BotPresenceMessage;
    
    // If player count is enabled, append the count to the presence message
    if (bShowPlayerCount)
    {
        int32 PlayerCount = GetCurrentPlayerCount();
        PresenceMessage = FString::Printf(TEXT("%s (%d player%s)"), 
            *BotPresenceMessage, 
            PlayerCount,
            PlayerCount == 1 ? TEXT("") : TEXT("s"));
    }
    
    GatewayClient->UpdatePresence(PresenceMessage);
    UE_LOG(LogDiscordBotSubsystem, Verbose, TEXT("Bot presence updated: %s"), *PresenceMessage);
}


FString UDiscordBotSubsystem::FormatDiscordSender(const FString& Username) const
{
    FString Formatted = DiscordSenderFormat;
    Formatted = Formatted.Replace(TEXT("{username}"), *Username);
    return Formatted;
}

FString UDiscordBotSubsystem::FormatGameSender(const FString& PlayerName) const
{
    FString Formatted = GameSenderFormat;
    Formatted = Formatted.Replace(TEXT("{playername}"), *PlayerName);
    return Formatted;
}
