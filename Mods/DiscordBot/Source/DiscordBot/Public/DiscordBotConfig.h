// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "DiscordBotConfig.generated.h"

/**
 * Configuration for the Discord Bot mod.
 *
 * Values are stored in <GameDir>/Configs/DiscordBot.ini and can be edited
 * directly in that file or through Unreal Editor under
 * Project Settings > Plugins > Discord Bot Settings.
 *
 * Required settings:
 *   BotToken  – Discord bot token from the Developer Portal.
 *   ChannelId – Numeric ID of the Discord text channel to bridge.
 */
UCLASS(Config = DiscordBot, DefaultConfig, meta = (DisplayName = "Discord Bot Settings"))
class DISCORDBOT_API UDiscordBotConfig : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	UDiscordBotConfig();

	/**
	 * Discord bot token obtained from the Discord Developer Portal.
	 * Never share or commit this value to source control.
	 */
	UPROPERTY(Config, EditAnywhere, Category = "Discord",
		meta = (Tooltip = "Bot token from the Discord Developer Portal."))
	FString BotToken;

	/** Numeric ID of the Discord text channel to bridge with in-game chat. */
	UPROPERTY(Config, EditAnywhere, Category = "Discord",
		meta = (Tooltip = "Numeric ID of the Discord channel to relay messages to/from."))
	FString ChannelId;

	/**
	 * Name shown in Satisfactory chat when a Discord message is relayed into the game.
	 * Defaults to "Discord".  The full sender tag will look like:
	 *   "Discord (username)" where username is the Discord author's handle.
	 */
	UPROPERTY(Config, EditAnywhere, Category = "Discord",
		meta = (Tooltip = "Sender label shown in-game for incoming Discord messages."))
	FString InGameDisplayName;

	/**
	 * How often (in seconds) the mod polls Discord for new messages.
	 * Lower values give faster Discord-to-game delivery but increase API traffic.
	 * Minimum enforced at runtime: 2 seconds.
	 */
	UPROPERTY(Config, EditAnywhere, Category = "Discord",
		meta = (ClampMin = "2.0", Tooltip = "Seconds between Discord message polls."))
	float PollingIntervalSeconds;

	/** Convenience accessor for the CDO (config singleton). */
	static const UDiscordBotConfig* Get();
};
