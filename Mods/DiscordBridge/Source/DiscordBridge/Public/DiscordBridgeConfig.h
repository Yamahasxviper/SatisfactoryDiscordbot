// Copyright Coffee Stain Studios. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

/**
 * Configuration for the Discord Bridge mod.
 *
 * Values are read from and written to
 *   <ServerDir>/FactoryGame/Configs/DiscordBridge.cfg   (JSON format)
 *
 * The file is created automatically with defaults on the first server start.
 * To enable the bot:
 *   1. Open  FactoryGame/Configs/DiscordBridge.cfg
 *   2. Set   "BotToken"  – the token from the Discord Developer Portal (Bot → Token).
 *            Treat this value as a password; do not share it.
 *   3. Set   "ChannelId" – the snowflake ID of the target text channel.
 *            Enable Developer Mode in Discord, right-click the channel, "Copy Channel ID".
 *   4. Restart the server.
 *
 * Discord bot requirements
 * ────────────────────────
 *  • Privileged Gateway Intents that must be enabled in the Discord Developer
 *    Portal (Bot → Privileged Gateway Intents):
 *      – Presence Intent        (GUILD_PRESENCES)
 *      – Server Members Intent  (GUILD_MEMBERS)
 *      – Message Content Intent (MESSAGE_CONTENT)
 *  • The bot must be invited to your server with at minimum the
 *    "Send Messages" and "Read Message History" permissions in the target channel.
 */
struct DISCORDBRIDGE_API FDiscordBridgeConfig
{
	// ── Connection ────────────────────────────────────────────────────────────

	/** Discord bot token (Bot → Token in the Developer Portal). Treat as a password. */
	FString BotToken;

	/** Snowflake ID of the Discord text channel to bridge with in-game chat. */
	FString ChannelId;

	// ── Message formats ───────────────────────────────────────────────────────

	/** Format for game → Discord messages. Placeholders: {PlayerName}, {Message}. */
	FString GameToDiscordFormat{ TEXT("**{PlayerName}**: {Message}") };

	/** Format for Discord → game messages. Placeholders: {Username}, {Message}. */
	FString DiscordToGameFormat{ TEXT("[Discord] {Username}: {Message}") };

	// ── Behaviour ─────────────────────────────────────────────────────────────

	/** When true, messages from bot accounts are ignored (prevents echo loops). */
	bool bIgnoreBotMessages{ true };

	// ── Server status messages ────────────────────────────────────────────────

	/** Posted to Discord when the server comes online. Leave empty to disable. */
	FString ServerOnlineMessage{ TEXT(":green_circle: Server is now **online**!") };

	/** Posted to Discord when the server shuts down. Leave empty to disable. */
	FString ServerOfflineMessage{ TEXT(":red_circle: Server is now **offline**.") };

	// ── Log viewer command ────────────────────────────────────────────────────

	/**
	 * Discord command prefix that triggers the server log viewer.
	 * When a user types this exact text in the bridged channel the bot replies
	 * with the last LogLineCount lines of the Unreal Engine server log.
	 * Leave empty to disable the command.
	 */
	FString LogCommandPrefix{ TEXT("!logs") };

	/**
	 * Number of log lines returned by the !logs command.
	 * The output is split into multiple Discord messages if needed to stay
	 * within the 2 000-character message limit.
	 */
	int32 LogLineCount{ 50 };

	/**
	 * Loads configuration from <ProjectDir>/Configs/DiscordBridge.cfg.
	 * If the file does not exist it is created with default values and those
	 * defaults are returned.  Any missing keys in an existing file are added
	 * on the next save.
	 */
	static FDiscordBridgeConfig LoadOrCreate();

	/** Returns the absolute path to the JSON config file. */
	static FString GetConfigFilePath();
};
