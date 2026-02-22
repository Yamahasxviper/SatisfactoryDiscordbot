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

		// Trim leading/trailing whitespace from credential fields to prevent
		// subtle mismatches when operators accidentally include spaces.
		Config.BotToken  = Config.BotToken.TrimStartAndEnd();
		Config.ChannelId = Config.ChannelId.TrimStartAndEnd();

		bLoadedFromMod = true;
		UE_LOG(LogTemp, Log, TEXT("DiscordBridge: Loaded config from %s"), *ModFilePath);
	}
	else
	{
		// Primary config missing – create it with defaults so the operator has
		// a ready-made file to fill in.
		UE_LOG(LogTemp, Log,
		       TEXT("DiscordBridge: Config file not found at '%s'. "
		            "Creating it with defaults."), *ModFilePath);

		const FString DefaultContent =
			TEXT("[DiscordBridge]\n")
			TEXT("; Discord bot token (Bot -> Token in the Developer Portal). Treat as a password.\n")
			TEXT("BotToken=\n")
			TEXT("; Snowflake ID of the Discord text channel to bridge with in-game chat.\n")
			TEXT("; Enable Developer Mode in Discord, right-click a channel, then \"Copy Channel ID\".\n")
			TEXT("ChannelId=\n")
			TEXT("; Display name for this server. Used as %ServerName% in GameToDiscordFormat.\n")
			TEXT("ServerName=\n")
			TEXT("; Format for game -> Discord messages. Placeholders: %ServerName%, %PlayerName%, %Message%.\n")
			TEXT("GameToDiscordFormat=**%PlayerName%**: %Message%\n")
			TEXT("; Format for Discord -> game messages. Placeholders: %Username%, %PlayerName% (alias for %Username%), %Message%.\n")
			TEXT("DiscordToGameFormat=%Message%\n")
			TEXT("; Format for the sender name shown in-game for Discord messages. Placeholders: %Username%, %PlayerName% (alias for %Username%).\n")
			TEXT("; Example (Satisfactory-branded): DiscordSenderFormat=[Satisfactory] %PlayerName%\n")
			TEXT("DiscordSenderFormat=[Discord] %Username%\n")
			TEXT("; When True, messages from bot accounts are ignored (prevents echo loops).\n")
			TEXT("bIgnoreBotMessages=True\n")
			TEXT("; Message posted to Discord when the server comes online. Leave empty to disable.\n")
			TEXT("ServerOnlineMessage=:green_circle: Server is now **online**!\n")
			TEXT("; Message posted to Discord when the server shuts down. Leave empty to disable.\n")
			TEXT("ServerOfflineMessage=:red_circle: Server is now **offline**.\n");

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
			TEXT("ServerOfflineMessage=%s\n"),
			*ModFilePath,
			*Config.BotToken,
			*Config.ChannelId,
			*Config.ServerName,
			*Config.GameToDiscordFormat,
			*Config.DiscordToGameFormat,
			*Config.DiscordSenderFormat,
			Config.bIgnoreBotMessages ? TEXT("True") : TEXT("False"),
			*Config.ServerOnlineMessage,
			*Config.ServerOfflineMessage);

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
