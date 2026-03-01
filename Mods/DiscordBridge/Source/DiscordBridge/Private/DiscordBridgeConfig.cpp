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

	// Returns the INI value when the key exists (including empty string values).
	// Use for optional settings where an empty value intentionally disables the feature.
	FString GetIniStringOrDefault(const FConfigFile& Cfg,
	                              const FString& Key,
	                              const FString& Default)
	{
		FString Value;
		return Cfg.GetString(ConfigSection, *Key, Value) ? Value : Default;
	}

	// Returns the INI value only when non-empty; falls back to Default otherwise.
	// Use for format/reason strings where leaving a setting blank means "use the default".
	FString GetIniStringOrFallback(const FConfigFile& Cfg,
	                               const FString& Key,
	                               const FString& Default)
	{
		FString Value;
		return (Cfg.GetString(ConfigSection, *Key, Value) && !Value.IsEmpty()) ? Value : Default;
	}

	bool GetIniBoolOrDefault(const FConfigFile& Cfg,
	                         const FString& Key,
	                         bool Default)
	{
		FString StrValue;
		if (!Cfg.GetString(ConfigSection, *Key, StrValue) || StrValue.IsEmpty())
			return Default;
		bool Value = Default;
		Cfg.GetBool(ConfigSection, *Key, Value);
		return Value;
	}

	float GetIniFloatOrDefault(const FConfigFile& Cfg,
	                           const FString& Key,
	                           float Default)
	{
		FString Value;
		if (Cfg.GetString(ConfigSection, *Key, Value) && !Value.IsEmpty())
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
		if (Cfg.GetString(ConfigSection, *Key, Value) && !Value.IsEmpty())
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

FString FDiscordBridgeConfig::GetWhitelistConfigFilePath()
{
	// Optional separate whitelist config in the mod's Config folder.
	// When present, whitelist settings here take priority over DefaultDiscordBridge.ini.
	// On a deployed server:
	//   <ServerRoot>/FactoryGame/Mods/DiscordBridge/Config/DefaultDiscordBridgeWhitelist.ini
	return FPaths::Combine(FPaths::ProjectDir(),
	                       TEXT("Mods"), TEXT("DiscordBridge"),
	                       TEXT("Config"), TEXT("DefaultDiscordBridgeWhitelist.ini"));
}

FString FDiscordBridgeConfig::GetBanConfigFilePath()
{
	// Optional separate ban-system config in the mod's Config folder.
	// When present, ban settings here take priority over DefaultDiscordBridge.ini.
	// On a deployed server:
	//   <ServerRoot>/FactoryGame/Mods/DiscordBridge/Config/DefaultDiscordBridgeBan.ini
	return FPaths::Combine(FPaths::ProjectDir(),
	                       TEXT("Mods"), TEXT("DiscordBridge"),
	                       TEXT("Config"), TEXT("DefaultDiscordBridgeBan.ini"));
}

FString FDiscordBridgeConfig::GetWhitelistBackupConfigFilePath()
{
	// Backup for the optional whitelist config – never touched by mod updates.
	// Written automatically whenever DefaultDiscordBridgeWhitelist.ini has
	// at least one user-set key.
	// On a deployed server:
	//   <ServerRoot>/FactoryGame/Saved/Config/DiscordBridgeWhitelist.ini
	return FPaths::Combine(FPaths::ProjectSavedDir(), TEXT("Config"), TEXT("DiscordBridgeWhitelist.ini"));
}

FString FDiscordBridgeConfig::GetBanBackupConfigFilePath()
{
	// Backup for the optional ban config – never touched by mod updates.
	// Written automatically whenever DefaultDiscordBridgeBan.ini has at
	// least one user-set key.
	// On a deployed server:
	//   <ServerRoot>/FactoryGame/Saved/Config/DiscordBridgeBan.ini
	return FPaths::Combine(FPaths::ProjectSavedDir(), TEXT("Config"), TEXT("DiscordBridgeBan.ini"));
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
		Config.GameToDiscordFormat  = GetIniStringOrFallback(ConfigFile, TEXT("GameToDiscordFormat"),  Config.GameToDiscordFormat);
		Config.DiscordToGameFormat  = GetIniStringOrFallback(ConfigFile, TEXT("DiscordToGameFormat"),  Config.DiscordToGameFormat);

		// Backward compatibility: if the old separate DiscordSenderFormat key is
		// present and the operator has not overridden DiscordToGameFormat to a value
		// that already mentions %Message%, synthesise a combined format so existing
		// configs continue to render as before.
		{
			FString OldSenderFmt;
			if (ConfigFile.GetString(ConfigSection, TEXT("DiscordSenderFormat"), OldSenderFmt) && !OldSenderFmt.IsEmpty())
			{
				// Only auto-combine when DiscordToGameFormat is still the default
				// (i.e. the operator has not explicitly customised it).
				if (Config.DiscordToGameFormat == TEXT("[Discord] %Username%: %Message%"))
				{
					Config.DiscordToGameFormat = OldSenderFmt + TEXT(": %Message%");
				}
				UE_LOG(LogTemp, Warning,
				       TEXT("DiscordBridge: 'DiscordSenderFormat' is deprecated. "
				            "Use 'DiscordToGameFormat' to control the full in-game line. "
				            "Effective format is now: \"%s\""), *Config.DiscordToGameFormat);
			}
		}
		Config.bIgnoreBotMessages   = GetIniBoolOrDefault  (ConfigFile, TEXT("IgnoreBotMessages"),
		                              GetIniBoolOrDefault  (ConfigFile, TEXT("bIgnoreBotMessages"),   Config.bIgnoreBotMessages));
		Config.ServerOnlineMessage  = GetIniStringOrDefault(ConfigFile, TEXT("ServerOnlineMessage"),  Config.ServerOnlineMessage);
		Config.ServerOfflineMessage = GetIniStringOrDefault(ConfigFile, TEXT("ServerOfflineMessage"), Config.ServerOfflineMessage);
		Config.bShowPlayerCountInPresence      = GetIniBoolOrDefault  (ConfigFile, TEXT("ShowPlayerCountInPresence"),
		                                         GetIniBoolOrDefault  (ConfigFile, TEXT("bShowPlayerCountInPresence"),      Config.bShowPlayerCountInPresence));
		Config.PlayerCountPresenceFormat       = GetIniStringOrFallback(ConfigFile, TEXT("PlayerCountPresenceFormat"),       Config.PlayerCountPresenceFormat);
		Config.PlayerCountUpdateIntervalSeconds = GetIniFloatOrDefault (ConfigFile, TEXT("PlayerCountUpdateIntervalSeconds"), Config.PlayerCountUpdateIntervalSeconds);
		Config.PlayerCountActivityType         = GetIniIntOrDefault   (ConfigFile, TEXT("PlayerCountActivityType"),         Config.PlayerCountActivityType);
		Config.WhitelistCommandRoleId          = GetIniStringOrDefault(ConfigFile, TEXT("WhitelistCommandRoleId"),          Config.WhitelistCommandRoleId);
		Config.BanCommandRoleId                = GetIniStringOrDefault(ConfigFile, TEXT("BanCommandRoleId"),                Config.BanCommandRoleId);
		Config.WhitelistCommandPrefix          = GetIniStringOrDefault(ConfigFile, TEXT("WhitelistCommandPrefix"),          Config.WhitelistCommandPrefix);
		Config.WhitelistRoleId                 = GetIniStringOrDefault(ConfigFile, TEXT("WhitelistRoleId"),                 Config.WhitelistRoleId);
		Config.WhitelistChannelId              = GetIniStringOrDefault(ConfigFile, TEXT("WhitelistChannelId"),              Config.WhitelistChannelId);
		Config.WhitelistKickDiscordMessage     = GetIniStringOrDefault(ConfigFile, TEXT("WhitelistKickDiscordMessage"),     Config.WhitelistKickDiscordMessage);
		Config.WhitelistKickReason             = GetIniStringOrFallback(ConfigFile, TEXT("WhitelistKickReason"),             Config.WhitelistKickReason);
		Config.bWhitelistEnabled               = GetIniBoolOrDefault  (ConfigFile, TEXT("WhitelistEnabled"),               Config.bWhitelistEnabled);
		Config.bBanSystemEnabled               = GetIniBoolOrDefault  (ConfigFile, TEXT("BanSystemEnabled"),               Config.bBanSystemEnabled);
		Config.BanCommandPrefix                = GetIniStringOrDefault(ConfigFile, TEXT("BanCommandPrefix"),                Config.BanCommandPrefix);
		Config.BanChannelId                    = GetIniStringOrDefault(ConfigFile, TEXT("BanChannelId"),                    Config.BanChannelId);
		Config.bBanCommandsEnabled             = GetIniBoolOrDefault  (ConfigFile, TEXT("BanCommandsEnabled"),             Config.bBanCommandsEnabled);
		Config.BanKickDiscordMessage           = GetIniStringOrDefault(ConfigFile, TEXT("BanKickDiscordMessage"),           Config.BanKickDiscordMessage);
		Config.BanKickReason                   = GetIniStringOrFallback(ConfigFile, TEXT("BanKickReason"),                   Config.BanKickReason);
		Config.InGameWhitelistCommandPrefix    = GetIniStringOrDefault(ConfigFile, TEXT("InGameWhitelistCommandPrefix"),    Config.InGameWhitelistCommandPrefix);
		Config.InGameBanCommandPrefix          = GetIniStringOrDefault(ConfigFile, TEXT("InGameBanCommandPrefix"),          Config.InGameBanCommandPrefix);

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
		else
		{
			// Detect configs written before the whitelist / ban system was added
			// (upgrade scenario).  If either section's key-value pair is absent
			// from the file, append the missing sections so server operators can
			// see and configure the new settings without losing their existing ones.
			// Exception: if a dedicated separate file exists for that subsystem,
			// the operator has intentionally moved those settings there – do not
			// re-add them to the primary config file.
			FString TmpVal;
			const bool bFileHasWhitelist = ConfigFile.GetString(ConfigSection, TEXT("WhitelistEnabled"), TmpVal);
			const bool bFileHasBan       = ConfigFile.GetString(ConfigSection, TEXT("BanSystemEnabled"), TmpVal);

			const bool bHasSeparateWhitelistFile = PlatformFile.FileExists(*GetWhitelistConfigFilePath());
			const bool bHasSeparateBanFile       = PlatformFile.FileExists(*GetBanConfigFilePath());

			if ((!bFileHasWhitelist && !bHasSeparateWhitelistFile) ||
			    (!bFileHasBan       && !bHasSeparateBanFile))
			{
				UE_LOG(LogTemp, Log,
				       TEXT("DiscordBridge: Config at '%s' is missing whitelist/ban settings "
				            "(older version detected). Appending new settings."),
				       *ModFilePath);

				FString ExistingContent;
				FFileHelper::LoadFileToString(ExistingContent, *ModFilePath);

				FString AppendContent;

				if (!bFileHasWhitelist && !bHasSeparateWhitelistFile)
				{
					AppendContent +=
						TEXT("\n")
						TEXT("; -- WHITELIST (added by mod update) -----------------------------------------\n")
						TEXT("; Controls the built-in server whitelist, manageable via Discord commands.\n")
						TEXT(";\n")
						TEXT("; The whitelist and the ban system are COMPLETELY INDEPENDENT of each other.\n")
						TEXT("; You can use either one, both, or neither:\n")
						TEXT(";\n")
						TEXT(";   Whitelist only:   WhitelistEnabled=True,  BanSystemEnabled=False\n")
						TEXT(";   Ban system only:  WhitelistEnabled=False, BanSystemEnabled=True\n")
						TEXT(";   Both:             WhitelistEnabled=True,  BanSystemEnabled=True\n")
						TEXT(";   Neither:          WhitelistEnabled=False, BanSystemEnabled=False\n")
						TEXT(";\n")
						TEXT("; Whether the whitelist is active. Applied on every server restart.\n")
						TEXT("; Default: False (all players can join).\n")
						TEXT("WhitelistEnabled=False\n")
						TEXT(";\n")
						TEXT("; Snowflake ID of the Discord role whose members may run !whitelist commands.\n")
						TEXT("; Leave empty (default) to disable !whitelist commands for all Discord users.\n")
						TEXT("WhitelistCommandRoleId=\n")
						TEXT(";\n")
						TEXT("; Prefix that triggers whitelist commands in the bridged Discord channel.\n")
						TEXT("; Set to empty to disable Discord-based whitelist management entirely.\n")
						TEXT("WhitelistCommandPrefix=!whitelist\n")
						TEXT(";\n")
						TEXT("; Snowflake ID of the Discord role assigned to whitelisted members.\n")
						TEXT("; Leave empty to disable Discord role integration.\n")
						TEXT("WhitelistRoleId=\n")
						TEXT(";\n")
						TEXT("; Snowflake ID of a dedicated Discord channel for whitelisted members.\n")
						TEXT("; Leave empty to disable the whitelist-only channel.\n")
						TEXT("WhitelistChannelId=\n")
						TEXT(";\n")
						TEXT("; Message posted to Discord when a non-whitelisted player is kicked.\n")
						TEXT("; Leave empty to disable this notification.\n")
						TEXT("; Placeholder: %PlayerName% - in-game name of the kicked player.\n")
						TEXT("WhitelistKickDiscordMessage=:boot: **%PlayerName%** tried to join but is not on the whitelist and was kicked.\n")
						TEXT(";\n")
						TEXT("; Reason shown in-game to the player when they are kicked for not being whitelisted.\n")
						TEXT("WhitelistKickReason=You are not on this server's whitelist. Contact the server admin to be added.\n")
						TEXT(";\n")
						TEXT("; Prefix that triggers whitelist commands in the in-game chat.\n")
						TEXT("; Set to empty to disable in-game whitelist commands.\n")
						TEXT("InGameWhitelistCommandPrefix=!whitelist\n");
				}

				if (!bFileHasBan && !bHasSeparateBanFile)
				{
					AppendContent +=
						TEXT("\n")
						TEXT("; -- BAN SYSTEM (added by mod update) ----------------------------------------\n")
						TEXT("; Controls the built-in player ban system, manageable via Discord commands.\n")
						TEXT("; Bans are stored in <ServerRoot>/FactoryGame/Saved/ServerBanlist.json.\n")
						TEXT(";\n")
						TEXT("; The ban system and the whitelist are COMPLETELY INDEPENDENT of each other.\n")
						TEXT("; See the quick-start guide in the WHITELIST section above.\n")
						TEXT(";\n")
						TEXT("; Controls whether the ban system is active on startup. Applied on every\n")
						TEXT("; server restart — change this value and restart to enable or disable bans.\n")
						TEXT("; Runtime !ban on / !ban off commands update the state for the current\n")
						TEXT("; session; this config value takes effect again on the next restart.\n")
						TEXT("; Default: True (ban enforcement is on by default).\n")
						TEXT("BanSystemEnabled=True\n")
						TEXT(";\n")
						TEXT("; Snowflake ID of the Discord role whose members may run !ban commands.\n")
						TEXT("; Leave empty (default) to disable !ban commands for all Discord users.\n")
						TEXT("BanCommandRoleId=\n")
						TEXT(";\n")
						TEXT("; Prefix that triggers ban commands in the bridged Discord channel.\n")
						TEXT("; Set to empty to disable Discord-based ban management entirely.\n")
						TEXT("BanCommandPrefix=!ban\n")
						TEXT(";\n")
						TEXT("; Snowflake ID of a dedicated Discord channel for ban management.\n")
						TEXT("; Leave empty to disable the ban-only channel.\n")
						TEXT("; When set:\n")
						TEXT(";   - !ban commands typed here are accepted (sender must hold BanCommandRoleId).\n")
						TEXT(";   - Ban-kick notifications are ALSO posted here (in addition to the main channel).\n")
						TEXT("; Get the channel ID the same way as ChannelId above.\n")
						TEXT("BanChannelId=\n")
						TEXT(";\n")
						TEXT("; Message posted to Discord when a banned player is kicked.\n")
						TEXT("; Leave empty to disable this notification.\n")
						TEXT("; Placeholder: %PlayerName% - in-game name of the kicked player.\n")
						TEXT("BanKickDiscordMessage=:hammer: **%PlayerName%** is banned from this server and was kicked.\n")
						TEXT(";\n")
						TEXT("; Reason shown in-game to the player when they are kicked for being banned.\n")
						TEXT("BanKickReason=You are banned from this server.\n")
						TEXT(";\n")
						TEXT("; Prefix that triggers ban commands in the in-game chat.\n")
						TEXT("; Set to empty to disable in-game ban commands.\n")
						TEXT("InGameBanCommandPrefix=!ban\n");
				}

				if (FFileHelper::SaveStringToFile(ExistingContent + AppendContent, *ModFilePath))
				{
					UE_LOG(LogTemp, Log,
					       TEXT("DiscordBridge: Updated '%s' with whitelist/ban settings. "
					            "Review and configure them, then restart the server."),
					       *ModFilePath);
				}
				else
				{
					UE_LOG(LogTemp, Warning,
					       TEXT("DiscordBridge: Could not update '%s' with whitelist/ban settings."),
					       *ModFilePath);
				}
			}

			// Second pass: detect individual settings that were added in later updates
			// but may be absent from configs that already have the whitelist/ban sections.
			// Only appends the specific missing keys so no existing custom values are lost.
			{
				FString AppendContent2;

				if (bFileHasWhitelist &&
				    !ConfigFile.GetString(ConfigSection, TEXT("InGameWhitelistCommandPrefix"), TmpVal))
				{
					AppendContent2 +=
						TEXT("\n")
						TEXT("; InGameWhitelistCommandPrefix (added by mod update) -------------------\n")
						TEXT("; Prefix that triggers whitelist commands when typed in the in-game chat.\n")
						TEXT("; Set to empty to disable in-game whitelist commands.\n")
						TEXT("InGameWhitelistCommandPrefix=!whitelist\n");
				}

				if (bFileHasBan &&
				    !ConfigFile.GetString(ConfigSection, TEXT("BanCommandRoleId"), TmpVal))
				{
					AppendContent2 +=
						TEXT("\n")
						TEXT("; BanCommandRoleId (added by mod update) --------------------------------\n")
						TEXT("; Snowflake ID of the Discord role whose members may run !ban commands.\n")
						TEXT("; Leave empty (default) to disable !ban commands for all Discord users.\n")
						TEXT("; Get the role ID the same way as WhitelistCommandRoleId above.\n")
						TEXT(";\n")
						TEXT("; IMPORTANT: holding this role does NOT bypass the ban check when joining.\n")
						TEXT("; These members are still banned if their name appears on the ban list.\n")
						TEXT("BanCommandRoleId=\n");
				}

				if (bFileHasBan &&
				    !ConfigFile.GetString(ConfigSection, TEXT("InGameBanCommandPrefix"), TmpVal))
				{
					AppendContent2 +=
						TEXT("\n")
						TEXT("; InGameBanCommandPrefix (added by mod update) -------------------------\n")
						TEXT("; Prefix that triggers ban commands when typed in the in-game chat.\n")
						TEXT("; Set to empty to disable in-game ban commands.\n")
						TEXT("InGameBanCommandPrefix=!ban\n");
				}

				if (bFileHasBan &&
				    !ConfigFile.GetString(ConfigSection, TEXT("BanChannelId"), TmpVal))
				{
					AppendContent2 +=
						TEXT("\n")
						TEXT("; BanChannelId (added by mod update) -----------------------------------\n")
						TEXT("; Snowflake ID of a dedicated Discord channel for ban management.\n")
						TEXT("; Leave empty to disable the ban-only channel.\n")
						TEXT("; When set:\n")
						TEXT(";   - !ban commands typed here are accepted (sender must hold BanCommandRoleId).\n")
						TEXT(";   - Ban-kick notifications are ALSO posted here (in addition to the main channel).\n")
						TEXT("; Get the channel ID the same way as ChannelId above.\n")
						TEXT("BanChannelId=\n");
				}

				if (bFileHasBan &&
				    !ConfigFile.GetString(ConfigSection, TEXT("BanCommandsEnabled"), TmpVal))
				{
					AppendContent2 +=
						TEXT("\n")
						TEXT("; BanCommandsEnabled (added by mod update) ----------------------------\n")
						TEXT("; When True (default), !ban Discord and in-game commands are enabled.\n")
						TEXT("; Set to False to disable ban commands while still enforcing bans.\n")
						TEXT("BanCommandsEnabled=True\n");
				}

				if (!AppendContent2.IsEmpty())
				{
					UE_LOG(LogTemp, Log,
					       TEXT("DiscordBridge: Config at '%s' is missing individual settings "
					            "(older version detected). Appending missing entries."),
					       *ModFilePath);

					FString ExistingContent2;
					FFileHelper::LoadFileToString(ExistingContent2, *ModFilePath);

					if (FFileHelper::SaveStringToFile(ExistingContent2 + AppendContent2, *ModFilePath))
					{
						UE_LOG(LogTemp, Log,
						       TEXT("DiscordBridge: Updated '%s' with missing settings. "
						            "Review and configure them, then restart the server."),
						       *ModFilePath);
					}
					else
					{
						UE_LOG(LogTemp, Warning,
						       TEXT("DiscordBridge: Could not update '%s' with missing settings."),
						       *ModFilePath);
					}
				}
			}
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
			TEXT("; DiscordBridge - Primary Configuration File\n")
			TEXT("; ===========================================\n")
			TEXT("; 1. Set BotToken and ChannelId below.\n")
			TEXT("; 2. Restart the server. The bridge starts automatically.\n")
			TEXT("; Backup: <ServerRoot>/FactoryGame/Saved/Config/DiscordBridge.ini (auto-saved)\n")
			TEXT("; Optional separate files (settings there override what is set below):\n")
			TEXT(";   DefaultDiscordBridgeWhitelist.ini  – whitelist settings only\n")
			TEXT(";   DefaultDiscordBridgeBan.ini         – ban system settings only\n")
			TEXT("; Bot setup: Discord Developer Portal -> your app -> Bot\n")
			TEXT(";   - Enable Presence, Server Members and Message Content intents.\n")
			TEXT(";   - Invite the bot with Send Messages + Read Message History permissions.\n")
			TEXT(";   - Enable Developer Mode in Discord, right-click the channel, Copy Channel ID.\n")
			TEXT("\n")
			TEXT("; -- CONNECTION ---------------------------------------------------------------\n")
			TEXT("; Discord bot token (Bot -> Token in Developer Portal). Treat as a password.\n")
			TEXT("BotToken=\n")
			TEXT("; Snowflake ID of the Discord text channel to bridge with in-game chat.\n")
			TEXT("ChannelId=\n")
			TEXT("; Display name for this server. Used as the %ServerName% placeholder.\n")
			TEXT("ServerName=\n")
			TEXT("\n")
			TEXT("; -- CHAT CUSTOMISATION -------------------------------------------------------\n")
			TEXT("; Format for game -> Discord. Placeholders: %ServerName%, %PlayerName%, %Message%\n")
			TEXT("; Default: **%PlayerName%**: %Message%\n")
			TEXT("GameToDiscordFormat=\n")
			TEXT("; Format for Discord -> game. Placeholders: %Username%, %PlayerName%, %Message%\n")
			TEXT("; Default: [Discord] %Username%: %Message%\n")
			TEXT("DiscordToGameFormat=\n")
			TEXT("\n")
			TEXT("; -- BEHAVIOUR ----------------------------------------------------------------\n")
			TEXT("; When True, messages from bot accounts are ignored (prevents echo loops).\n")
			TEXT("; Default: True\n")
			TEXT("IgnoreBotMessages=\n")
			TEXT("\n")
			TEXT("; -- SERVER STATUS MESSAGES ---------------------------------------------------\n")
			TEXT("; Message posted to Discord when the server starts. Leave empty to disable.\n")
			TEXT("ServerOnlineMessage=:green_circle: Server is now **online**!\n")
			TEXT("; Message posted to Discord when the server stops. Leave empty to disable.\n")
			TEXT("ServerOfflineMessage=:red_circle: Server is now **offline**.\n")
			TEXT("\n")
			TEXT("; -- PLAYER COUNT PRESENCE ----------------------------------------------------\n")
			TEXT("; When True, the bot's Discord status shows the current player count.\n")
			TEXT("; Default: True\n")
			TEXT("ShowPlayerCountInPresence=\n")
			TEXT("; Text shown in the bot's Discord presence. Placeholders: %PlayerCount%, %ServerName%\n")
			TEXT("; Default: Satisfactory with %PlayerCount% players\n")
			TEXT("PlayerCountPresenceFormat=\n")
			TEXT("; How often (in seconds) to refresh the player count. Minimum 15. Default 60.\n")
			TEXT("PlayerCountUpdateIntervalSeconds=\n")
			TEXT("; Activity type: 0=Playing, 2=Listening to, 3=Watching, 5=Competing in. Default 0.\n")
			TEXT("PlayerCountActivityType=\n")
			TEXT("\n")
			TEXT("; -- WHITELIST / BAN SYSTEM ---------------------------------------------------\n")
			TEXT("; Whitelist and ban system settings are configured in their own dedicated files:\n")
			TEXT(";   DefaultDiscordBridgeWhitelist.ini  - all whitelist settings\n")
			TEXT(";   DefaultDiscordBridgeBan.ini         - all ban system settings\n")
			TEXT("; Those files ship with the mod. Uncomment and fill in the settings you need\n")
			TEXT("; there, then restart the server. No whitelist or ban settings are required here.\n");

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
	// Whitelist/ban settings in this backup are loaded for backward compatibility
	// only (old backups written before dedicated backup files were introduced).
	// Going forward, whitelist/ban settings are always in their own dedicated
	// backups (DiscordBridgeWhitelist.ini / DiscordBridgeBan.ini); Step 2b reads
	// those and overrides any values loaded here.
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
		Config.GameToDiscordFormat  = GetIniStringOrFallback(BackupFile, TEXT("GameToDiscordFormat"),  Config.GameToDiscordFormat);
		Config.DiscordToGameFormat  = GetIniStringOrFallback(BackupFile, TEXT("DiscordToGameFormat"),  Config.DiscordToGameFormat);
		Config.bIgnoreBotMessages   = GetIniBoolOrDefault  (BackupFile, TEXT("IgnoreBotMessages"),
	                              GetIniBoolOrDefault  (BackupFile, TEXT("bIgnoreBotMessages"),   Config.bIgnoreBotMessages));
		Config.ServerOnlineMessage  = GetIniStringOrDefault(BackupFile, TEXT("ServerOnlineMessage"),  Config.ServerOnlineMessage);
		Config.ServerOfflineMessage = GetIniStringOrDefault(BackupFile, TEXT("ServerOfflineMessage"), Config.ServerOfflineMessage);
		Config.bShowPlayerCountInPresence       = GetIniBoolOrDefault  (BackupFile, TEXT("ShowPlayerCountInPresence"),
	                                          GetIniBoolOrDefault  (BackupFile, TEXT("bShowPlayerCountInPresence"),       Config.bShowPlayerCountInPresence));
		Config.PlayerCountPresenceFormat        = GetIniStringOrFallback(BackupFile, TEXT("PlayerCountPresenceFormat"),        Config.PlayerCountPresenceFormat);
		Config.PlayerCountUpdateIntervalSeconds = GetIniFloatOrDefault (BackupFile, TEXT("PlayerCountUpdateIntervalSeconds"), Config.PlayerCountUpdateIntervalSeconds);
		Config.PlayerCountActivityType          = GetIniIntOrDefault   (BackupFile, TEXT("PlayerCountActivityType"),          Config.PlayerCountActivityType);
		Config.WhitelistCommandRoleId           = GetIniStringOrDefault(BackupFile, TEXT("WhitelistCommandRoleId"),           Config.WhitelistCommandRoleId);
		Config.BanCommandRoleId                 = GetIniStringOrDefault(BackupFile, TEXT("BanCommandRoleId"),                 Config.BanCommandRoleId);
		Config.WhitelistCommandPrefix           = GetIniStringOrDefault(BackupFile, TEXT("WhitelistCommandPrefix"),           Config.WhitelistCommandPrefix);
		Config.WhitelistRoleId                  = GetIniStringOrDefault(BackupFile, TEXT("WhitelistRoleId"),                  Config.WhitelistRoleId);
		Config.WhitelistChannelId               = GetIniStringOrDefault(BackupFile, TEXT("WhitelistChannelId"),               Config.WhitelistChannelId);
		Config.WhitelistKickDiscordMessage      = GetIniStringOrDefault(BackupFile, TEXT("WhitelistKickDiscordMessage"),      Config.WhitelistKickDiscordMessage);
		Config.WhitelistKickReason              = GetIniStringOrFallback(BackupFile, TEXT("WhitelistKickReason"),              Config.WhitelistKickReason);
		Config.bWhitelistEnabled                = GetIniBoolOrDefault  (BackupFile, TEXT("WhitelistEnabled"),                Config.bWhitelistEnabled);
		Config.bBanSystemEnabled                = GetIniBoolOrDefault  (BackupFile, TEXT("BanSystemEnabled"),                Config.bBanSystemEnabled);
		Config.BanCommandPrefix                 = GetIniStringOrDefault(BackupFile, TEXT("BanCommandPrefix"),                 Config.BanCommandPrefix);
		Config.BanChannelId                     = GetIniStringOrDefault(BackupFile, TEXT("BanChannelId"),                     Config.BanChannelId);
		Config.bBanCommandsEnabled              = GetIniBoolOrDefault  (BackupFile, TEXT("BanCommandsEnabled"),              Config.bBanCommandsEnabled);
		Config.BanKickDiscordMessage            = GetIniStringOrDefault(BackupFile, TEXT("BanKickDiscordMessage"),            Config.BanKickDiscordMessage);
		Config.BanKickReason                    = GetIniStringOrFallback(BackupFile, TEXT("BanKickReason"),                    Config.BanKickReason);
		Config.InGameWhitelistCommandPrefix     = GetIniStringOrDefault(BackupFile, TEXT("InGameWhitelistCommandPrefix"),     Config.InGameWhitelistCommandPrefix);
		Config.InGameBanCommandPrefix           = GetIniStringOrDefault(BackupFile, TEXT("InGameBanCommandPrefix"),           Config.InGameBanCommandPrefix);

		// Only log the "restored from backup" message when credentials were
		// actually recovered (i.e. previously blank in primary but now non-empty
		// from the backup). Avoid a misleading message when the backup also has
		// blank credentials (e.g. first server start before credentials are set).
		const bool bRestoredToken   = !bHadToken   && !Config.BotToken.IsEmpty();
		const bool bRestoredChannel = !bHadChannel && !Config.ChannelId.IsEmpty();
		if (bRestoredToken || bRestoredChannel)
		{
			UE_LOG(LogTemp, Log,
			       TEXT("DiscordBridge: Credentials not set in primary config '%s'. "
			            "Loaded from backup at '%s'. "
			            "Copy your BotToken and ChannelId back into the primary config "
			            "to silence this message."),
			       *ModFilePath, *BackupFilePath);
		}
	}

	// ── Step 2b: overlay settings from optional separate config files ────────────
	// If DefaultDiscordBridgeWhitelist.ini / DefaultDiscordBridgeBan.ini exist
	// in the mod's Config folder, any setting defined there overrides the value
	// loaded from the primary config or the backup.
	//
	// Each separate file has its own dedicated backup in Saved/Config/:
	//   DiscordBridgeWhitelist.ini  – backed up (and always maintained) by Step 3
	//   DiscordBridgeBan.ini         – backed up (and always maintained) by Step 3
	//
	// If a mod update resets a separate file to the all-commented-out shipped
	// template, Step 3's backup is restored here automatically so operator
	// settings survive upgrades.  Step 3 writes these dedicated backups on every
	// server start (whether settings came from the separate files or the main
	// config), so they are always current and always separate from DiscordBridge.ini.
	{
		// ── Whitelist ─────────────────────────────────────────────────────────
		const FString WhitelistFilePath       = GetWhitelistConfigFilePath();
		const FString WhitelistBackupFilePath = GetWhitelistBackupConfigFilePath();

		auto ApplyWhitelistSettings = [&](const FConfigFile& Cfg)
		{
			Config.bWhitelistEnabled            = GetIniBoolOrDefault  (Cfg, TEXT("WhitelistEnabled"),            Config.bWhitelistEnabled);
			Config.WhitelistCommandRoleId       = GetIniStringOrDefault(Cfg, TEXT("WhitelistCommandRoleId"),       Config.WhitelistCommandRoleId);
			Config.WhitelistCommandPrefix       = GetIniStringOrDefault(Cfg, TEXT("WhitelistCommandPrefix"),       Config.WhitelistCommandPrefix);
			Config.WhitelistRoleId              = GetIniStringOrDefault(Cfg, TEXT("WhitelistRoleId"),              Config.WhitelistRoleId);
			Config.WhitelistChannelId           = GetIniStringOrDefault(Cfg, TEXT("WhitelistChannelId"),           Config.WhitelistChannelId);
			Config.WhitelistKickDiscordMessage  = GetIniStringOrDefault(Cfg, TEXT("WhitelistKickDiscordMessage"),  Config.WhitelistKickDiscordMessage);
			Config.WhitelistKickReason          = GetIniStringOrFallback(Cfg, TEXT("WhitelistKickReason"),         Config.WhitelistKickReason);
			Config.InGameWhitelistCommandPrefix = GetIniStringOrDefault(Cfg, TEXT("InGameWhitelistCommandPrefix"), Config.InGameWhitelistCommandPrefix);
		};

		if (PlatformFile.FileExists(*WhitelistFilePath))
		{
			// Use the presence of ';' in the raw file as the indicator of whether
			// this file still has its comment lines.  Alpakit strips every line that
			// begins with ';' during packaging, so a file with no ';' characters has
			// been packaged and stripped.  This is more reliable than checking for
			// active keys, because the shipped file may contain active default values
			// (e.g. WhitelistEnabled=False) that would otherwise be misread as
			// user-configured settings.
			FString WhitelistFileRaw;
			FFileHelper::LoadFileToString(WhitelistFileRaw, *WhitelistFilePath);
			const bool bWhitelistFileHasComments = WhitelistFileRaw.Contains(TEXT(";"));

			if (!bWhitelistFileHasComments)
			{
				// File has no ';' characters – Alpakit stripped comment lines.
				if (PlatformFile.FileExists(*WhitelistBackupFilePath))
				{
					// User had previously configured settings → restore from backup.
					FString BackupRaw;
					if (FFileHelper::LoadFileToString(BackupRaw, *WhitelistBackupFilePath) &&
					    FFileHelper::SaveStringToFile(BackupRaw, *WhitelistFilePath))
					{
						UE_LOG(LogTemp, Warning,
						       TEXT("DiscordBridge: Whitelist config was reset by a mod update. "
						            "Restored from backup at '%s'."), *WhitelistBackupFilePath);
						FConfigFile RestoredFile;
						RestoredFile.Read(WhitelistFilePath);
						ApplyWhitelistSettings(RestoredFile);
						UE_LOG(LogTemp, Log,
						       TEXT("DiscordBridge: Applied restored whitelist settings."));
					}
					else
					{
						UE_LOG(LogTemp, Warning,
						       TEXT("DiscordBridge: Whitelist config was reset but could not restore from '%s'."),
						       *WhitelistBackupFilePath);
					}
				}
				else
				{
					// Fresh install – write annotated template so operators see setup instructions.
					UE_LOG(LogTemp, Log,
					       TEXT("DiscordBridge: Whitelist config at '%s' has no comments "
					            "(stripped during packaging). "
					            "Rewriting with annotated template."), *WhitelistFilePath);

				const FString WhitelistDefaultContent =
					TEXT("[DiscordBridge]\n")
					TEXT("; DiscordBridge - Whitelist Configuration (Optional Override File)\n")
					TEXT("; ================================================================\n")
					TEXT("; 1. Remove the leading ';' from each setting you want to enable below.\n")
					TEXT("; 2. Fill in the value after the '='.\n")
					TEXT("; 3. Restart the server. Settings here take priority over built-in defaults.\n")
					TEXT("; Backup: <ServerRoot>/FactoryGame/Saved/Config/DiscordBridgeWhitelist.ini (auto-saved)\n")
					TEXT(";   The mod writes a backup of your whitelist settings here automatically so they\n")
					TEXT(";   survive mod updates that reset this file. The backup is only written when at\n")
					TEXT(";   least one setting below is uncommented (active).\n")
					TEXT("; All other settings (connection, chat, ban system, etc.) are in DefaultDiscordBridge.ini.\n")
					TEXT("\n")
					TEXT("; -- WHITELIST ----------------------------------------------------------------\n")
					TEXT("; Controls the built-in server whitelist, manageable via Discord commands.\n")
					TEXT("; Whitelist entries are stored in <ServerRoot>/FactoryGame/Saved/ServerWhitelist.json\n")
					TEXT("; and persist across server restarts automatically.\n")
					TEXT("; Whitelist and ban system are INDEPENDENT - use either, both, or neither:\n")
					TEXT(";   Whitelist only:  WhitelistEnabled=True,  BanSystemEnabled=False\n")
					TEXT(";   Ban only:        WhitelistEnabled=False, BanSystemEnabled=True  (default)\n")
					TEXT(";   Both:            WhitelistEnabled=True,  BanSystemEnabled=True\n")
					TEXT(";   Neither:         WhitelistEnabled=False, BanSystemEnabled=False\n")
					TEXT("\n")
					TEXT("; Controls whether the whitelist is active when the server starts.\n")
					TEXT("; When True, only players listed in ServerWhitelist.json (or who hold the\n")
					TEXT("; WhitelistRoleId Discord role, if configured) are allowed to join.\n")
					TEXT("; When False, all players can join regardless of the whitelist.\n")
					TEXT("; This value is applied on EVERY server restart - it is the authoritative\n")
					TEXT("; toggle. Note: !whitelist on / !whitelist off Discord commands update the\n")
					TEXT("; in-memory state for the current session only and do not override this value.\n")
					TEXT("; Default: False\n")
					TEXT(";WhitelistEnabled=False\n")
					TEXT("\n")
					TEXT("; Snowflake ID of the Discord role whose members are allowed to run !whitelist\n")
					TEXT("; commands. When set, ONLY members who hold this role can issue !whitelist\n")
					TEXT("; commands in the bridged Discord channel. When left empty, !whitelist commands\n")
					TEXT("; are disabled for all Discord users (deny-by-default) until a role ID is provided.\n")
					TEXT("; IMPORTANT: holding this role does NOT grant access to the game server. Role\n")
					TEXT("; holders are still subject to the normal whitelist and ban checks when joining.\n")
					TEXT("; How to get the role ID: Discord Settings -> Advanced -> Developer Mode, then\n")
					TEXT("; right-click the role in Server Settings -> Roles -> Copy Role ID.\n")
					TEXT("; Example: WhitelistCommandRoleId=123456789012345678\n")
					TEXT(";WhitelistCommandRoleId=\n")
					TEXT("\n")
					TEXT("; Prefix that triggers whitelist management commands when typed in the bridged\n")
					TEXT("; Discord channel. Set to an empty string to disable Discord-based whitelist\n")
					TEXT("; management entirely.\n")
					TEXT("; Supported commands (type in the bridged Discord channel):\n")
					TEXT(";   !whitelist on                       - enable the whitelist\n")
					TEXT(";   !whitelist off                      - disable the whitelist (all players can join)\n")
					TEXT(";   !whitelist add <name>               - add a player by in-game name\n")
					TEXT(";   !whitelist remove <name>            - remove a player by in-game name\n")
					TEXT(";   !whitelist list                     - list all whitelisted players\n")
					TEXT(";   !whitelist status                   - show current whitelist and ban system state\n")
					TEXT(";   !whitelist role add <discord_id>    - grant the WhitelistRoleId role to a Discord user\n")
					TEXT(";   !whitelist role remove <discord_id> - revoke the WhitelistRoleId role from a Discord user\n")
					TEXT("; Default: !whitelist\n")
					TEXT(";WhitelistCommandPrefix=!whitelist\n")
					TEXT("\n")
					TEXT("; Snowflake ID of the Discord role used to identify whitelisted members.\n")
					TEXT("; Leave empty to disable Discord role integration.\n")
					TEXT("; How to get the role ID: Discord Settings -> Advanced -> Developer Mode, then\n")
					TEXT("; right-click the role in Server Settings -> Roles -> Copy Role ID.\n")
					TEXT("; Example: WhitelistRoleId=111222333444555666\n")
					TEXT(";WhitelistRoleId=\n")
					TEXT("\n")
					TEXT("; Snowflake ID of a dedicated Discord channel for whitelisted members.\n")
					TEXT("; Leave empty to disable the whitelist-only channel.\n")
					TEXT("; How to get the channel ID: right-click the channel in Discord with Developer\n")
					TEXT("; Mode enabled -> Copy Channel ID.\n")
					TEXT("; Example: WhitelistChannelId=222333444555666777\n")
					TEXT(";WhitelistChannelId=\n")
					TEXT("\n")
					TEXT("; Message posted to the main Discord channel whenever a non-whitelisted player\n")
					TEXT("; tries to join and is kicked. Leave empty (delete the text after the =) to\n")
					TEXT("; disable this notification.\n")
					TEXT("; Available placeholder:\n")
					TEXT(";   %PlayerName%  - the in-game name of the player who was kicked\n")
					TEXT("; Example: WhitelistKickDiscordMessage=:no_entry: **%PlayerName%** is not whitelisted and was removed.\n")
					TEXT(";WhitelistKickDiscordMessage=:boot: **%PlayerName%** tried to join but is not on the whitelist and was kicked.\n")
					TEXT("\n")
					TEXT("; Text shown in-game to the player in the disconnected / kicked screen when\n")
					TEXT("; they are kicked because they are not on the whitelist.\n")
					TEXT("; Default: You are not on this server's whitelist. Contact the server admin to be added.\n")
					TEXT("; Example: WhitelistKickReason=You are not whitelisted. DM an admin on Discord to request access.\n")
					TEXT(";WhitelistKickReason=\n")
					TEXT("\n")
					TEXT("; Prefix that triggers whitelist management commands when typed in the\n")
					TEXT("; Satisfactory in-game chat. Lets server admins manage the whitelist from\n")
					TEXT("; inside the game without needing Discord. Set to an empty string to disable\n")
					TEXT("; in-game whitelist commands.\n")
					TEXT("; Supported commands (type in the Satisfactory in-game chat):\n")
					TEXT(";   !whitelist on            - enable the whitelist\n")
					TEXT(";   !whitelist off           - disable the whitelist (all players can join)\n")
					TEXT(";   !whitelist add <name>    - add a player by in-game name\n")
					TEXT(";   !whitelist remove <name> - remove a player by in-game name\n")
					TEXT(";   !whitelist list          - list all whitelisted players\n")
					TEXT(";   !whitelist status        - show current whitelist and ban system state\n")
					TEXT("; Note: in-game whitelist commands do not support role management\n")
					TEXT("; (!whitelist role add/remove), which is available from Discord only.\n")
					TEXT("; Default: !whitelist\n")
					TEXT(";InGameWhitelistCommandPrefix=!whitelist\n");

				if (FFileHelper::SaveStringToFile(WhitelistDefaultContent, *WhitelistFilePath))
				{
					UE_LOG(LogTemp, Log,
					       TEXT("DiscordBridge: Wrote annotated whitelist template to '%s'. "
					            "Uncomment and fill in settings you need, then restart the server."),
					       *WhitelistFilePath);
				}
				else
				{
					UE_LOG(LogTemp, Warning,
					       TEXT("DiscordBridge: Could not write annotated whitelist template to '%s'."),
					       *WhitelistFilePath);
				}
				}  // end fresh-install branch
			}  // end !bWhitelistFileHasComments
			else
			{
				// File has ';' comment characters – it is an annotated template written
				// by the mod at runtime, or the developer's unpackaged source copy.
				// Check for user-set (uncommented) keys and apply them.
				FConfigFile WhitelistConfigFile;
				WhitelistConfigFile.Read(WhitelistFilePath);

				FString TmpCheck;
				const bool bWhitelistHasUserSettings =
					WhitelistConfigFile.GetString(ConfigSection, TEXT("WhitelistEnabled"),             TmpCheck) ||
					WhitelistConfigFile.GetString(ConfigSection, TEXT("WhitelistCommandRoleId"),       TmpCheck) ||
					WhitelistConfigFile.GetString(ConfigSection, TEXT("WhitelistCommandPrefix"),       TmpCheck) ||
					WhitelistConfigFile.GetString(ConfigSection, TEXT("WhitelistRoleId"),              TmpCheck) ||
					WhitelistConfigFile.GetString(ConfigSection, TEXT("WhitelistChannelId"),           TmpCheck) ||
					WhitelistConfigFile.GetString(ConfigSection, TEXT("WhitelistKickDiscordMessage"),  TmpCheck) ||
					WhitelistConfigFile.GetString(ConfigSection, TEXT("WhitelistKickReason"),          TmpCheck) ||
					WhitelistConfigFile.GetString(ConfigSection, TEXT("InGameWhitelistCommandPrefix"), TmpCheck);

				if (bWhitelistHasUserSettings)
				{
					ApplyWhitelistSettings(WhitelistConfigFile);
					UE_LOG(LogTemp, Log,
					       TEXT("DiscordBridge: Applied whitelist overrides from '%s'."), *WhitelistFilePath);

					// Back up verbatim to Saved/Config/ so settings survive mod updates.
					PlatformFile.CreateDirectoryTree(*FPaths::GetPath(WhitelistBackupFilePath));
					FString RawContent;
					if (FFileHelper::LoadFileToString(RawContent, *WhitelistFilePath) &&
					    FFileHelper::SaveStringToFile(RawContent, *WhitelistBackupFilePath))
					{
						UE_LOG(LogTemp, Log,
						       TEXT("DiscordBridge: Backed up whitelist config to '%s'."), *WhitelistBackupFilePath);
					}
					else
					{
						UE_LOG(LogTemp, Warning,
						       TEXT("DiscordBridge: Could not back up whitelist config to '%s'."), *WhitelistBackupFilePath);
					}
				}
				// else: annotated template present but no settings configured yet – use hardcoded defaults.
			}
		}

		// ── Ban ───────────────────────────────────────────────────────────────
		const FString BanFilePath       = GetBanConfigFilePath();
		const FString BanBackupFilePath = GetBanBackupConfigFilePath();

		auto ApplyBanSettings = [&](const FConfigFile& Cfg)
		{
			Config.bBanSystemEnabled      = GetIniBoolOrDefault  (Cfg, TEXT("BanSystemEnabled"),      Config.bBanSystemEnabled);
			Config.BanCommandRoleId       = GetIniStringOrDefault(Cfg, TEXT("BanCommandRoleId"),       Config.BanCommandRoleId);
			Config.BanCommandPrefix       = GetIniStringOrDefault(Cfg, TEXT("BanCommandPrefix"),       Config.BanCommandPrefix);
			Config.BanChannelId           = GetIniStringOrDefault(Cfg, TEXT("BanChannelId"),           Config.BanChannelId);
			Config.bBanCommandsEnabled    = GetIniBoolOrDefault  (Cfg, TEXT("BanCommandsEnabled"),     Config.bBanCommandsEnabled);
			Config.BanKickDiscordMessage  = GetIniStringOrDefault(Cfg, TEXT("BanKickDiscordMessage"),  Config.BanKickDiscordMessage);
			Config.BanKickReason          = GetIniStringOrFallback(Cfg, TEXT("BanKickReason"),         Config.BanKickReason);
			Config.InGameBanCommandPrefix = GetIniStringOrDefault(Cfg, TEXT("InGameBanCommandPrefix"), Config.InGameBanCommandPrefix);
		};

		if (PlatformFile.FileExists(*BanFilePath))
		{
			// Use the presence of ';' in the raw file as the indicator of whether
			// this file still has its comment lines.  Alpakit strips every line that
			// begins with ';' during packaging, so a file with no ';' characters has
			// been packaged and stripped.  This is more reliable than checking for
			// active keys, because the shipped file may contain active default values
			// (e.g. BanSystemEnabled=True) that would otherwise be misread as
			// user-configured settings.
			FString BanFileRaw;
			FFileHelper::LoadFileToString(BanFileRaw, *BanFilePath);
			const bool bBanFileHasComments = BanFileRaw.Contains(TEXT(";"));

			if (!bBanFileHasComments)
			{
				// File has no ';' characters – Alpakit stripped comment lines.
				if (PlatformFile.FileExists(*BanBackupFilePath))
				{
					// User had previously configured settings → restore from backup.
					FString BackupRaw;
					if (FFileHelper::LoadFileToString(BackupRaw, *BanBackupFilePath) &&
					    FFileHelper::SaveStringToFile(BackupRaw, *BanFilePath))
					{
						UE_LOG(LogTemp, Warning,
						       TEXT("DiscordBridge: Ban config was reset by a mod update. "
						            "Restored from backup at '%s'."), *BanBackupFilePath);
						FConfigFile RestoredFile;
						RestoredFile.Read(BanFilePath);
						ApplyBanSettings(RestoredFile);
						UE_LOG(LogTemp, Log,
						       TEXT("DiscordBridge: Applied restored ban settings."));
					}
					else
					{
						UE_LOG(LogTemp, Warning,
						       TEXT("DiscordBridge: Ban config was reset but could not restore from '%s'."),
						       *BanBackupFilePath);
					}
				}
				else
				{
					// Fresh install – write annotated template so operators see setup instructions.
					UE_LOG(LogTemp, Log,
					       TEXT("DiscordBridge: Ban config at '%s' has no comments "
					            "(stripped during packaging). "
					            "Rewriting with annotated template."), *BanFilePath);

				const FString BanDefaultContent =
					TEXT("[DiscordBridge]\n")
					TEXT("; DiscordBridge - Ban System Configuration (Optional Override File)\n")
					TEXT("; =================================================================\n")
					TEXT("; 1. Remove the leading ';' from each setting you want to enable below.\n")
					TEXT("; 2. Fill in the value after the '='.\n")
					TEXT("; 3. Restart the server. Settings here take priority over built-in defaults.\n")
					TEXT("; Backup: <ServerRoot>/FactoryGame/Saved/Config/DiscordBridgeBan.ini (auto-saved)\n")
					TEXT(";   The mod writes a backup of your ban settings here automatically so they\n")
					TEXT(";   survive mod updates that reset this file. The backup is only written when at\n")
					TEXT(";   least one setting below is uncommented (active).\n")
					TEXT("; All other settings (connection, chat, whitelist, etc.) are in DefaultDiscordBridge.ini.\n")
					TEXT("\n")
					TEXT("; -- BAN SYSTEM ---------------------------------------------------------------\n")
					TEXT("; Controls the built-in player ban system, manageable via Discord commands.\n")
					TEXT("; Bans are stored in <ServerRoot>/FactoryGame/Saved/ServerBanlist.json and\n")
					TEXT("; persist across server restarts automatically.\n")
					TEXT("; Ban system and whitelist are INDEPENDENT (see DefaultDiscordBridgeWhitelist.ini).\n")
					TEXT("; You can use either, both, or neither:\n")
					TEXT(";   Ban only:  BanSystemEnabled=True,  WhitelistEnabled=False  (default)\n")
					TEXT(";   Both:      BanSystemEnabled=True,  WhitelistEnabled=True\n")
					TEXT(";   Neither:   BanSystemEnabled=False, WhitelistEnabled=False\n")
					TEXT("\n")
					TEXT("; Controls whether the ban system is active when the server starts.\n")
					TEXT("; When True, players listed in ServerBanlist.json are kicked on join.\n")
					TEXT("; When False, banned players can join freely (bans are not enforced).\n")
					TEXT("; This value is applied on EVERY server restart - it is the authoritative\n")
					TEXT("; toggle. Note: !ban on / !ban off Discord commands update the in-memory state\n")
					TEXT("; for the current session only and do not override this config value.\n")
					TEXT("; Default: True (ban list is enforced on every server start)\n")
					TEXT(";BanSystemEnabled=True\n")
					TEXT("\n")
					TEXT("; Snowflake ID of the Discord role whose members are allowed to run !ban commands.\n")
					TEXT("; When set, ONLY members who hold this role can issue !ban commands in the bridged\n")
					TEXT("; Discord channel. When left empty, !ban commands are disabled for all Discord\n")
					TEXT("; users (deny-by-default) until a role ID is provided.\n")
					TEXT("; How to get the role ID: Discord Settings -> Advanced -> Developer Mode, then\n")
					TEXT("; right-click the role in Server Settings -> Roles -> Copy Role ID.\n")
					TEXT("; Example: BanCommandRoleId=987654321098765432\n")
					TEXT(";BanCommandRoleId=\n")
					TEXT("\n")
					TEXT("; Prefix that triggers ban management commands when typed in the bridged Discord\n")
					TEXT("; channel. Set to an empty string to disable Discord-based ban management.\n")
					TEXT("; Supported commands (type in the bridged Discord channel):\n")
					TEXT(";   !ban add <name>               - ban a player by in-game name\n")
					TEXT(";   !ban remove <name>            - unban a player by in-game name\n")
					TEXT(";   !ban list                     - list all banned players\n")
					TEXT(";   !ban status                   - show current ban system and whitelist state\n")
					TEXT(";   !ban on                       - enable the ban system for this session\n")
					TEXT(";   !ban off                      - disable the ban system for this session\n")
					TEXT(";   !ban role add <discord_id>    - grant the BanCommandRoleId role to a Discord user\n")
					TEXT(";   !ban role remove <discord_id> - revoke the BanCommandRoleId role from a Discord user\n")
					TEXT("; Default: !ban\n")
					TEXT(";BanCommandPrefix=!ban\n")
					TEXT("\n")
					TEXT("; Snowflake ID of a dedicated Discord channel for ban management.\n")
					TEXT("; Leave empty to disable the ban-only channel.\n")
					TEXT("; How to get the channel ID: right-click the channel in Discord with Developer\n")
					TEXT("; Mode enabled -> Copy Channel ID.\n")
					TEXT("; Example: BanChannelId=567890123456789012\n")
					TEXT(";BanChannelId=\n")
					TEXT("\n")
					TEXT("; Master on/off switch for the ban command interface.\n")
					TEXT("; When True (default), !ban Discord and in-game commands are accepted (still\n")
					TEXT("; gated by BanCommandRoleId). When False, all !ban commands are silently\n")
					TEXT("; ignored while existing bans are STILL enforced on join (BanSystemEnabled is\n")
					TEXT("; unaffected).\n")
					TEXT("; Default: True\n")
					TEXT(";BanCommandsEnabled=True\n")
					TEXT("\n")
					TEXT("; Message posted to the main Discord channel whenever a banned player tries to\n")
					TEXT("; join and is kicked. Leave empty (delete the text after the =) to disable\n")
					TEXT("; this notification.\n")
					TEXT("; Available placeholder:\n")
					TEXT(";   %PlayerName%  - the in-game name of the banned player who was kicked\n")
					TEXT("; Example: BanKickDiscordMessage=:no_entry: **%PlayerName%** is banned and was removed.\n")
					TEXT(";BanKickDiscordMessage=:hammer: **%PlayerName%** is banned from this server and was kicked.\n")
					TEXT("\n")
					TEXT("; Text shown in-game to the player in the disconnected / kicked screen when\n")
					TEXT("; they are kicked because they are on the ban list.\n")
					TEXT("; Default: You are banned from this server.\n")
					TEXT("; Example: BanKickReason=You have been banned. Contact the server admin to appeal.\n")
					TEXT(";BanKickReason=\n")
					TEXT("\n")
					TEXT("; Prefix that triggers ban management commands when typed in the Satisfactory\n")
					TEXT("; in-game chat. Lets server admins manage bans from inside the game without\n")
					TEXT("; needing Discord. Set to an empty string to disable in-game ban commands.\n")
					TEXT("; Supported commands (type in the Satisfactory in-game chat):\n")
					TEXT(";   !ban add <name>    - ban a player by in-game name\n")
					TEXT(";   !ban remove <name> - unban a player by in-game name\n")
					TEXT(";   !ban list          - list all banned players\n")
					TEXT(";   !ban status        - show current ban system and whitelist state\n")
					TEXT(";   !ban on            - enable the ban system for this session\n")
					TEXT(";   !ban off           - disable the ban system for this session\n")
					TEXT("; Note: in-game ban commands do not support role management (!ban role add/remove),\n")
					TEXT("; which is available from Discord only.\n")
					TEXT("; Default: !ban\n")
					TEXT(";InGameBanCommandPrefix=!ban\n");

				if (FFileHelper::SaveStringToFile(BanDefaultContent, *BanFilePath))
				{
					UE_LOG(LogTemp, Log,
					       TEXT("DiscordBridge: Wrote annotated ban template to '%s'. "
					            "Uncomment and fill in settings you need, then restart the server."),
					       *BanFilePath);
				}
				else
				{
					UE_LOG(LogTemp, Warning,
					       TEXT("DiscordBridge: Could not write annotated ban template to '%s'."),
					       *BanFilePath);
				}
				}  // end fresh-install branch
			}  // end !bBanFileHasComments
			else
			{
				// File has ';' comment characters – it is an annotated template written
				// by the mod at runtime, or the developer's unpackaged source copy.
				// Check for user-set (uncommented) keys and apply them.
				FConfigFile BanConfigFile;
				BanConfigFile.Read(BanFilePath);

				FString TmpCheck;
				const bool bBanHasUserSettings =
					BanConfigFile.GetString(ConfigSection, TEXT("BanSystemEnabled"),      TmpCheck) ||
					BanConfigFile.GetString(ConfigSection, TEXT("BanCommandRoleId"),      TmpCheck) ||
					BanConfigFile.GetString(ConfigSection, TEXT("BanCommandPrefix"),      TmpCheck) ||
					BanConfigFile.GetString(ConfigSection, TEXT("BanChannelId"),          TmpCheck) ||
					BanConfigFile.GetString(ConfigSection, TEXT("BanCommandsEnabled"),    TmpCheck) ||
					BanConfigFile.GetString(ConfigSection, TEXT("BanKickDiscordMessage"), TmpCheck) ||
					BanConfigFile.GetString(ConfigSection, TEXT("BanKickReason"),         TmpCheck) ||
					BanConfigFile.GetString(ConfigSection, TEXT("InGameBanCommandPrefix"),TmpCheck);

				if (bBanHasUserSettings)
				{
					ApplyBanSettings(BanConfigFile);
					UE_LOG(LogTemp, Log,
					       TEXT("DiscordBridge: Applied ban overrides from '%s'."), *BanFilePath);

					// Back up verbatim to Saved/Config/ so settings survive mod updates.
					PlatformFile.CreateDirectoryTree(*FPaths::GetPath(BanBackupFilePath));
					FString RawContent;
					if (FFileHelper::LoadFileToString(RawContent, *BanFilePath) &&
					    FFileHelper::SaveStringToFile(RawContent, *BanBackupFilePath))
					{
						UE_LOG(LogTemp, Log,
						       TEXT("DiscordBridge: Backed up ban config to '%s'."), *BanBackupFilePath);
					}
					else
					{
						UE_LOG(LogTemp, Warning,
						       TEXT("DiscordBridge: Could not back up ban config to '%s'."), *BanBackupFilePath);
					}
				}
				// else: annotated template present but no settings configured yet – use hardcoded defaults.
			}
		}
	}

	// ── Step 3: keep backups up to date ──────────────────────────────────────
	// Write up-to-date backups on every server start so all settings survive
	// the next mod update.  The backup is written even when BotToken/ChannelId
	// are still blank (e.g. fresh Alpakit install before credentials are set):
	// this ensures the file exists from the very first start and that settings
	// such as message formats are preserved regardless of credential state.
	//
	// Each logical group of settings has its OWN dedicated backup file in
	// Saved/Config/ so settings are never mixed across files:
	//   DiscordBridge.ini         – connection + chat + presence  (this file)
	//   DiscordBridgeWhitelist.ini – all whitelist settings        (always separate)
	//   DiscordBridgeBan.ini       – all ban settings              (always separate)
	//
	// Whitelist and ban settings are NEVER written to DiscordBridge.ini.
	// They always have their own dedicated backup files (written below), which
	// are restored automatically by Step 2b when a mod update resets their
	// primary files (DefaultDiscordBridgeWhitelist.ini / DefaultDiscordBridgeBan.ini).
	{
		// ── DiscordBridge.ini: connection, chat and presence only ─────────────
		FString BackupContent = FString::Printf(
			TEXT("[DiscordBridge]\n")
			TEXT("; Auto-generated backup of %s\n")
			TEXT("; This file is read automatically when the primary config is missing credentials.\n")
			TEXT("; Whitelist and ban settings are in their own backup files:\n")
			TEXT(";   DiscordBridgeWhitelist.ini  – whitelist settings\n")
			TEXT(";   DiscordBridgeBan.ini         – ban system settings\n")
			TEXT("BotToken=%s\n")
			TEXT("ChannelId=%s\n")
			TEXT("ServerName=%s\n")
			TEXT("GameToDiscordFormat=%s\n")
			TEXT("DiscordToGameFormat=%s\n")
			TEXT("IgnoreBotMessages=%s\n")
			TEXT("ServerOnlineMessage=%s\n")
			TEXT("ServerOfflineMessage=%s\n")
			TEXT("ShowPlayerCountInPresence=%s\n")
			TEXT("PlayerCountPresenceFormat=%s\n")
			TEXT("PlayerCountUpdateIntervalSeconds=%s\n")
			TEXT("PlayerCountActivityType=%d\n"),
			*ModFilePath,
			*Config.BotToken,
			*Config.ChannelId,
			*Config.ServerName,
			*Config.GameToDiscordFormat,
			*Config.DiscordToGameFormat,
			Config.bIgnoreBotMessages ? TEXT("True") : TEXT("False"),
			*Config.ServerOnlineMessage,
			*Config.ServerOfflineMessage,
			Config.bShowPlayerCountInPresence ? TEXT("True") : TEXT("False"),
			*Config.PlayerCountPresenceFormat,
			*FString::SanitizeFloat(Config.PlayerCountUpdateIntervalSeconds),
			Config.PlayerCountActivityType);

		PlatformFile.CreateDirectoryTree(*FPaths::GetPath(BackupFilePath));

		if (FFileHelper::SaveStringToFile(BackupContent, *BackupFilePath))
		{
			if (Config.BotToken.IsEmpty())
			{
				UE_LOG(LogTemp, Log,
				       TEXT("DiscordBridge: Wrote backup config at '%s' (credentials not yet configured)."),
				       *BackupFilePath);
			}
			else
			{
				UE_LOG(LogTemp, Log,
				       TEXT("DiscordBridge: Updated backup config at '%s'."), *BackupFilePath);
			}
		}
		else
		{
			UE_LOG(LogTemp, Warning,
			       TEXT("DiscordBridge: Could not write backup config to '%s'."),
			       *BackupFilePath);
		}

		// ── DiscordBridgeWhitelist.ini: all whitelist settings ────────────────
		// Always maintained as a dedicated backup, regardless of whether settings
		// came from DefaultDiscordBridgeWhitelist.ini or DefaultDiscordBridge.ini.
		// This ensures whitelist settings are never mixed into DiscordBridge.ini
		// and always have their own restorable backup for mod updates.
		{
			const FString WLBackupPath = GetWhitelistBackupConfigFilePath();
			const FString WLBackupContent = FString::Printf(
				TEXT("[DiscordBridge]\n")
				TEXT("; Auto-generated backup of whitelist settings\n")
				TEXT("; Restored automatically when DefaultDiscordBridgeWhitelist.ini is reset by a mod update.\n")
				TEXT("WhitelistEnabled=%s\n")
				TEXT("WhitelistCommandRoleId=%s\n")
				TEXT("WhitelistCommandPrefix=%s\n")
				TEXT("WhitelistRoleId=%s\n")
				TEXT("WhitelistChannelId=%s\n")
				TEXT("WhitelistKickDiscordMessage=%s\n")
				TEXT("WhitelistKickReason=%s\n")
				TEXT("InGameWhitelistCommandPrefix=%s\n"),
				Config.bWhitelistEnabled ? TEXT("True") : TEXT("False"),
				*Config.WhitelistCommandRoleId,
				*Config.WhitelistCommandPrefix,
				*Config.WhitelistRoleId,
				*Config.WhitelistChannelId,
				*Config.WhitelistKickDiscordMessage,
				*Config.WhitelistKickReason,
				*Config.InGameWhitelistCommandPrefix);

			PlatformFile.CreateDirectoryTree(*FPaths::GetPath(WLBackupPath));
			if (FFileHelper::SaveStringToFile(WLBackupContent, *WLBackupPath))
			{
				UE_LOG(LogTemp, Log,
				       TEXT("DiscordBridge: Updated whitelist backup at '%s'."), *WLBackupPath);
			}
			else
			{
				UE_LOG(LogTemp, Warning,
				       TEXT("DiscordBridge: Could not write whitelist backup to '%s'."), *WLBackupPath);
			}
		}

		// ── DiscordBridgeBan.ini: all ban settings ────────────────────────────
		// Always maintained as a dedicated backup, regardless of whether settings
		// came from DefaultDiscordBridgeBan.ini or DefaultDiscordBridge.ini.
		// This ensures ban settings are never mixed into DiscordBridge.ini
		// and always have their own restorable backup for mod updates.
		{
			const FString BanBackupPath = GetBanBackupConfigFilePath();
			const FString BanBackupContent = FString::Printf(
				TEXT("[DiscordBridge]\n")
				TEXT("; Auto-generated backup of ban system settings\n")
				TEXT("; Restored automatically when DefaultDiscordBridgeBan.ini is reset by a mod update.\n")
				TEXT("BanSystemEnabled=%s\n")
				TEXT("BanCommandRoleId=%s\n")
				TEXT("BanCommandPrefix=%s\n")
				TEXT("BanChannelId=%s\n")
				TEXT("BanCommandsEnabled=%s\n")
				TEXT("BanKickDiscordMessage=%s\n")
				TEXT("BanKickReason=%s\n")
				TEXT("InGameBanCommandPrefix=%s\n"),
				Config.bBanSystemEnabled ? TEXT("True") : TEXT("False"),
				*Config.BanCommandRoleId,
				*Config.BanCommandPrefix,
				*Config.BanChannelId,
				Config.bBanCommandsEnabled ? TEXT("True") : TEXT("False"),
				*Config.BanKickDiscordMessage,
				*Config.BanKickReason,
				*Config.InGameBanCommandPrefix);

			PlatformFile.CreateDirectoryTree(*FPaths::GetPath(BanBackupPath));
			if (FFileHelper::SaveStringToFile(BanBackupContent, *BanBackupPath))
			{
				UE_LOG(LogTemp, Log,
				       TEXT("DiscordBridge: Updated ban backup at '%s'."), *BanBackupPath);
			}
			else
			{
				UE_LOG(LogTemp, Warning,
				       TEXT("DiscordBridge: Could not write ban backup to '%s'."), *BanBackupPath);
			}
		}
	}

	return Config;
}
