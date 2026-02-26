// Copyright Coffee Stain Studios. All Rights Reserved.

#include "DiscordBridgeConfig.h"

#include "HAL/PlatformFileManager.h"
#include "Misc/ConfigCacheIni.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"

// ─────────────────────────────────────────────────────────────────────────────
// Helpers
// ─────────────────────────────────────────────────────────────────────────────

namespace
{
	static const TCHAR* ConfigSection = TEXT("DiscordBridge");

	FString GetIniStringOrDefault(const FConfigFile& Cfg,
	                              const FString& Key,
	                              const FString& Default)
	{
		FString Value;
		return Cfg.GetString(ConfigSection, *Key, Value) ? Value : Default;
	}

	bool GetIniBoolOrDefault(const FConfigFile& Cfg,
	                         const FString& Key,
	                         bool Default)
	{
		bool Value = Default;
		Cfg.GetBool(ConfigSection, *Key, Value);
		return Value;
	}

	float GetIniFloatOrDefault(const FConfigFile& Cfg,
	                           const FString& Key,
	                           float Default)
	{
		FString Value;
		if (Cfg.GetString(ConfigSection, *Key, Value))
		{
			return FCString::Atof(*Value);
		}
		return Default;
	}

	int32 GetIniIntOrDefault(const FConfigFile& Cfg,
	                         const FString& Key,
	                         int32 Default)
	{
		FString Value;
		if (Cfg.GetString(ConfigSection, *Key, Value))
		{
			return FCString::Atoi(*Value);
		}
		return Default;
	}
} // anonymous namespace

// ─────────────────────────────────────────────────────────────────────────────
// FDiscordBridgeConfig
// ─────────────────────────────────────────────────────────────────────────────

FString FDiscordBridgeConfig::GetModConfigFilePath()
{
	// The primary config lives in the mod's own Config folder so it is the
	// first place a server operator would look.  On a deployed server:
	//   <ServerRoot>/FactoryGame/Mods/DiscordBridge/Config/DefaultDiscordBridge.ini
	// NOTE: Alpakit/SMM mod updates overwrite this file; the mod automatically
	// saves a backup to GetBackupConfigFilePath() so credentials survive upgrades.
	return FPaths::Combine(FPaths::ProjectDir(),
	                       TEXT("Mods"), TEXT("DiscordBridge"),
	                       TEXT("Config"), TEXT("DefaultDiscordBridge.ini"));
}

FString FDiscordBridgeConfig::GetBackupConfigFilePath()
{
	// Backup config in Saved/Config/ – never touched by Alpakit mod updates.
	// Written automatically whenever the primary config has valid credentials.
	// On a deployed server:
	//   <ServerRoot>/FactoryGame/Saved/Config/DiscordBridge.ini
	return FPaths::Combine(FPaths::ProjectSavedDir(), TEXT("Config"), TEXT("DiscordBridge.ini"));
}

