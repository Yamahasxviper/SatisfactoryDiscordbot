// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "DiscordBotSettings.generated.h"

/**
 * Project-wide settings for the Discord chat bridge.
 *
 * Configure in Config/DefaultPLUGIN_NAME.ini, or via
 * Editor > Project Settings > Plugins > PLUGIN_NAME > Discord Bot.
 *
 * The mod connects to a local WebSocket relay service (RelayWebSocketUrl).
 * The relay is responsible for authenticating with Discord using the
 * BotToken and forwarding messages to/from ChannelId.
 *
 * A minimal Python relay example is included alongside this template.
 */
UCLASS(Config = PLUGIN_NAME, DefaultConfig, meta = (DisplayName = "Discord Bot"))
class UDiscordBotSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	UDiscordBotSettings();

	// UDeveloperSettings interface
	virtual FName GetCategoryName() const override { return TEXT("Plugins"); }

	/**
	 * Your Discord bot token.
	 * Sent to the relay service so it can authenticate with the Discord API.
	 * Keep this secret – do not commit it to version control.
	 */
	UPROPERTY(Config, EditAnywhere, Category = "Discord Bot")
	FString BotToken;

	/**
	 * Numeric ID of the Discord text channel to bridge with the in-game chat.
	 * Right-click the channel in Discord (Developer Mode must be enabled) and
	 * select "Copy Channel ID".
	 */
	UPROPERTY(Config, EditAnywhere, Category = "Discord Bot")
	FString ChannelId;

	/**
	 * WebSocket URL of the local relay service that connects to Discord.
	 * The relay must be running on the game-server machine before the world loads.
	 * Default: ws://localhost:8765
	 */
	UPROPERTY(Config, EditAnywhere, Category = "Discord Bot")
	FString RelayWebSocketUrl;

	/** When true, in-game player chat messages are forwarded to the Discord channel. */
	UPROPERTY(Config, EditAnywhere, Category = "Discord Bot")
	bool bGameToDiscord;

	/** When true, Discord channel messages are displayed in the in-game chat. */
	UPROPERTY(Config, EditAnywhere, Category = "Discord Bot")
	bool bDiscordToGame;

	/**
	 * Prefix prepended to Discord usernames when showing their messages in-game.
	 * For example, setting this to "[Discord]" produces "[Discord] Username: text".
	 * Leave empty to show only the username.
	 */
	UPROPERTY(Config, EditAnywhere, Category = "Discord Bot")
	FString DiscordPrefix;
};
