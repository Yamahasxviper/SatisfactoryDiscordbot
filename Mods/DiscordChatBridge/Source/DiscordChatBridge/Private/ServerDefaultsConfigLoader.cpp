// Copyright (c) 2024 Discord Chat Bridge Contributors

#include "ServerDefaultsConfigLoader.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "HAL/PlatformFileManager.h"

bool FServerDefaultsConfigLoader::LoadFromServerDefaults(FDiscordBotConfig& OutConfig)
{
	FString ServerDefaultsPath = GetServerDefaultsPath();
	FString ConfigFilePath = FPaths::Combine(ServerDefaultsPath, TEXT("DiscordChatBridge.txt"));
	
	UE_LOG(LogTemp, Log, TEXT("ServerDefaultsConfigLoader: Attempting to load config from: %s"), *ConfigFilePath);
	
	if (!FPaths::FileExists(ConfigFilePath))
	{
		UE_LOG(LogTemp, Warning, TEXT("ServerDefaultsConfigLoader: Config file not found at: %s"), *ConfigFilePath);
		return false;
	}
	
	return ParseTxtConfig(ConfigFilePath, OutConfig);
}

bool FServerDefaultsConfigLoader::ParseTxtConfig(const FString& FilePath, FDiscordBotConfig& OutConfig)
{
	FString FileContent;
	if (!FFileHelper::LoadFileToString(FileContent, *FilePath))
	{
		UE_LOG(LogTemp, Error, TEXT("ServerDefaultsConfigLoader: Failed to read file: %s"), *FilePath);
		return false;
	}
	
	TArray<FString> Lines;
	FileContent.ParseIntoArrayLines(Lines);
	
	int32 LoadedSettings = 0;
	
	for (const FString& Line : Lines)
	{
		FString Key, Value;
		if (ParseConfigLine(Line, Key, Value))
		{
			SetConfigValue(Key, Value, OutConfig);
			LoadedSettings++;
		}
	}
	
	UE_LOG(LogTemp, Log, TEXT("ServerDefaultsConfigLoader: Successfully loaded %d settings from TXT config"), LoadedSettings);
	
	// Validate required settings
	if (OutConfig.BotToken.IsEmpty() || OutConfig.ChannelId.IsEmpty())
	{
		UE_LOG(LogTemp, Warning, TEXT("ServerDefaultsConfigLoader: Configuration incomplete - BotToken and ChannelId must be set"));
		return false;
	}
	
	return true;
}

bool FServerDefaultsConfigLoader::ParseConfigLine(const FString& Line, FString& OutKey, FString& OutValue)
{
	// Trim whitespace
	FString TrimmedLine = Line.TrimStartAndEnd();
	
	// Skip empty lines
	if (TrimmedLine.IsEmpty())
	{
		return false;
	}
	
	// Skip comments (lines starting with #)
	if (TrimmedLine.StartsWith(TEXT("#")))
	{
		return false;
	}
	
	// Find the = separator
	int32 SeparatorIndex = INDEX_NONE;
	if (!TrimmedLine.FindChar('=', SeparatorIndex))
	{
		UE_LOG(LogTemp, Warning, TEXT("ServerDefaultsConfigLoader: Invalid line format (no '=' found): %s"), *TrimmedLine);
		return false;
	}
	
	// Split into key and value
	OutKey = TrimmedLine.Left(SeparatorIndex).TrimStartAndEnd();
	OutValue = TrimmedLine.Mid(SeparatorIndex + 1).TrimStartAndEnd();
	
	// Skip if key is empty
	if (OutKey.IsEmpty())
	{
		return false;
	}
	
	return true;
}

FString FServerDefaultsConfigLoader::GetServerDefaultsPath()
{
	// Get the Mods directory path
	FString ModsPath = FPaths::ProjectModsDir();
	
	// Combine with DiscordChatBridge/ServerDefaults
	FString ServerDefaultsPath = FPaths::Combine(ModsPath, TEXT("DiscordChatBridge"), TEXT("ServerDefaults"));
	
	return FPaths::ConvertRelativePathToFull(ServerDefaultsPath);
}

