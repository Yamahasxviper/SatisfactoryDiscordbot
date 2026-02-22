// Copyright Coffee Stain Studios. All Rights Reserved.

#include "DiscordBridgeConfig.h"

#include "Dom/JsonObject.h"
#include "HAL/PlatformFileManager.h"
#include "Interfaces/IPluginManager.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"
#include "Serialization/JsonWriter.h"

// ─────────────────────────────────────────────────────────────────────────────
// Helpers
// ─────────────────────────────────────────────────────────────────────────────

namespace
{
	FString GetStringFieldOrDefault(const TSharedPtr<FJsonObject>& Json,
	                                const FString& Key,
	                                const FString& Default)
	{
		FString Value;
		if (Json->TryGetStringField(Key, Value))
		{
			return Value;
		}
		return Default;
	}

	bool GetBoolFieldOrDefault(const TSharedPtr<FJsonObject>& Json,
	                           const FString& Key,
	                           bool Default)
	{
		bool Value = Default;
		Json->TryGetBoolField(Key, Value);
		return Value;
	}
} // anonymous namespace

// ─────────────────────────────────────────────────────────────────────────────
// FDiscordBridgeConfig
// ─────────────────────────────────────────────────────────────────────────────

FString FDiscordBridgeConfig::GetConfigFilePath()
{
	// Primary location: inside the plugin's own Config directory.
	// On a deployed server this resolves to
	//   <ServerRoot>/FactoryGame/Mods/DiscordBridge/Config/DiscordBridge.cfg
	// Alpakit only stages files that exist in the source tree, so
	// DiscordBridge.cfg (which is auto-generated at runtime and is gitignored)
	// is never included in a packaged update and therefore survives mod upgrades.
	const TSharedPtr<IPlugin> Plugin = IPluginManager::Get().FindPlugin(TEXT("DiscordBridge"));
	if (Plugin.IsValid())
	{
		return FPaths::Combine(Plugin->GetBaseDir(), TEXT("Config"), TEXT("DiscordBridge.cfg"));
	}

	// Fallback (should not be reached in a normal deployment): use the SML
	// convention so the mod still functions even without a valid plugin entry.
	return FPaths::Combine(FPaths::ProjectDir(), TEXT("Configs"), TEXT("DiscordBridge.cfg"));
}

FDiscordBridgeConfig FDiscordBridgeConfig::LoadOrCreate()
{
	FDiscordBridgeConfig Config;
	const FString FilePath = GetConfigFilePath();

	IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();

	bool bNeedsSave = false;

	if (PlatformFile.FileExists(*FilePath))
	{
		FString FileContent;
		if (FFileHelper::LoadFileToString(FileContent, *FilePath))
		{
			TSharedPtr<FJsonObject> JsonObject;
			const TSharedRef<TJsonReader<>> Reader =
				TJsonReaderFactory<>::Create(FileContent);

			if (FJsonSerializer::Deserialize(Reader, JsonObject) && JsonObject.IsValid())
			{
				Config.BotToken             = GetStringFieldOrDefault(JsonObject, TEXT("BotToken"),             TEXT(""));
				Config.ChannelId            = GetStringFieldOrDefault(JsonObject, TEXT("ChannelId"),            TEXT(""));
				Config.GameToDiscordFormat  = GetStringFieldOrDefault(JsonObject, TEXT("GameToDiscordFormat"),  Config.GameToDiscordFormat);
				Config.DiscordToGameFormat  = GetStringFieldOrDefault(JsonObject, TEXT("DiscordToGameFormat"),  Config.DiscordToGameFormat);
				Config.bIgnoreBotMessages   = GetBoolFieldOrDefault  (JsonObject, TEXT("bIgnoreBotMessages"),   Config.bIgnoreBotMessages);
				Config.ServerOnlineMessage  = GetStringFieldOrDefault(JsonObject, TEXT("ServerOnlineMessage"),  Config.ServerOnlineMessage);
				Config.ServerOfflineMessage = GetStringFieldOrDefault(JsonObject, TEXT("ServerOfflineMessage"), Config.ServerOfflineMessage);

				UE_LOG(LogTemp, Log, TEXT("DiscordBridge: Loaded config from %s"), *FilePath);
			}
			else
			{
				// The file exists but contains malformed JSON.  Log a warning and
				// use built-in defaults for this session.  We intentionally do NOT
				// overwrite the file so the server operator can inspect and fix it
				// without losing their BotToken / ChannelId settings.
				UE_LOG(LogTemp, Warning,
				       TEXT("DiscordBridge: Failed to parse config JSON at '%s'. "
				            "Using built-in defaults for this session. "
				            "Please fix the JSON syntax and restart the server."), *FilePath);
			}
		}
		else
		{
			UE_LOG(LogTemp, Warning,
			       TEXT("DiscordBridge: Failed to read config file at '%s'. "
			            "Using built-in defaults."), *FilePath);
		}
	}
	else
	{
		UE_LOG(LogTemp, Log,
		       TEXT("DiscordBridge: Config file not found at '%s'. "
		            "Creating it with defaults."), *FilePath);
		bNeedsSave = true;
	}

	if (bNeedsSave)
	{
		TSharedPtr<FJsonObject> JsonObject = MakeShared<FJsonObject>();
		JsonObject->SetStringField(TEXT("BotToken"),             Config.BotToken);
		JsonObject->SetStringField(TEXT("ChannelId"),            Config.ChannelId);
		JsonObject->SetStringField(TEXT("GameToDiscordFormat"),  Config.GameToDiscordFormat);
		JsonObject->SetStringField(TEXT("DiscordToGameFormat"),  Config.DiscordToGameFormat);
		JsonObject->SetBoolField  (TEXT("bIgnoreBotMessages"),   Config.bIgnoreBotMessages);
		JsonObject->SetStringField(TEXT("ServerOnlineMessage"),  Config.ServerOnlineMessage);
		JsonObject->SetStringField(TEXT("ServerOfflineMessage"), Config.ServerOfflineMessage);

		FString JsonContent;
		const TSharedRef<TJsonWriter<>> Writer =
			TJsonWriterFactory<>::Create(&JsonContent);
		FJsonSerializer::Serialize(JsonObject.ToSharedRef(), Writer);

		// Ensure the Configs directory exists before writing.
		PlatformFile.CreateDirectoryTree(*FPaths::GetPath(FilePath));

		if (FFileHelper::SaveStringToFile(JsonContent, *FilePath))
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
