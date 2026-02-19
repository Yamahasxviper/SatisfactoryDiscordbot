// Copyright (c) 2024 Yamahasxviper

#include "DiscordBotSubsystem.h"
#include "Engine/World.h"
#include "Misc/ConfigCacheIni.h"
#include "Logging/LogMacros.h"
#include "FGChatManager.h"
#include "EngineUtils.h"
#include "GameFramework/GameStateBase.h"
#include "GameFramework/PlayerState.h"
#include "DiscordGatewayClientCustom.h"

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
        // Use explicit config filename for cross-platform compatibility (especially dedicated servers)
        FString ConfigFilename = GConfig->GetConfigFilename(TEXT("Game"));
        GConfig->GetBool(TEXT("DiscordBot"), TEXT("bEnabled"), bEnabled, ConfigFilename);
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
        
        GatewayClient = GetWorld()->SpawnActor<ADiscordGatewayClientCustom>(ADiscordGatewayClientCustom::StaticClass(), SpawnParams);
        
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
        // Use explicit config filename for cross-platform compatibility (especially dedicated servers)
        FString ConfigFilename = GConfig->GetConfigFilename(TEXT("Game"));
        GConfig->GetString(TEXT("DiscordBot"), TEXT("BotToken"), BotToken, ConfigFilename);
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
        // Use explicit config filename for cross-platform compatibility (especially dedicated servers)
        FString ConfigFilename = GConfig->GetConfigFilename(TEXT("Game"));
        
        // Load two-way chat enabled flag
        GConfig->GetBool(TEXT("DiscordBot"), TEXT("bEnableTwoWayChat"), bTwoWayChatEnabled, ConfigFilename);
        
        // Load Discord channel IDs
        // Support both comma-separated format (ChatChannelId=123,456,789) and array format (+ChatChannelId=123)
        FString CommaSeparatedChannels;
        if (GConfig->GetString(TEXT("DiscordBot"), TEXT("ChatChannelId"), CommaSeparatedChannels, ConfigFilename))
        {
            // Parse comma-separated channel IDs
            TArray<FString> ParsedChannels;
            CommaSeparatedChannels.ParseIntoArray(ParsedChannels, TEXT(","), true);
            
            for (FString& ChannelId : ParsedChannels)
            {
                // Trim whitespace from each channel ID
                ChannelId.TrimStartAndEndInline();
                // Skip empty IDs and placeholder values
                if (!ChannelId.IsEmpty() && 
                    ChannelId != TEXT("YOUR_CHANNEL_ID_HERE") &&
                    !ChannelId.StartsWith(TEXT("YOUR_")))
                {
                    ChatChannelIds.Add(ChannelId);
                }
            }
        }
        
        // Also try loading array format for backward compatibility
        TArray<FString> ArrayChannels;
        GConfig->GetArray(TEXT("DiscordBot"), TEXT("ChatChannelId"), ArrayChannels, ConfigFilename);
        
        // Add array channels if not already present (avoid duplicates)
        for (const FString& ChannelId : ArrayChannels)
        {
            // Skip placeholder values
            if (!ChatChannelIds.Contains(ChannelId) && 
                !ChannelId.IsEmpty() && 
                ChannelId != TEXT("YOUR_CHANNEL_ID_HERE") &&
                !ChannelId.StartsWith(TEXT("YOUR_")))
            {
                ChatChannelIds.Add(ChannelId);
            }
        }
        
        // Load sender format strings
        FString LoadedDiscordFormat;
        if (GConfig->GetString(TEXT("DiscordBot"), TEXT("DiscordSenderFormat"), LoadedDiscordFormat, ConfigFilename))
        {
            if (!LoadedDiscordFormat.IsEmpty())
            {
                DiscordSenderFormat = LoadedDiscordFormat;
            }
        }
        
        FString LoadedGameFormat;
        if (GConfig->GetString(TEXT("DiscordBot"), TEXT("GameSenderFormat"), LoadedGameFormat, ConfigFilename))
        {
            if (!LoadedGameFormat.IsEmpty())
            {
                GameSenderFormat = LoadedGameFormat;
            }
        }
        
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
    BotActivityType = 0; // Default to "Playing"
    bShowPlayerCount = true;
    bShowPlayerNames = false;
    MaxPlayerNamesToShow = 10;
    PlayerNamesFormat = TEXT("with {names}");
    bUseCustomPresenceFormat = false;
    CustomPresenceFormat.Empty();
    PlayerCountUpdateInterval = 30.0f; // Default to 30 seconds
    
    if (GConfig)
    {
        // Use explicit config filename for cross-platform compatibility (especially dedicated servers)
        FString ConfigFilename = GConfig->GetConfigFilename(TEXT("Game"));
        
        // Load server notification enabled flag
        GConfig->GetBool(TEXT("DiscordBot"), TEXT("bEnableServerNotifications"), bServerNotificationsEnabled, ConfigFilename);
        
        // Load notification channel ID
        GConfig->GetString(TEXT("DiscordBot"), TEXT("NotificationChannelId"), NotificationChannelId, ConfigFilename);
        
        // Clear notification channel ID if it's a placeholder value
        if (NotificationChannelId == TEXT("YOUR_NOTIFICATION_CHANNEL_ID_HERE") || 
            NotificationChannelId.StartsWith(TEXT("YOUR_")))
        {
            NotificationChannelId.Empty();
        }
        
        // Load custom messages
        FString CustomStartMessage;
        if (GConfig->GetString(TEXT("DiscordBot"), TEXT("ServerStartMessage"), CustomStartMessage, ConfigFilename))
        {
            if (!CustomStartMessage.IsEmpty())
            {
                ServerStartMessage = CustomStartMessage;
            }
        }
        
        FString CustomStopMessage;
        if (GConfig->GetString(TEXT("DiscordBot"), TEXT("ServerStopMessage"), CustomStopMessage, ConfigFilename))
        {
            if (!CustomStopMessage.IsEmpty())
            {
                ServerStopMessage = CustomStopMessage;
            }
        }
        
        // Load bot presence message
        FString CustomPresenceMessage;
        if (GConfig->GetString(TEXT("DiscordBot"), TEXT("BotPresenceMessage"), CustomPresenceMessage, ConfigFilename))
        {
            if (!CustomPresenceMessage.IsEmpty())
            {
                BotPresenceMessage = CustomPresenceMessage;
            }
        }
        
        // Load bot activity type
        FString ActivityTypeStr;
        if (GConfig->GetString(TEXT("DiscordBot"), TEXT("BotActivityType"), ActivityTypeStr, ConfigFilename))
        {
            // Support both string names and numeric values
            ActivityTypeStr = ActivityTypeStr.TrimStartAndEnd().ToLower();
            if (ActivityTypeStr == TEXT("playing") || ActivityTypeStr == TEXT("0"))
            {
                BotActivityType = 0;
            }
            else if (ActivityTypeStr == TEXT("streaming") || ActivityTypeStr == TEXT("1"))
            {
                BotActivityType = 1;
            }
            else if (ActivityTypeStr == TEXT("listening") || ActivityTypeStr == TEXT("listening to") || ActivityTypeStr == TEXT("2"))
            {
                BotActivityType = 2;
            }
            else if (ActivityTypeStr == TEXT("watching") || ActivityTypeStr == TEXT("3"))
            {
                BotActivityType = 3;
            }
            else if (ActivityTypeStr == TEXT("competing") || ActivityTypeStr == TEXT("competing in") || ActivityTypeStr == TEXT("5"))
            {
                BotActivityType = 5;
            }
            else
            {
                // Try parsing as integer
                BotActivityType = FCString::Atoi(*ActivityTypeStr);
                if (BotActivityType < 0 || (BotActivityType > 3 && BotActivityType != 5))
                {
                    UE_LOG(LogDiscordBotSubsystem, Warning, TEXT("Invalid BotActivityType '%s', defaulting to 0 (Playing)"), *ActivityTypeStr);
                    BotActivityType = 0;
                }
            }
        }
        
        // Load player count settings
        GConfig->GetBool(TEXT("DiscordBot"), TEXT("bShowPlayerCount"), bShowPlayerCount, ConfigFilename);
        GConfig->GetFloat(TEXT("DiscordBot"), TEXT("PlayerCountUpdateInterval"), PlayerCountUpdateInterval, ConfigFilename);
        
        // Load player names settings
        GConfig->GetBool(TEXT("DiscordBot"), TEXT("bShowPlayerNames"), bShowPlayerNames, ConfigFilename);
        GConfig->GetInt(TEXT("DiscordBot"), TEXT("MaxPlayerNamesToShow"), MaxPlayerNamesToShow, ConfigFilename);
        
        FString CustomNamesFormat;
        if (GConfig->GetString(TEXT("DiscordBot"), TEXT("PlayerNamesFormat"), CustomNamesFormat, ConfigFilename))
        {
            if (!CustomNamesFormat.IsEmpty())
            {
                PlayerNamesFormat = CustomNamesFormat;
            }
        }
        
        // Load custom presence format
        GConfig->GetBool(TEXT("DiscordBot"), TEXT("bUseCustomPresenceFormat"), bUseCustomPresenceFormat, ConfigFilename);
        FString CustomFormat;
        if (GConfig->GetString(TEXT("DiscordBot"), TEXT("CustomPresenceFormat"), CustomFormat, ConfigFilename))
        {
            if (!CustomFormat.IsEmpty())
            {
                CustomPresenceFormat = CustomFormat;
            }
        }
        
        if (bServerNotificationsEnabled)
        {
            UE_LOG(LogDiscordBotSubsystem, Log, TEXT("Server notifications enabled"));
            if (!NotificationChannelId.IsEmpty())
            {
                UE_LOG(LogDiscordBotSubsystem, Log, TEXT("  - Notification Channel ID: %s"), *NotificationChannelId);
            }
            
            if (bUseCustomPresenceFormat && !CustomPresenceFormat.IsEmpty())
            {
                UE_LOG(LogDiscordBotSubsystem, Log, TEXT("  - Using custom presence format: %s"), *CustomPresenceFormat);
            }
            else if (bShowPlayerCount)
            {
                UE_LOG(LogDiscordBotSubsystem, Log, TEXT("  - Player count display enabled (update interval: %.1fs)"), PlayerCountUpdateInterval);
            }
            
            if (bShowPlayerNames)
            {
                UE_LOG(LogDiscordBotSubsystem, Log, TEXT("  - Player names display enabled (max names: %d)"), MaxPlayerNamesToShow);
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
    
    if (NotificationChannelId.IsEmpty())
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
    
    if (NotificationChannelId.IsEmpty())
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

TArray<FString> UDiscordBotSubsystem::GetCurrentPlayerNames() const
{
    TArray<FString> PlayerNames;
    
    UWorld* World = GetWorld();
    if (!World)
    {
        return PlayerNames;
    }
    
    AGameStateBase* GameState = World->GetGameState();
    if (!GameState)
    {
        return PlayerNames;
    }
    
    // Get player names from PlayerArray
    for (APlayerState* PlayerState : GameState->PlayerArray)
    {
        if (PlayerState && !PlayerState->GetPlayerName().IsEmpty())
        {
            PlayerNames.Add(PlayerState->GetPlayerName());
        }
    }
    
    return PlayerNames;
}

FString UDiscordBotSubsystem::FormatPlayerNames(const TArray<FString>& PlayerNames) const
{
    if (PlayerNames.Num() == 0)
    {
        return TEXT("");
    }
    
    // Determine how many names to show
    int32 NamesToShow = PlayerNames.Num();
    if (MaxPlayerNamesToShow > 0 && NamesToShow > MaxPlayerNamesToShow)
    {
        NamesToShow = MaxPlayerNamesToShow;
    }
    
    // Build the names string
    FString NamesString;
    for (int32 i = 0; i < NamesToShow; i++)
    {
        if (i > 0)
        {
            if (i == NamesToShow - 1 && NamesToShow == PlayerNames.Num())
            {
                // Last name and we're showing all names
                NamesString += TEXT(" and ");
            }
            else
            {
                NamesString += TEXT(", ");
            }
        }
        NamesString += PlayerNames[i];
    }
    
    // Add "and X more" if we're not showing all names
    if (MaxPlayerNamesToShow > 0 && PlayerNames.Num() > MaxPlayerNamesToShow)
    {
        int32 RemainingCount = PlayerNames.Num() - MaxPlayerNamesToShow;
        NamesString += FString::Printf(TEXT(" and %d more"), RemainingCount);
    }
    
    // Apply the format string
    FString Result = PlayerNamesFormat;
    Result = Result.Replace(TEXT("{names}"), *NamesString);
    Result = Result.Replace(TEXT("{count}"), *FString::FromInt(PlayerNames.Num()));
    
    return Result;
}

FString UDiscordBotSubsystem::BuildPresenceFromCustomFormat() const
{
    FString Result = CustomPresenceFormat;
    
    // Replace {message} or {servername} with the base bot presence message
    Result = Result.Replace(TEXT("{message}"), *BotPresenceMessage);
    Result = Result.Replace(TEXT("{servername}"), *BotPresenceMessage);
    
    // Get current player count
    int32 PlayerCount = GetCurrentPlayerCount();
    Result = Result.Replace(TEXT("{playercount}"), *FString::FromInt(PlayerCount));
    Result = Result.Replace(TEXT("{count}"), *FString::FromInt(PlayerCount));
    
    // Get and format player names
    TArray<FString> PlayerNames = GetCurrentPlayerNames();
    if (PlayerNames.Num() > 0)
    {
        // Build formatted names string
        int32 NamesToShow = PlayerNames.Num();
        if (MaxPlayerNamesToShow > 0 && NamesToShow > MaxPlayerNamesToShow)
        {
            NamesToShow = MaxPlayerNamesToShow;
        }
        
        FString NamesString;
        for (int32 i = 0; i < NamesToShow; i++)
        {
            if (i > 0)
            {
                if (i == NamesToShow - 1 && NamesToShow == PlayerNames.Num())
                {
                    NamesString += TEXT(" and ");
                }
                else
                {
                    NamesString += TEXT(", ");
                }
            }
            NamesString += PlayerNames[i];
        }
        
        if (MaxPlayerNamesToShow > 0 && PlayerNames.Num() > MaxPlayerNamesToShow)
        {
            int32 RemainingCount = PlayerNames.Num() - MaxPlayerNamesToShow;
            NamesString += FString::Printf(TEXT(" and %d more"), RemainingCount);
        }
        
        Result = Result.Replace(TEXT("{names}"), *NamesString);
        Result = Result.Replace(TEXT("{playernames}"), *NamesString);
    }
    else
    {
        // Replace with empty string if no players
        Result = Result.Replace(TEXT("{names}"), TEXT(""));
        Result = Result.Replace(TEXT("{playernames}"), TEXT(""));
    }
    
    // Handle player/players grammar
    FString PlayerWord = (PlayerCount == 1) ? TEXT("player") : TEXT("players");
    Result = Result.Replace(TEXT("{player_s}"), *PlayerWord);
    
    return Result;
}

void UDiscordBotSubsystem::UpdateBotPresenceWithPlayerCount()
{
    if (!IsBotConnected() || !GatewayClient)
    {
        return;
    }
    
    FString PresenceMessage;
    
    // Check if custom presence format is enabled
    if (bUseCustomPresenceFormat && !CustomPresenceFormat.IsEmpty())
    {
        PresenceMessage = BuildPresenceFromCustomFormat();
    }
    // If player names are enabled, show names instead of count
    else if (bShowPlayerNames)
    {
        TArray<FString> PlayerNames = GetCurrentPlayerNames();
        if (PlayerNames.Num() > 0)
        {
            FString FormattedNames = FormatPlayerNames(PlayerNames);
            PresenceMessage = FString::Printf(TEXT("%s %s"), *BotPresenceMessage, *FormattedNames);
        }
        else
        {
            // No players online
            PresenceMessage = BotPresenceMessage;
        }
    }
    // Otherwise, if player count is enabled, append the count to the presence message
    else if (bShowPlayerCount)
    {
        int32 PlayerCount = GetCurrentPlayerCount();
        PresenceMessage = FString::Printf(TEXT("%s (%d player%s)"), 
            *BotPresenceMessage, 
            PlayerCount,
            PlayerCount == 1 ? TEXT("") : TEXT("s"));
    }
    else
    {
        // No special formatting, just use the base message
        PresenceMessage = BotPresenceMessage;
    }
    
    GatewayClient->UpdatePresence(PresenceMessage, BotActivityType);
    UE_LOG(LogDiscordBotSubsystem, Verbose, TEXT("Bot presence updated: %s (Type: %d)"), *PresenceMessage, BotActivityType);
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