FDiscordBridgeConfig FDiscordBridgeConfig::LoadOrCreate()
{
	FDiscordBridgeConfig Config;
	const FString ModFilePath    = GetModConfigFilePath();
	const FString BackupFilePath = GetBackupConfigFilePath();

	IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();

	// ── Step 1: load the primary config (mod folder) ─────────────────────────
	bool bLoadedFromMod = false;
	if (PlatformFile.FileExists(*ModFilePath))
	{
		FConfigFile ConfigFile;
		ConfigFile.Read(ModFilePath);

		Config.BotToken             = GetIniStringOrDefault(ConfigFile, TEXT("BotToken"),             TEXT(""));
		Config.ChannelId            = GetIniStringOrDefault(ConfigFile, TEXT("ChannelId"),            TEXT(""));
		Config.ServerName           = GetIniStringOrDefault(ConfigFile, TEXT("ServerName"),           Config.ServerName);
		Config.GameToDiscordFormat  = GetIniStringOrDefault(ConfigFile, TEXT("GameToDiscordFormat"),  Config.GameToDiscordFormat);
		Config.DiscordToGameFormat  = GetIniStringOrDefault(ConfigFile, TEXT("DiscordToGameFormat"),  Config.DiscordToGameFormat);
		Config.DiscordSenderFormat  = GetIniStringOrDefault(ConfigFile, TEXT("DiscordSenderFormat"),  Config.DiscordSenderFormat);
		Config.bIgnoreBotMessages   = GetIniBoolOrDefault  (ConfigFile, TEXT("bIgnoreBotMessages"),   Config.bIgnoreBotMessages);
		Config.ServerOnlineMessage  = GetIniStringOrDefault(ConfigFile, TEXT("ServerOnlineMessage"),  Config.ServerOnlineMessage);
		Config.ServerOfflineMessage = GetIniStringOrDefault(ConfigFile, TEXT("ServerOfflineMessage"), Config.ServerOfflineMessage);
		Config.bShowPlayerCountInPresence      = GetIniBoolOrDefault  (ConfigFile, TEXT("bShowPlayerCountInPresence"),      Config.bShowPlayerCountInPresence);
		Config.PlayerCountPresenceFormat       = GetIniStringOrDefault(ConfigFile, TEXT("PlayerCountPresenceFormat"),       Config.PlayerCountPresenceFormat);
		Config.PlayerCountUpdateIntervalSeconds = GetIniFloatOrDefault (ConfigFile, TEXT("PlayerCountUpdateIntervalSeconds"), Config.PlayerCountUpdateIntervalSeconds);
		Config.PlayerCountActivityType         = GetIniIntOrDefault   (ConfigFile, TEXT("PlayerCountActivityType"),         Config.PlayerCountActivityType);
		Config.WhitelistCommandPrefix          = GetIniStringOrDefault(ConfigFile, TEXT("WhitelistCommandPrefix"),          Config.WhitelistCommandPrefix);
		Config.WhitelistRoleId                 = GetIniStringOrDefault(ConfigFile, TEXT("WhitelistRoleId"),                 Config.WhitelistRoleId);
		Config.WhitelistChannelId              = GetIniStringOrDefault(ConfigFile, TEXT("WhitelistChannelId"),              Config.WhitelistChannelId);
		Config.WhitelistKickDiscordMessage     = GetIniStringOrDefault(ConfigFile, TEXT("WhitelistKickDiscordMessage"),     Config.WhitelistKickDiscordMessage);

		// Trim leading/trailing whitespace from credential fields to prevent
		// subtle mismatches when operators accidentally include spaces.
		Config.BotToken  = Config.BotToken.TrimStartAndEnd();
		Config.ChannelId = Config.ChannelId.TrimStartAndEnd();

		bLoadedFromMod = true;
		UE_LOG(LogTemp, Log, TEXT("DiscordBridge: Loaded config from %s"), *ModFilePath);

		// When BotToken is empty the file has not been configured yet – this
		// happens when Alpakit strips the ';' comments during packaging and
		// ships a comment-free ini.  Rewrite it with the full annotated
		// template so operators see the setting descriptions on first use.
		if (Config.BotToken.IsEmpty())
		{
			UE_LOG(LogTemp, Log,
			       TEXT("DiscordBridge: Config at '%s' has no BotToken – "
			            "rewriting with annotated template so comments are visible."),
			       *ModFilePath);
			bLoadedFromMod = false; // fall through to the DefaultContent write below
		}
	}

	if (!bLoadedFromMod)
	{
		if (!PlatformFile.FileExists(*ModFilePath))
		{
			// Primary config missing – create it with defaults so the operator has
			// a ready-made file to fill in.
			UE_LOG(LogTemp, Log,
			       TEXT("DiscordBridge: Config file not found at '%s'. "
			            "Creating it with defaults."), *ModFilePath);
		}

		const FString DefaultContent =
			TEXT("[DiscordBridge]\n")
			TEXT("; =============================================================================\n")
			TEXT(";  DiscordBridge - Primary Configuration File\n")
			TEXT("; =============================================================================\n")
			TEXT(";  HOW TO USE THIS FILE\n")
			TEXT(";  1. Fill in BotToken and ChannelId in the CONNECTION section below.\n")
			TEXT(";  2. Customise the message formats in the CHAT TWO-WAY section (optional).\n")
			TEXT(";  3. Restart the server.  The bridge starts automatically.\n")
			TEXT(";\n")
			TEXT(";  BACKUP - A copy is auto-saved to:\n")
			TEXT(";    <ServerRoot>/FactoryGame/Saved/Config/DiscordBridge.ini\n")
			TEXT(";  If a mod update resets this file your credentials are restored from that\n")
			TEXT(";  backup automatically until you copy them back here.\n")
			TEXT("; =============================================================================\n")
			TEXT(";  DISCORD BOT SETUP  (Discord Developer Portal -> your application -> Bot)\n")
			TEXT(";  1. Create a bot and copy its Token into BotToken below.\n")
			TEXT(";  2. Enable all three Privileged Gateway Intents:\n")
			TEXT(";       Presence Intent, Server Members Intent, Message Content Intent\n")
			TEXT(";  3. Invite the bot with Send Messages + Read Message History permissions.\n")
			TEXT(";  4. Enable Developer Mode in Discord, right-click the channel,\n")
			TEXT(";     and choose Copy Channel ID. Paste that into ChannelId below.\n")
			TEXT(";  5. Restart the server.\n")
			TEXT("; =============================================================================\n")
			TEXT("\n")
			TEXT("; -- CONNECTION ---------------------------------------------------------------\n")
			TEXT("; Discord bot token (Bot -> Token in the Developer Portal). Treat as a password.\n")
			TEXT("BotToken=\n")
			TEXT("; Snowflake ID of the Discord text channel to bridge with in-game chat.\n")
			TEXT("ChannelId=\n")
			TEXT("; Display name for this server.\n")
			TEXT("; Used as the %ServerName% placeholder in GameToDiscordFormat and PlayerCountPresenceFormat.\n")
			TEXT("; Example: ServerName=My Satisfactory Server\n")
			TEXT("ServerName=\n")
			TEXT("\n")
			TEXT("; -- CHAT TWO-WAY CUSTOMISATION -----------------------------------------------\n")
			TEXT("; These three settings control how messages look on both sides of the bridge.\n")
			TEXT(";\n")
			TEXT("; +-- GAME -> DISCORD ----------------------------------------------------------+\n")
			TEXT("; | Format applied when an in-game player message is forwarded to Discord.      |\n")
			TEXT("; | Placeholders: %ServerName%, %PlayerName%, %Message%                        |\n")
			TEXT("; | Examples:                                                                   |\n")
			TEXT("; |   GameToDiscordFormat=**%PlayerName%**: %Message%                          |\n")
			TEXT("; |   GameToDiscordFormat=**[%ServerName%] %PlayerName%**: %Message%           |\n")
			TEXT("; +-----------------------------------------------------------------------------+\n")
			TEXT("GameToDiscordFormat=**%PlayerName%**: %Message%\n")
			TEXT(";\n")
			TEXT("; +-- DISCORD -> GAME (message body) -------------------------------------------+\n")
			TEXT("; | Format used as the in-game chat message body when a Discord message arrives. |\n")
			TEXT("; | Placeholders: %Username%, %PlayerName% (alias for %Username%), %Message%    |\n")
			TEXT("; | Examples:                                                                   |\n")
			TEXT("; |   DiscordToGameFormat=%Message%              (just the message)             |\n")
			TEXT("; |   DiscordToGameFormat=%Username%: %Message%  (name + message)              |\n")
			TEXT("; +-----------------------------------------------------------------------------+\n")
			TEXT("DiscordToGameFormat=%Message%\n")
			TEXT(";\n")
			TEXT("; +-- DISCORD -> GAME (sender name column) -------------------------------------+\n")
			TEXT("; | Format for the sender name shown in the Satisfactory chat UI.              |\n")
			TEXT("; | Placeholders: %Username%, %PlayerName% (alias for %Username%)              |\n")
			TEXT("; | Examples:                                                                   |\n")
			TEXT("; |   DiscordSenderFormat=[Discord] %Username%       (default)                 |\n")
			TEXT("; |   DiscordSenderFormat=%Username%                 (name only)               |\n")
			TEXT("; |   DiscordSenderFormat=[Satisfactory] %Username%  (branded)                 |\n")
			TEXT("; +-----------------------------------------------------------------------------+\n")
			TEXT("DiscordSenderFormat=[Discord] %Username%\n")
			TEXT("\n")
			TEXT("; -- BEHAVIOUR ----------------------------------------------------------------\n")
			TEXT("; When True, messages from bot accounts are ignored (prevents echo loops).\n")
			TEXT("bIgnoreBotMessages=True\n")
			TEXT("\n")
			TEXT("; -- SERVER STATUS MESSAGES ---------------------------------------------------\n")
			TEXT("; These messages are posted to Discord when the server starts or stops.\n")
			TEXT("; Leave a value empty to disable that notification.\n")
			TEXT(";\n")
			TEXT("; Message posted to Discord when the server comes online.\n")
			TEXT("ServerOnlineMessage=:green_circle: Server is now **online**!\n")
			TEXT("; Message posted to Discord when the server shuts down.\n")
			TEXT("ServerOfflineMessage=:red_circle: Server is now **offline**.\n")
			TEXT("\n")
			TEXT("; -- PLAYER COUNT PRESENCE ----------------------------------------------------\n")
			TEXT("; Controls the bot's Discord status (the Now Playing activity).\n")
			TEXT(";\n")
			TEXT("; When True, the bot's Discord presence is updated periodically to show the\n")
			TEXT("; current number of connected players.\n")
			TEXT("bShowPlayerCountInPresence=True\n")
			TEXT(";\n")
			TEXT("; The text shown in the bot's Discord presence.\n")
			TEXT("; Write anything you like, or use the placeholders below.\n")
			TEXT(";   %PlayerCount%  - current number of connected players\n")
			TEXT(";   %ServerName%   - value of ServerName above\n")
			TEXT("; Examples:\n")
			TEXT(";   PlayerCountPresenceFormat=Satisfactory with %PlayerCount% players\n")
			TEXT(";   PlayerCountPresenceFormat=%ServerName% - %PlayerCount% online\n")
			TEXT("PlayerCountPresenceFormat=\n")
			TEXT(";\n")
			TEXT("; How often (in seconds) to refresh the player count in the bot's presence.\n")
			TEXT("; Minimum value is 15 seconds. Default is 60 seconds.\n")
			TEXT("PlayerCountUpdateIntervalSeconds=60.0\n")
			TEXT(";\n")
			TEXT("; Discord activity type - controls the verb Discord shows before the text.\n")
			TEXT(";   0 = Playing         ->  Playing <your text here>\n")
			TEXT(";   2 = Listening to    ->  Listening to <your text here>\n")
			TEXT(";   3 = Watching        ->  Watching <your text here>\n")
			TEXT(";   5 = Competing in    ->  Competing in <your text here>\n")
			TEXT("PlayerCountActivityType=0\n")
			TEXT("\n")
			TEXT("; -- WHITELIST ----------------------------------------------------------------\n")
			TEXT("; Controls the built-in server whitelist, manageable via Discord commands.\n")
			TEXT(";\n")
			TEXT("; Prefix that triggers whitelist commands when typed in the bridged channel.\n")
			TEXT("; Set to empty to disable Discord-based whitelist management entirely.\n")
			TEXT("; Available commands:\n")
			TEXT(";   !whitelist on                       - enable the whitelist\n")
			TEXT(";   !whitelist off                      - disable the whitelist\n")
			TEXT(";   !whitelist add <name>               - add a player by in-game name\n")
			TEXT(";   !whitelist remove <name>            - remove a player by in-game name\n")
			TEXT(";   !whitelist list                     - list all whitelisted players\n")
			TEXT(";   !whitelist status                   - show current enabled/disabled state\n")
			TEXT(";   !whitelist role add <discord_id>    - grant WhitelistRoleId to a Discord user\n")
			TEXT(";   !whitelist role remove <discord_id> - revoke WhitelistRoleId from a Discord user\n")
			TEXT("WhitelistCommandPrefix=!whitelist\n")
			TEXT(";\n")
			TEXT("; Snowflake ID of the Discord role assigned to whitelisted members.\n")
			TEXT("; Leave empty to disable Discord role integration.\n")
			TEXT("; When set, the bot checks this role before relaying messages from\n")
			TEXT("; WhitelistChannelId to the game, and the role add/remove commands\n")
			TEXT("; manage this role via the Discord REST API (bot needs Manage Roles permission).\n")
			TEXT("; To get the role ID: Discord Settings -> Advanced -> Developer Mode,\n")
			TEXT("; then right-click the role in Server Settings -> Roles and choose Copy Role ID.\n")
			TEXT("WhitelistRoleId=\n")
			TEXT(";\n")
			TEXT("; Snowflake ID of a dedicated Discord channel for whitelisted members.\n")
			TEXT("; Leave empty to disable the whitelist-only channel.\n")
			TEXT("; When set:\n")
			TEXT(";   - In-game messages from whitelisted players are ALSO posted here.\n")
			TEXT(";   - Messages from this channel are relayed to game only when the sender\n")
			TEXT(";     holds WhitelistRoleId (if WhitelistRoleId is configured).\n")
			TEXT("; Get the channel ID the same way as ChannelId above.\n")
			TEXT("WhitelistChannelId=\n")
			TEXT(";\n")
			TEXT("; Message posted to Discord when a non-whitelisted player is kicked.\n")
			TEXT("; Leave empty to disable this notification.\n")
			TEXT("; Placeholder: %PlayerName% - in-game name of the kicked player.\n")
			TEXT("WhitelistKickDiscordMessage=:boot: **%PlayerName%** tried to join but is not on the whitelist and was kicked.\n");

		// Ensure the Config directory exists before writing.
		PlatformFile.CreateDirectoryTree(*FPaths::GetPath(ModFilePath));

		if (FFileHelper::SaveStringToFile(DefaultContent, *ModFilePath))
		{
			UE_LOG(LogTemp, Log,
			       TEXT("DiscordBridge: Wrote default config to '%s'. "
			            "Set BotToken and ChannelId in that file, then restart "
			            "the server to enable the Discord bridge."), *ModFilePath);
		}
		else
		{
			UE_LOG(LogTemp, Warning,
			       TEXT("DiscordBridge: Could not write default config to '%s'."),
			       *ModFilePath);
		}
	}

	// ── Step 2: fall back to backup when credentials are missing ──────────────
	// This happens after a mod update resets the primary config file.
	if ((Config.BotToken.IsEmpty() || Config.ChannelId.IsEmpty()) &&
	    PlatformFile.FileExists(*BackupFilePath))
	{
		FConfigFile BackupFile;
		BackupFile.Read(BackupFilePath);

		const bool bHadToken   = !Config.BotToken.IsEmpty();
		const bool bHadChannel = !Config.ChannelId.IsEmpty();

		if (Config.BotToken.IsEmpty())
		{
			Config.BotToken = GetIniStringOrDefault(BackupFile, TEXT("BotToken"), TEXT("")).TrimStartAndEnd();
		}
		if (Config.ChannelId.IsEmpty())
		{
			Config.ChannelId = GetIniStringOrDefault(BackupFile, TEXT("ChannelId"), TEXT("")).TrimStartAndEnd();
		}

		// Restore all other user-customised settings from the backup so that
		// message formats and status messages also survive a mod update that
		// resets the primary config to its shipped defaults.
		Config.ServerName           = GetIniStringOrDefault(BackupFile, TEXT("ServerName"),           Config.ServerName);
		Config.GameToDiscordFormat  = GetIniStringOrDefault(BackupFile, TEXT("GameToDiscordFormat"),  Config.GameToDiscordFormat);
		Config.DiscordToGameFormat  = GetIniStringOrDefault(BackupFile, TEXT("DiscordToGameFormat"),  Config.DiscordToGameFormat);
		Config.DiscordSenderFormat  = GetIniStringOrDefault(BackupFile, TEXT("DiscordSenderFormat"),  Config.DiscordSenderFormat);
		Config.bIgnoreBotMessages   = GetIniBoolOrDefault  (BackupFile, TEXT("bIgnoreBotMessages"),   Config.bIgnoreBotMessages);
		Config.ServerOnlineMessage  = GetIniStringOrDefault(BackupFile, TEXT("ServerOnlineMessage"),  Config.ServerOnlineMessage);
		Config.ServerOfflineMessage = GetIniStringOrDefault(BackupFile, TEXT("ServerOfflineMessage"), Config.ServerOfflineMessage);
		Config.bShowPlayerCountInPresence       = GetIniBoolOrDefault  (BackupFile, TEXT("bShowPlayerCountInPresence"),       Config.bShowPlayerCountInPresence);
		Config.PlayerCountPresenceFormat        = GetIniStringOrDefault(BackupFile, TEXT("PlayerCountPresenceFormat"),        Config.PlayerCountPresenceFormat);
		Config.PlayerCountUpdateIntervalSeconds = GetIniFloatOrDefault (BackupFile, TEXT("PlayerCountUpdateIntervalSeconds"), Config.PlayerCountUpdateIntervalSeconds);
		Config.PlayerCountActivityType          = GetIniIntOrDefault   (BackupFile, TEXT("PlayerCountActivityType"),          Config.PlayerCountActivityType);
		Config.WhitelistCommandPrefix           = GetIniStringOrDefault(BackupFile, TEXT("WhitelistCommandPrefix"),           Config.WhitelistCommandPrefix);
		Config.WhitelistRoleId                  = GetIniStringOrDefault(BackupFile, TEXT("WhitelistRoleId"),                  Config.WhitelistRoleId);
		Config.WhitelistChannelId               = GetIniStringOrDefault(BackupFile, TEXT("WhitelistChannelId"),               Config.WhitelistChannelId);
		Config.WhitelistKickDiscordMessage      = GetIniStringOrDefault(BackupFile, TEXT("WhitelistKickDiscordMessage"),      Config.WhitelistKickDiscordMessage);

		if (!bHadToken || !bHadChannel)
		{
			UE_LOG(LogTemp, Log,
			       TEXT("DiscordBridge: Credentials not set in primary config '%s'. "
			            "Loaded from backup at '%s'. "
			            "Copy your BotToken and ChannelId back into the primary config "
			            "to silence this message."),
			       *ModFilePath, *BackupFilePath);
		}
	}

	// ── Step 3: keep the backup up to date ────────────────────────────────────
	// Whenever the primary config has valid credentials, write an up-to-date
	// backup so they survive the next mod update.
	if (!Config.BotToken.IsEmpty() && !Config.ChannelId.IsEmpty() && bLoadedFromMod)
	{
		const FString BackupContent = FString::Printf(
			TEXT("[DiscordBridge]\n")
			TEXT("; Auto-generated backup of %s\n")
			TEXT("; This file is read automatically when the primary config is missing credentials.\n")
			TEXT("BotToken=%s\n")
			TEXT("ChannelId=%s\n")
			TEXT("ServerName=%s\n")
			TEXT("GameToDiscordFormat=%s\n")
			TEXT("DiscordToGameFormat=%s\n")
			TEXT("DiscordSenderFormat=%s\n")
			TEXT("bIgnoreBotMessages=%s\n")
			TEXT("ServerOnlineMessage=%s\n")
			TEXT("ServerOfflineMessage=%s\n")
			TEXT("bShowPlayerCountInPresence=%s\n")
			TEXT("PlayerCountPresenceFormat=%s\n")
			TEXT("PlayerCountUpdateIntervalSeconds=%s\n")
			TEXT("PlayerCountActivityType=%d\n")
			TEXT("WhitelistCommandPrefix=%s\n")
			TEXT("WhitelistRoleId=%s\n")
			TEXT("WhitelistChannelId=%s\n")
			TEXT("WhitelistKickDiscordMessage=%s\n"),
			*ModFilePath,
			*Config.BotToken,
			*Config.ChannelId,
			*Config.ServerName,
			*Config.GameToDiscordFormat,
			*Config.DiscordToGameFormat,
			*Config.DiscordSenderFormat,
			Config.bIgnoreBotMessages ? TEXT("True") : TEXT("False"),
			*Config.ServerOnlineMessage,
			*Config.ServerOfflineMessage,
			Config.bShowPlayerCountInPresence ? TEXT("True") : TEXT("False"),
			*Config.PlayerCountPresenceFormat,
			*FString::SanitizeFloat(Config.PlayerCountUpdateIntervalSeconds),
			Config.PlayerCountActivityType,
			*Config.WhitelistCommandPrefix,
			*Config.WhitelistRoleId,
			*Config.WhitelistChannelId,
			*Config.WhitelistKickDiscordMessage);

		PlatformFile.CreateDirectoryTree(*FPaths::GetPath(BackupFilePath));

		if (FFileHelper::SaveStringToFile(BackupContent, *BackupFilePath))
		{
			UE_LOG(LogTemp, Log,
			       TEXT("DiscordBridge: Updated backup config at '%s'."), *BackupFilePath);
		}
		else
		{
			UE_LOG(LogTemp, Warning,
			       TEXT("DiscordBridge: Could not write backup config to '%s'."),
			       *BackupFilePath);
		}
	}

	return Config;
}
