// Copyright SatisfactoryDiscordbot. All Rights Reserved.

#include "SatisfactoryDiscordBotModule.h"
#include "Modules/ModuleManager.h"

DEFINE_LOG_CATEGORY(LogSatisfactoryDiscordBot);

void FSatisfactoryDiscordBotModule::StartupModule()
{
	UE_LOG(LogSatisfactoryDiscordBot, Log, TEXT("SatisfactoryDiscordBot module started"));
}

void FSatisfactoryDiscordBotModule::ShutdownModule()
{
	UE_LOG(LogSatisfactoryDiscordBot, Log, TEXT("SatisfactoryDiscordBot module stopped"));
}

IMPLEMENT_MODULE(FSatisfactoryDiscordBotModule, SatisfactoryDiscordBot)
