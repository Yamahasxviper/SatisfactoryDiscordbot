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
