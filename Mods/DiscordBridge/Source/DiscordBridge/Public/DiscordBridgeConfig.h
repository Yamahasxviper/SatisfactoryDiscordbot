// Copyright Coffee Stain Studios. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

/**
 * Configuration for the Discord Bridge mod.
 *
 * Values are read from the plugin's own INI config file:
 *   <ServerRoot>/FactoryGame/Mods/DiscordBridge/Config/DefaultDiscordBridge.ini
 *
 * To enable the bot:
 *   1. Open  Mods/DiscordBridge/Config/DefaultDiscordBridge.ini
 *   2. Set   BotToken  – the token from the Discord Developer Portal (Bot → Token).
 *            Treat this value as a password; do not share it.
 *   3. Set   ChannelId – the snowflake ID of the target text channel.
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

	/**
	 * Loads configuration from the plugin's Config/DefaultDiscordBridge.ini file.
	 * If the file does not exist it is created with default values and those
	 * defaults are returned.
	 */
	static FDiscordBridgeConfig LoadOrCreate();

	/** Returns the absolute path to the INI config file. */
	static FString GetConfigFilePath();
};
