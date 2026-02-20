// Copyright Epic Games, Inc. All Rights Reserved.

#include "DiscordBotConfig.h"

UDiscordBotConfig::UDiscordBotConfig()
{
	CategoryName             = TEXT("Mods");
	SectionName              = TEXT("DiscordBot");

	BotToken                 = TEXT("");
	ChannelId                = TEXT("");
	InGameDisplayName        = TEXT("Discord");
	PollingIntervalSeconds   = 5.0f;
}

const UDiscordBotConfig* UDiscordBotConfig::Get()
{
	return GetDefault<UDiscordBotConfig>();
}
