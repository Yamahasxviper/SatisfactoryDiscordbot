// Copyright (c) 2024 Yamahasxviper

#include "DiscordBotModule.h"
#include "DiscordGatewayClient.h"
#include "Logging/LogMacros.h"

DEFINE_LOG_CATEGORY_STATIC(LogDiscordBot, Log, All);

void FDiscordBotModule::StartupModule()
{
    UE_LOG(LogDiscordBot, Log, TEXT("DiscordBot module starting up"));
}

void FDiscordBotModule::ShutdownModule()
{
    UE_LOG(LogDiscordBot, Log, TEXT("DiscordBot module shutting down"));
}

IMPLEMENT_MODULE(FDiscordBotModule, DiscordBot)
