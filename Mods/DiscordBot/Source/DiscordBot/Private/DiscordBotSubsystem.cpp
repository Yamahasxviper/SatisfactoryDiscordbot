#include "DiscordBotSubsystem.h"

#include "Engine/GameInstance.h"

DEFINE_LOG_CATEGORY_STATIC(LogDiscordBotSubsystem, Log, All);

void UDiscordBotSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);

    GatewayClient = NewObject<UDiscordGatewayClient>(this);

    if (bAutoConnect && !BotToken.IsEmpty())
    {
        UE_LOG(LogDiscordBotSubsystem, Log, TEXT("Auto-connecting Discord bot with config token."));
        Connect(BotToken);
    }
}

void UDiscordBotSubsystem::Deinitialize()
{
    if (GatewayClient)
    {
        GatewayClient->Disconnect();
    }
    Super::Deinitialize();
}

void UDiscordBotSubsystem::Connect(const FString& InBotToken, int32 InIntents)
{
    if (!GatewayClient)
    {
        GatewayClient = NewObject<UDiscordGatewayClient>(this);
    }

    const FString& TokenToUse = InBotToken.IsEmpty() ? BotToken : InBotToken;

    if (TokenToUse.IsEmpty())
    {
        UE_LOG(LogDiscordBotSubsystem, Error,
            TEXT("No bot token provided. Set BotToken in Config/DiscordBot.ini or pass it to Connect()."));
        return;
    }

    GatewayClient->Connect(TokenToUse, InIntents);
}

void UDiscordBotSubsystem::Disconnect()
{
    if (GatewayClient)
    {
        GatewayClient->Disconnect();
    }
}
