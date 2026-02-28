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
		Config.DiscordSenderFormat  = GetIniStringOrFallback(ConfigFile, TEXT("DiscordSenderFormat"),  Config.DiscordSenderFormat);
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
			FString TmpVal;
			const bool bFileHasWhitelist = ConfigFile.GetString(ConfigSection, TEXT("WhitelistEnabled"), TmpVal);
			const bool bFileHasBan       = ConfigFile.GetString(ConfigSection, TEXT("BanSystemEnabled"), TmpVal);

			if (!bFileHasWhitelist || !bFileHasBan)
			{
				UE_LOG(LogTemp, Log,
				       TEXT("DiscordBridge: Config at '%s' is missing whitelist/ban settings "
				            "(older version detected). Appending new settings."),
				       *ModFilePath);

				FString ExistingContent;
				FFileHelper::LoadFileToString(ExistingContent, *ModFilePath);

				FString AppendContent;

				if (!bFileHasWhitelist)
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
						TEXT(";   Neither:          WhitelistEnabled=False, BanSystemEnabled=False  (default)\n")
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

				if (!bFileHasBan)
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
						TEXT("; Default: False.\n")
						TEXT("BanSystemEnabled=False\n")
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
			TEXT("; Format for Discord -> game (message body). Placeholders: %Username%, %Message%\n")
			TEXT("; Default: %Message%\n")
			TEXT("DiscordToGameFormat=\n")
			TEXT("; Format for Discord -> game (sender name column). Placeholders: %Username%\n")
			TEXT("; Default: [Discord] %Username%\n")
			TEXT("DiscordSenderFormat=\n")
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
			TEXT("; -- WHITELIST ----------------------------------------------------------------\n")
			TEXT("; Controls the built-in server whitelist, manageable via Discord commands.\n")
			TEXT("; Whitelist and ban system are INDEPENDENT - use either, both, or neither:\n")
			TEXT(";   Whitelist only:  WhitelistEnabled=True,  BanSystemEnabled=False\n")
			TEXT(";   Ban only:        WhitelistEnabled=False, BanSystemEnabled=True\n")
			TEXT(";   Both:            WhitelistEnabled=True,  BanSystemEnabled=True\n")
			TEXT(";   Neither:         WhitelistEnabled=False, BanSystemEnabled=False  (default)\n")
			TEXT("; Whether the whitelist is active. Applied on every server restart.\n")
			TEXT("; Default: False\n")
			TEXT("WhitelistEnabled=\n")
			TEXT("; Snowflake ID of the Discord role allowed to run !whitelist commands. Leave empty to disable.\n")
			TEXT("WhitelistCommandRoleId=\n")
			TEXT("; Prefix for whitelist commands in Discord. Set empty to disable.\n")
			TEXT("WhitelistCommandPrefix=!whitelist\n")
			TEXT("; Snowflake ID of the Discord role assigned to whitelisted members. Leave empty to disable.\n")
			TEXT("WhitelistRoleId=\n")
			TEXT("; Snowflake ID of a dedicated Discord channel for whitelisted members. Leave empty to disable.\n")
			TEXT("WhitelistChannelId=\n")
			TEXT("; Message posted to Discord when a non-whitelisted player is kicked. Placeholder: %PlayerName%\n")
			TEXT("WhitelistKickDiscordMessage=:boot: **%PlayerName%** tried to join but is not on the whitelist and was kicked.\n")
			TEXT("; Reason shown in-game to a kicked non-whitelisted player.\n")
			TEXT("; Default: You are not on this server's whitelist. Contact the server admin to be added.\n")
			TEXT("WhitelistKickReason=\n")
			TEXT("\n")
			TEXT("; -- BAN SYSTEM ---------------------------------------------------------------\n")
			TEXT("; Controls the built-in player ban system, manageable via Discord commands.\n")
			TEXT("; Bans are stored in <ServerRoot>/FactoryGame/Saved/ServerBanlist.json.\n")
			TEXT("; Ban system and whitelist are INDEPENDENT (see WHITELIST section above).\n")
			TEXT("; Whether the ban system is active on startup. Applied on every server restart.\n")
			TEXT("; Default: False\n")
			TEXT("BanSystemEnabled=\n")
			TEXT("; Snowflake ID of the Discord role allowed to run !ban commands. Leave empty to disable.\n")
			TEXT("BanCommandRoleId=\n")
			TEXT("; Prefix for ban commands in Discord. Set empty to disable.\n")
			TEXT("BanCommandPrefix=!ban\n")
			TEXT("; Snowflake ID of a dedicated Discord channel for ban management. Leave empty to disable.\n")
			TEXT("BanChannelId=\n")
			TEXT("; When True (default), !ban commands are enabled. When False, commands are disabled but bans are still enforced.\n")
			TEXT("; Default: True\n")
			TEXT("BanCommandsEnabled=\n")
			TEXT("; Message posted to Discord when a banned player is kicked. Placeholder: %PlayerName%\n")
			TEXT("BanKickDiscordMessage=:hammer: **%PlayerName%** is banned from this server and was kicked.\n")
			TEXT("; Reason shown in-game to a kicked banned player.\n")
			TEXT("; Default: You are banned from this server.\n")
			TEXT("BanKickReason=\n")
			TEXT("\n")
			TEXT("; -- IN-GAME COMMANDS ---------------------------------------------------------\n")
			TEXT("; Prefix for whitelist commands in in-game chat. Set empty to disable.\n")
			TEXT("InGameWhitelistCommandPrefix=!whitelist\n")
			TEXT("; Prefix for ban commands in in-game chat. Set empty to disable.\n")
			TEXT("InGameBanCommandPrefix=!ban\n");

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
		Config.GameToDiscordFormat  = GetIniStringOrFallback(BackupFile, TEXT("GameToDiscordFormat"),  Config.GameToDiscordFormat);
		Config.DiscordToGameFormat  = GetIniStringOrFallback(BackupFile, TEXT("DiscordToGameFormat"),  Config.DiscordToGameFormat);
		Config.DiscordSenderFormat  = GetIniStringOrFallback(BackupFile, TEXT("DiscordSenderFormat"),  Config.DiscordSenderFormat);
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
			TEXT("IgnoreBotMessages=%s\n")
			TEXT("ServerOnlineMessage=%s\n")
			TEXT("ServerOfflineMessage=%s\n")
			TEXT("ShowPlayerCountInPresence=%s\n")
			TEXT("PlayerCountPresenceFormat=%s\n")
			TEXT("PlayerCountUpdateIntervalSeconds=%s\n")
			TEXT("PlayerCountActivityType=%d\n")
			TEXT("WhitelistCommandRoleId=%s\n")
			TEXT("BanCommandRoleId=%s\n")
			TEXT("WhitelistEnabled=%s\n")
			TEXT("WhitelistCommandPrefix=%s\n")
			TEXT("WhitelistRoleId=%s\n")
			TEXT("WhitelistChannelId=%s\n")
			TEXT("WhitelistKickDiscordMessage=%s\n")
			TEXT("WhitelistKickReason=%s\n")
			TEXT("BanSystemEnabled=%s\n")
			TEXT("BanCommandPrefix=%s\n")
			TEXT("BanChannelId=%s\n")
			TEXT("BanCommandsEnabled=%s\n")
			TEXT("BanKickDiscordMessage=%s\n")
			TEXT("BanKickReason=%s\n")
			TEXT("InGameWhitelistCommandPrefix=%s\n")
			TEXT("InGameBanCommandPrefix=%s\n"),
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
			*Config.WhitelistCommandRoleId,
			*Config.BanCommandRoleId,
			Config.bWhitelistEnabled ? TEXT("True") : TEXT("False"),
			*Config.WhitelistCommandPrefix,
			*Config.WhitelistRoleId,
			*Config.WhitelistChannelId,
			*Config.WhitelistKickDiscordMessage,
			*Config.WhitelistKickReason,
			Config.bBanSystemEnabled ? TEXT("True") : TEXT("False"),
			*Config.BanCommandPrefix,
			*Config.BanChannelId,
			Config.bBanCommandsEnabled ? TEXT("True") : TEXT("False"),
			*Config.BanKickDiscordMessage,
			*Config.BanKickReason,
			*Config.InGameWhitelistCommandPrefix,
			*Config.InGameBanCommandPrefix);

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
