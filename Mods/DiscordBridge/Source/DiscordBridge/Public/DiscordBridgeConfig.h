// Copyright Coffee Stain Studios. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

/**
 * Configuration for the Discord Bridge mod.
 *
 * PRIMARY config (edit this one):
 *   <ServerRoot>/FactoryGame/Mods/DiscordBridge/Config/DefaultDiscordBridge.ini
 * This file ships with the mod and is the one server operators should fill in.
 * It is overwritten by Alpakit/SMM mod updates, but the mod automatically
 * saves a backup before each session so credentials can be restored.
 *
 * BACKUP config (auto-managed, survives mod updates):
 *   <ServerRoot>/FactoryGame/Saved/Config/DiscordBridge.ini
 * Written automatically whenever BotToken and ChannelId are loaded from the
 * primary config.  If the primary file is reset by a mod update, the mod falls
 * back to this backup so the bridge keeps working until the operator copies
 * their credentials back into the primary config.
 *
 * To enable the bot:
 *   1. Open  <ServerRoot>/FactoryGame/Mods/DiscordBridge/Config/DefaultDiscordBridge.ini
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

	/** Display name for this server used as the {ServerName} placeholder in
	 *  GameToDiscordFormat.  Leave empty to omit the server label. */
	FString ServerName;

	/** Format for game → Discord messages. Placeholders: {ServerName}, {PlayerName}, {Message}. */
	FString GameToDiscordFormat{ TEXT("**{PlayerName}**: {Message}") };

	/** Format for Discord → game messages. Placeholders: {Username}, {Message}.
	 *  This string is used as the in-game chat message body.
	 *  Default: just the raw Discord message text. */
	FString DiscordToGameFormat{ TEXT("{Message}") };

	/** Format for the sender name column in the in-game chat for Discord messages.
	 *  Available placeholder: {Username}.
	 *  Default: "[Discord] {Username}". */
	FString DiscordSenderFormat{ TEXT("[Discord] {Username}") };

	// ── Behaviour ─────────────────────────────────────────────────────────────

	/** When true, messages from bot accounts are ignored (prevents echo loops). */
	bool bIgnoreBotMessages{ true };

	// ── Server status messages ────────────────────────────────────────────────

	/** Posted to Discord when the server comes online. Leave empty to disable. */
	FString ServerOnlineMessage{ TEXT(":green_circle: Server is now **online**!") };

	/** Posted to Discord when the server shuts down. Leave empty to disable. */
	FString ServerOfflineMessage{ TEXT(":red_circle: Server is now **offline**.") };

	/**
	 * Loads configuration from the primary mod-folder INI, falling back to the
	 * Saved/Config backup when credentials are missing.  If the primary file does
	 * not exist it is created with default values.  When credentials are
	 * successfully loaded from the primary, an up-to-date backup is written to
	 * the Saved/Config location so they survive the next mod update.
	 */
	static FDiscordBridgeConfig LoadOrCreate();

	/** Returns the absolute path to the primary INI config file (mod folder). */
	static FString GetModConfigFilePath();

	/** Returns the absolute path to the backup INI config file (Saved/Config/). */
	static FString GetBackupConfigFilePath();
};
