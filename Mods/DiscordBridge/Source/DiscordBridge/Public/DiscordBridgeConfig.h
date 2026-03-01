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

	/** Display name for this server used as the %ServerName% placeholder in
	 *  GameToDiscordFormat.  Leave empty to omit the server label. */
	FString ServerName;

	/** Format for game → Discord messages. Placeholders: %ServerName%, %PlayerName%, %Message%. */
	FString GameToDiscordFormat{ TEXT("**%PlayerName%**: %Message%") };

	/** Format for Discord → game messages.  This single string controls the complete
	 *  line of text shown in the Satisfactory in-game chat whenever a Discord message
	 *  is relayed.  Available placeholders:
	 *    %Username%   – the Discord display name of the sender
	 *    %PlayerName% – alias for %Username%
	 *    %Message%    – the Discord message text
	 *  Default: "[Discord] %Username%: %Message%" */
	FString DiscordToGameFormat{ TEXT("[Discord] %Username%: %Message%") };

	// ── Behaviour ─────────────────────────────────────────────────────────────

	/** When true, messages from bot accounts are ignored (prevents echo loops). */
	bool bIgnoreBotMessages{ true };

	// ── Server status messages ────────────────────────────────────────────────

	/** Posted to Discord when the server comes online. Leave empty to disable. */
	FString ServerOnlineMessage{ TEXT(":green_circle: Server is now **online**!") };

	/** Posted to Discord when the server shuts down. Leave empty to disable. */
	FString ServerOfflineMessage{ TEXT(":red_circle: Server is now **offline**.") };

	// ── Player count presence ─────────────────────────────────────────────────

	// ── Whitelist ─────────────────────────────────────────────────────────────

	/**
	 * When true, the whitelist is enabled on every server start, overriding any
	 * runtime change made via !whitelist on / !whitelist off Discord commands.
	 * Default: false (all players can join).
	 */
	bool bWhitelistEnabled{ false };

	/**
	 * Snowflake ID of the Discord role whose members are allowed to run
	 * !whitelist management commands.
	 * Leave empty (or unset) to disable !whitelist commands entirely – no
	 * Discord user will be able to run them until a role ID is provided.
	 *
	 * IMPORTANT: holding this role does NOT grant automatic access to the game
	 * server.  Discord members with this role are still subject to the whitelist
	 * and ban checks when they join; they must be added to the whitelist separately.
	 *
	 * To get the role ID: Discord Settings → Advanced → Developer Mode, then
	 * right-click the role in Server Settings → Roles and choose Copy Role ID.
	 */
	FString WhitelistCommandRoleId;

	/**
	 * Snowflake ID of the Discord role whose members are allowed to run
	 * !ban management commands.
	 * Leave empty (or unset) to disable !ban commands entirely – no Discord
	 * user will be able to run them until a role ID is provided.
	 *
	 * This role is also the one granted or revoked by the
	 * `!ban role add <user_id>` and `!ban role remove <user_id>` commands,
	 * so holders can promote or demote other Discord members from within Discord
	 * without needing server-level role management access.
	 * The bot must have the **Manage Roles** permission for those commands to work.
	 *
	 * IMPORTANT: holding this role does NOT grant automatic access to the game
	 * server.  Discord members with this role are still subject to the whitelist
	 * and ban checks when they join; they must be added to the whitelist separately.
	 *
	 * To get the role ID: Discord Settings → Advanced → Developer Mode, then
	 * right-click the role in Server Settings → Roles and choose Copy Role ID.
	 */
	FString BanCommandRoleId;

	/**
	 * Prefix that triggers whitelist management commands from Discord.
	 * Set to an empty string to disable Discord-based whitelist management.
	 * Default: "!whitelist"
	 *
	 * Supported commands (type in the bridged Discord channel):
	 *   !whitelist on                       – enable the whitelist
	 *   !whitelist off                      – disable the whitelist
	 *   !whitelist add <name>               – add a player
	 *   !whitelist remove <name>            – remove a player
	 *   !whitelist list                     – list all whitelisted players
	 *   !whitelist status                   – show current enabled/disabled state
	 *   !whitelist role add <discord_id>    – grant WhitelistRoleId to a Discord user
	 *   !whitelist role remove <discord_id> – revoke WhitelistRoleId from a Discord user
	 */
	FString WhitelistCommandPrefix{ TEXT("!whitelist") };

	/**
	 * Snowflake ID of the Discord role used to identify whitelisted members.
	 * Leave empty to disable Discord role integration.
	 *
	 * When set:
	 *  • Messages received on WhitelistChannelId are relayed to the game only
	 *    when the sender holds this role.
	 *  • The `!whitelist role add/remove <user_id>` commands assign or revoke
	 *    this role via the Discord REST API (bot must have Manage Roles permission).
	 *  • At bot startup the member list is fetched from Discord and cached.
	 *    Any player whose in-game name matches a cached Discord display name
	 *    (server nickname, global name, or username) is allowed through the
	 *    whitelist check without needing an explicit entry in ServerWhitelist.json.
	 *    The cache is kept up to date by GUILD_MEMBER_ADD/UPDATE/REMOVE events.
	 */
	FString WhitelistRoleId;

	/**
	 * Snowflake ID of a dedicated Discord channel for whitelisted members.
	 * Leave empty to disable the whitelist-only channel.
	 *
	 * When set:
	 *  • In-game messages from players on the server whitelist are posted to
	 *    this channel in addition to the main ChannelId.
	 *  • Discord messages sent to this channel are relayed to the game only
	 *    when the sender holds WhitelistRoleId (if that field is non-empty).
	 */
	FString WhitelistChannelId;

	/**
	 * Snowflake ID of a dedicated Discord channel for ban management.
	 * Leave empty to disable the ban-only channel.
	 *
	 * When set:
	 *  • !ban commands issued from this channel are accepted (sender must still
	 *    hold BanCommandRoleId).  Command responses are sent back to this channel.
	 *  • Ban-kick notifications are also posted here (in addition to the main
	 *    ChannelId), giving admins a focused audit log of bans.
	 *
	 * Get the channel ID the same way as ChannelId (right-click the channel in
	 * Discord with Developer Mode enabled → Copy Channel ID).
	 *
	 * Example: BanChannelId=567890123456789012
	 */
	FString BanChannelId;

	/**
	 * Message posted to the main Discord channel whenever the whitelist kicks
	 * a player who tried to join.  Leave empty to disable the notification.
	 *
	 * Available placeholder:
	 *   %PlayerName%  – in-game name of the player who was kicked.
	 *
	 * Example:
	 *   WhitelistKickDiscordMessage=:boot: **%PlayerName%** is not whitelisted and was kicked.
	 */
	FString WhitelistKickDiscordMessage{
		TEXT(":boot: **%PlayerName%** tried to join but is not on the whitelist and was kicked.")
	};

	/**
	 * Reason shown in-game to the player when they are kicked because they are
	 * not on the whitelist.  This is the text the player sees in the "Disconnected"
	 * screen.
	 * Default: "You are not on this server's whitelist. Contact the server admin to be added."
	 */
	FString WhitelistKickReason{
		TEXT("You are not on this server's whitelist. Contact the server admin to be added.")
	};

	// ── Ban system ────────────────────────────────────────────────────────────

	/**
	 * When true, the ban system is active on every server start, overriding any
	 * runtime change made via !ban on / !ban off Discord commands.
	 * Default: true (ban list is enforced; set to false to disable enforcement).
	 */
	bool bBanSystemEnabled{ true };

	/**
	 * When true (default), Discord `!ban` commands and in-game `!ban` chat commands
	 * are enabled.  Set to false to disable the entire ban command interface while
	 * still enforcing bans on join (bBanSystemEnabled is unaffected).
	 *
	 * This is the "on/off from config" toggle for the command interface:
	 *   bBanCommandsEnabled=True   → admins can run !ban commands (subject to BanCommandRoleId)
	 *   bBanCommandsEnabled=False  → !ban commands are silently ignored; bans still enforced
	 *
	 * Default: true.
	 */
	bool bBanCommandsEnabled{ true };

	/**
	 * Prefix that triggers ban management commands from Discord.
	 * Set to an empty string to disable Discord-based ban management.
	 * Default: "!ban"
	 *
	 * Supported commands (type in the bridged Discord channel):
	 *   !ban add <name>    – ban a player by in-game name
	 *   !ban remove <name> – unban a player by in-game name
	 *   !ban list          – list all banned players
	 *   !ban status        – show current enabled/disabled state
	 *   !ban on            – enable the ban system
	 *   !ban off           – disable the ban system
	 */
	FString BanCommandPrefix{ TEXT("!ban") };

	/**
	 * Message posted to the main Discord channel whenever a banned player tries
	 * to join.  Leave empty to disable the notification.
	 *
	 * Available placeholder:
	 *   %PlayerName%  – in-game name of the player who was kicked.
	 *
	 * Example:
	 *   BanKickDiscordMessage=:hammer: **%PlayerName%** is banned and was kicked.
	 */
	FString BanKickDiscordMessage{
		TEXT(":hammer: **%PlayerName%** is banned from this server and was kicked.")
	};

	/**
	 * Reason shown in-game to the player when they are kicked for being banned.
	 * This is the text the player sees in the "Disconnected" screen.
	 * Default: "You are banned from this server."
	 */
	FString BanKickReason{ TEXT("You are banned from this server.") };

	// ── In-game commands ──────────────────────────────────────────────────────

	/**
	 * Prefix that triggers whitelist management commands when typed in the
	 * Satisfactory in-game chat.  Set to an empty string to disable in-game
	 * whitelist commands.
	 * Default: "!whitelist"
	 *
	 * Supported commands (type in the Satisfactory in-game chat):
	 *   !whitelist on            – enable the whitelist
	 *   !whitelist off           – disable the whitelist
	 *   !whitelist add <name>    – add a player by in-game name
	 *   !whitelist remove <name> – remove a player by in-game name
	 *   !whitelist list          – list all whitelisted players
	 *   !whitelist status        – show current enabled/disabled state
	 */
	FString InGameWhitelistCommandPrefix{ TEXT("!whitelist") };

	/**
	 * Prefix that triggers ban management commands when typed in the
	 * Satisfactory in-game chat.  Set to an empty string to disable in-game
	 * ban commands.
	 * Default: "!ban"
	 *
	 * Supported commands (type in the Satisfactory in-game chat):
	 *   !ban add <name>    – ban a player by in-game name
	 *   !ban remove <name> – unban a player by in-game name
	 *   !ban list          – list all banned players
	 *   !ban status        – show current enabled/disabled state
	 *   !ban on            – enable the ban system
	 *   !ban off           – disable the ban system
	 */
	FString InGameBanCommandPrefix{ TEXT("!ban") };

	// ── Player count presence ─────────────────────────────────────────────────

	/** When true, the bot's Discord presence activity shows the current player count. */
	bool bShowPlayerCountInPresence{ true };

	/** The custom text shown in the bot's Discord presence.
	 *  Type whatever you like.  Use %PlayerCount% to insert the live player count
	 *  and %ServerName% to insert the configured server name.
	 *  Example: "My Server with %PlayerCount% players" */
	FString PlayerCountPresenceFormat{ TEXT("Satisfactory with %PlayerCount% players") };

	/** How often (in seconds) to refresh the player count shown in the bot's presence.
	 *  Must be at least 15 seconds; the default is 60 seconds. */
	float PlayerCountUpdateIntervalSeconds{ 60.0f };

	/** Discord activity type used when displaying the player count.
	 *  Controls the verb shown before the activity text in Discord:
	 *   0 = Playing         → "Playing <your text>"
	 *   2 = Listening to    → "Listening to <your text>"
	 *   3 = Watching        → "Watching <your text>"
	 *   5 = Competing in    → "Competing in <your text>"
	 *  Default: 0 (Playing). */
	int32 PlayerCountActivityType{ 0 };

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

	/**
	 * Returns the absolute path to the whitelist-only backup INI (Saved/Config/).
	 *
	 * Written automatically whenever DefaultDiscordBridgeWhitelist.ini contains
	 * at least one user-set (uncommented) key.  If a mod update resets that
	 * file to the all-commented-out shipped template, the mod restores it from
	 * this backup so whitelist settings survive upgrades.
	 * On a deployed server:
	 *   <ServerRoot>/FactoryGame/Saved/Config/DiscordBridgeWhitelist.ini
	 */
	static FString GetWhitelistBackupConfigFilePath();

	/**
	 * Returns the absolute path to the ban-system-only backup INI (Saved/Config/).
	 *
	 * Written automatically whenever DefaultDiscordBridgeBan.ini contains at
	 * least one user-set (uncommented) key.  If a mod update resets that file
	 * to the all-commented-out shipped template, the mod restores it from this
	 * backup so ban settings survive upgrades.
	 * On a deployed server:
	 *   <ServerRoot>/FactoryGame/Saved/Config/DiscordBridgeBan.ini
	 */
	static FString GetBanBackupConfigFilePath();

	/**
	 * Returns the absolute path to the optional whitelist-only INI config file
	 * (mod Config folder).
	 *
	 * If this file exists, the whitelist settings it contains take priority over
	 * the same settings in the primary DefaultDiscordBridge.ini.  All other
	 * settings (connection, chat, ban, etc.) continue to come from the primary
	 * config.  This lets you keep whitelist configuration in its own dedicated
	 * file for easier management.
	 *
	 * The file is NOT created automatically – create it yourself when you want
	 * to use the separate-file workflow.  An annotated template can be found in
	 * the mod's Config folder as DefaultDiscordBridgeWhitelist.ini.
	 */
	static FString GetWhitelistConfigFilePath();

	/**
	 * Returns the absolute path to the optional ban-system-only INI config file
	 * (mod Config folder).
	 *
	 * If this file exists, the ban settings it contains take priority over the
	 * same settings in the primary DefaultDiscordBridge.ini.  All other settings
	 * (connection, chat, whitelist, etc.) continue to come from the primary
	 * config.  This lets you keep ban configuration in its own dedicated file
	 * for easier management.
	 *
	 * The file is NOT created automatically – create it yourself when you want
	 * to use the separate-file workflow.  An annotated template can be found in
	 * the mod's Config folder as DefaultDiscordBridgeBan.ini.
	 */
	static FString GetBanConfigFilePath();
};
