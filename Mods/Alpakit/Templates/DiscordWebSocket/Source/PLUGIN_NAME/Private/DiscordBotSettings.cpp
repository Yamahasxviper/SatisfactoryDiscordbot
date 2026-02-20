// Copyright Epic Games, Inc. All Rights Reserved.

#include "DiscordBotSettings.h"

UDiscordBotSettings::UDiscordBotSettings()
	: bGameToDiscord(true)
	, bDiscordToGame(true)
	, DiscordPrefix(TEXT("[Discord]"))
{
	// Default relay URL – matches the companion relay script's default port.
	RelayWebSocketUrl = TEXT("ws://localhost:8765");
}