void FServerDefaultsConfigLoader::SetConfigValue(const FString& Key, const FString& Value, FDiscordBotConfig& Config)
{
	// Skip placeholder values
	if (Value.StartsWith(TEXT("YOUR_")) && Value.EndsWith(TEXT("_HERE")))
	{
		UE_LOG(LogTemp, Verbose, TEXT("ServerDefaultsConfigLoader: Skipping placeholder value for key: %s"), *Key);
		return;
	}
	
	// Map keys to config struct fields
	if (Key.Equals(TEXT("BotToken"), ESearchCase::IgnoreCase))
	{
		Config.BotToken = Value;
		UE_LOG(LogTemp, Verbose, TEXT("ServerDefaultsConfigLoader: Set BotToken"));
	}
	else if (Key.Equals(TEXT("ChannelId"), ESearchCase::IgnoreCase))
	{
		Config.ChannelId = Value;
		UE_LOG(LogTemp, Verbose, TEXT("ServerDefaultsConfigLoader: Set ChannelId: %s"), *Value);
	}
	else if (Key.Equals(TEXT("PollIntervalSeconds"), ESearchCase::IgnoreCase))
	{
		Config.PollIntervalSeconds = ParseFloat(Value);
		UE_LOG(LogTemp, Verbose, TEXT("ServerDefaultsConfigLoader: Set PollIntervalSeconds: %f"), Config.PollIntervalSeconds);
	}
	else if (Key.Equals(TEXT("DiscordSourceLabel"), ESearchCase::IgnoreCase))
	{
		Config.DiscordSourceLabel = Value;
	}
	else if (Key.Equals(TEXT("GameSourceLabel"), ESearchCase::IgnoreCase))
	{
		Config.GameSourceLabel = Value;
	}
	else if (Key.Equals(TEXT("DiscordNameFormat"), ESearchCase::IgnoreCase))
	{
		Config.DiscordNameFormat = Value;
	}
	else if (Key.Equals(TEXT("GameNameFormat"), ESearchCase::IgnoreCase))
	{
		Config.GameNameFormat = Value;
	}
	else if (Key.Equals(TEXT("EnableServerNotifications"), ESearchCase::IgnoreCase))
	{
		Config.bEnableServerNotifications = ParseBool(Value);
	}
	else if (Key.Equals(TEXT("NotificationChannelId"), ESearchCase::IgnoreCase))
	{
		Config.NotificationChannelId = Value;
	}
	else if (Key.Equals(TEXT("ServerStartMessage"), ESearchCase::IgnoreCase))
	{
		Config.ServerStartMessage = Value;
	}
	else if (Key.Equals(TEXT("ServerStopMessage"), ESearchCase::IgnoreCase))
	{
		Config.ServerStopMessage = Value;
	}
	else if (Key.Equals(TEXT("EnableBotActivity"), ESearchCase::IgnoreCase))
	{
		Config.bEnableBotActivity = ParseBool(Value);
	}
	else if (Key.Equals(TEXT("UseGatewayForPresence"), ESearchCase::IgnoreCase))
	{
		Config.bUseGatewayForPresence = ParseBool(Value);
	}
	else if (Key.Equals(TEXT("BotActivityFormat"), ESearchCase::IgnoreCase))
	{
		Config.BotActivityFormat = Value;
	}
	else if (Key.Equals(TEXT("GatewayPresenceFormat"), ESearchCase::IgnoreCase))
	{
		Config.GatewayPresenceFormat = Value;
	}
	else if (Key.Equals(TEXT("GatewayActivityType"), ESearchCase::IgnoreCase))
	{
		Config.GatewayActivityType = ParseInt(Value);
	}
	else if (Key.Equals(TEXT("ActivityUpdateIntervalSeconds"), ESearchCase::IgnoreCase))
	{
		Config.ActivityUpdateIntervalSeconds = ParseFloat(Value);
	}
	else if (Key.Equals(TEXT("BotActivityChannelId"), ESearchCase::IgnoreCase))
	{
		Config.BotActivityChannelId = Value;
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("ServerDefaultsConfigLoader: Unknown configuration key: %s"), *Key);
	}
}

bool FServerDefaultsConfigLoader::ParseBool(const FString& Value)
{
	FString LowerValue = Value.ToLower();
	
	// Accept various boolean representations
	if (LowerValue.Equals(TEXT("true")) || LowerValue.Equals(TEXT("yes")) || 
		LowerValue.Equals(TEXT("1")) || LowerValue.Equals(TEXT("on")))
	{
		return true;
	}
	
	return false;
}

float FServerDefaultsConfigLoader::ParseFloat(const FString& Value)
{
	return FCString::Atof(*Value);
}

int32 FServerDefaultsConfigLoader::ParseInt(const FString& Value)
{
	return FCString::Atoi(*Value);
}
