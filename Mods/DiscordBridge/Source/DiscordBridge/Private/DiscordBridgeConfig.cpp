// Copyright Coffee Stain Studios. All Rights Reserved.

#include "DiscordBridgeConfig.h"

#include "HAL/PlatformFileManager.h"
#include "Interfaces/IPluginManager.h"
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

FString FDiscordBridgeConfig::GetConfigFilePath()
{
	// Resolve the path to the plugin's own DefaultDiscordBridge.ini file.
	// On a deployed server this resolves to:
	//   <ServerRoot>/FactoryGame/Mods/DiscordBridge/Config/DefaultDiscordBridge.ini
	// This file is staged by Alpakit (via PluginSettings.ini → AdditionalNonUSFDirectories)
	// and is directly editable by the server operator.
	const TSharedPtr<IPlugin> Plugin = IPluginManager::Get().FindPlugin(TEXT("DiscordBridge"));
	if (Plugin.IsValid())
	{
		return FPaths::Combine(Plugin->GetBaseDir(), TEXT("Config"), TEXT("DefaultDiscordBridge.ini"));
	}

	// Fallback: use the SML convention so the mod still functions without a valid plugin entry.
	return FPaths::Combine(FPaths::ProjectDir(), TEXT("Configs"), TEXT("DefaultDiscordBridge.ini"));
}

FDiscordBridgeConfig FDiscordBridgeConfig::LoadOrCreate()
{
	FDiscordBridgeConfig Config;
	const FString FilePath = GetConfigFilePath();

	IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();

	if (PlatformFile.FileExists(*FilePath))
	{
		FConfigFile ConfigFile;
		ConfigFile.Read(FilePath);

		Config.BotToken             = GetIniStringOrDefault(ConfigFile, TEXT("BotToken"),             TEXT(""));
		Config.ChannelId            = GetIniStringOrDefault(ConfigFile, TEXT("ChannelId"),            TEXT(""));
		Config.GameToDiscordFormat  = GetIniStringOrDefault(ConfigFile, TEXT("GameToDiscordFormat"),  Config.GameToDiscordFormat);
		Config.DiscordToGameFormat  = GetIniStringOrDefault(ConfigFile, TEXT("DiscordToGameFormat"),  Config.DiscordToGameFormat);
		Config.bIgnoreBotMessages   = GetIniBoolOrDefault  (ConfigFile, TEXT("bIgnoreBotMessages"),   Config.bIgnoreBotMessages);
		Config.ServerOnlineMessage  = GetIniStringOrDefault(ConfigFile, TEXT("ServerOnlineMessage"),  Config.ServerOnlineMessage);
		Config.ServerOfflineMessage = GetIniStringOrDefault(ConfigFile, TEXT("ServerOfflineMessage"), Config.ServerOfflineMessage);

		UE_LOG(LogTemp, Log, TEXT("DiscordBridge: Loaded config from %s"), *FilePath);
	}
	else
	{
		// The config file is missing. Write a template with default values so the
		// server operator has a ready-made file to fill in.
		UE_LOG(LogTemp, Log,
		       TEXT("DiscordBridge: Config file not found at '%s'. "
		            "Creating it with defaults."), *FilePath);

		const FString DefaultContent =
			TEXT("[DiscordBridge]\n")
			TEXT("; Discord bot token (Bot -> Token in the Developer Portal). Treat as a password.\n")
			TEXT("BotToken=\n")
			TEXT("; Snowflake ID of the Discord text channel to bridge with in-game chat.\n")
			TEXT("; Enable Developer Mode in Discord, right-click a channel, then \"Copy Channel ID\".\n")
			TEXT("ChannelId=\n")
			TEXT("; Format for game -> Discord messages. Placeholders: {PlayerName}, {Message}.\n")
			TEXT("GameToDiscordFormat=**{PlayerName}**: {Message}\n")
			TEXT("; Format for Discord -> game messages. Placeholders: {Username}, {Message}.\n")
			TEXT("DiscordToGameFormat=[Discord] {Username}: {Message}\n")
			TEXT("; When True, messages from bot accounts are ignored (prevents echo loops).\n")
			TEXT("bIgnoreBotMessages=True\n")
			TEXT("; Message posted to Discord when the server comes online. Leave empty to disable.\n")
			TEXT("ServerOnlineMessage=:green_circle: Server is now **online**!\n")
			TEXT("; Message posted to Discord when the server shuts down. Leave empty to disable.\n")
			TEXT("ServerOfflineMessage=:red_circle: Server is now **offline**.\n");

		// Ensure the Config directory exists before writing.
		PlatformFile.CreateDirectoryTree(*FPaths::GetPath(FilePath));

		if (FFileHelper::SaveStringToFile(DefaultContent, *FilePath))
		{
			UE_LOG(LogTemp, Log,
			       TEXT("DiscordBridge: Wrote default config to '%s'. "
			            "Set BotToken and ChannelId in that file, then restart "
			            "the server to enable the Discord bridge."), *FilePath);
		}
		else
		{
			UE_LOG(LogTemp, Warning,
			       TEXT("DiscordBridge: Could not write default config to '%s'."),
			       *FilePath);
		}
	}

	return Config;
}
