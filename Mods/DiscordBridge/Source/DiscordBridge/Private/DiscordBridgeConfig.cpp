// Copyright Coffee Stain Studios. All Rights Reserved.

#include "DiscordBridgeConfig.h"

UDiscordBridgeConfig::UDiscordBridgeConfig()
{
	GameToDiscordFormat    = TEXT("**{PlayerName}**: {Message}");
	DiscordToGameFormat    = TEXT("[Discord] {Username}: {Message}");
	bIgnoreBotMessages     = true;
	ServerOnlineMessage    = TEXT(":green_circle: Server is now **online**!");
	ServerOfflineMessage   = TEXT(":red_circle: Server is now **offline**.");
}
