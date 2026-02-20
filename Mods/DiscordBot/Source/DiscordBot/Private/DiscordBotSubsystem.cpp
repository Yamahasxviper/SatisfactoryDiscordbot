// Copyright Yamahasxviper. All Rights Reserved.

#include "DiscordBotSubsystem.h"

DEFINE_LOG_CATEGORY(LogDiscordBotSubsystem);

void UDiscordBotSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    UE_LOG(LogDiscordBotSubsystem, Log, TEXT("DiscordBotSubsystem initialized"));
}

void UDiscordBotSubsystem::Deinitialize()
{
    DisconnectBot();
    Super::Deinitialize();
}

void UDiscordBotSubsystem::InitializeBot(const FString& BotToken, int32 Intents)
{
    if (DiscordWebSocket)
    {
        UE_LOG(LogDiscordBotSubsystem, Warning, TEXT("InitializeBot called while a WebSocket already exists; disconnecting first"));
        DisconnectBot();
    }

    DiscordWebSocket = UDiscordBotWebSocket::Create(this, BotToken, Intents);
    DiscordWebSocket->Connect();

    UE_LOG(LogDiscordBotSubsystem, Log, TEXT("Discord bot connection initiated"));
}

void UDiscordBotSubsystem::DisconnectBot()
{
    if (DiscordWebSocket)
    {
        DiscordWebSocket->Disconnect();
        DiscordWebSocket = nullptr;
    }
}

bool UDiscordBotSubsystem::IsConnected() const
{
    return DiscordWebSocket &&
           DiscordWebSocket->GetConnectionState() == EDiscordWebSocketState::Connected;
}
