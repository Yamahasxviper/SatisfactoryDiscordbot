// Copyright Coffee Stain Studios. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

/**
 * Configuration for the Discord Bridge mod.
 *
 * The live configuration is read from (and auto-created in):
 *   <ServerRoot>/FactoryGame/Saved/Config/DiscordBridge.ini
 *
 * This location is NEVER touched by Alpakit mod updates, so BotToken and
 * ChannelId survive every upgrade.  An example with all settings and their
 * defaults is shipped with the mod at:
 *   <ServerRoot>/FactoryGame/Mods/DiscordBridge/Config/DefaultDiscordBridge.ini
 * (that file IS overwritten on updates – do not store credentials there).
 *
 * To enable the bot:
 *   1. Open  <ServerRoot>/FactoryGame/Saved/Config/DiscordBridge.ini
 *      (created automatically on first server start; or copy DefaultDiscordBridge.ini there).
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
	 * How often (in seconds) to poll the Discord REST API for new messages.
	 * Lower values give faster message delivery but use more HTTP requests.
	 * Discord allows up to 5 requests per second; the default of 2 s is safe.
	 * Minimum enforced value: 1.0 s.
	 */
	float PollIntervalSeconds{ 2.0f };

	/**
	 * Loads configuration from Saved/Config/DiscordBridge.ini.
	 * If the file does not exist it is created with default values and those
	 * defaults are returned.
	 */
	static FDiscordBridgeConfig LoadOrCreate();

	/** Returns the absolute path to the live INI config file (in Saved/Config/). */
	static FString GetConfigFilePath();
};
