// Copyright (c) 2024 Yamahasxviper

#include "DiscordBotSubsystem.h"
#include "Engine/World.h"
#include "Misc/ConfigCacheIni.h"
#include "Logging/LogMacros.h"

DEFINE_LOG_CATEGORY_STATIC(LogDiscordBotSubsystem, Log, All);

void UDiscordBotSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogDiscordBotSubsystem, Log, TEXT("Discord Bot Subsystem initialized"));
    
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
