// Copyright (c) 2024 Discord Chat Bridge Contributors

#pragma once

#include "CoreMinimal.h"
#include "DiscordAPI.h"

/**
 * Configuration loader that supports TXT format files with KEY=VALUE pairs
 * Provides automatic loading and persistence via SML configuration system
 */
class DISCORDCHATBRIDGE_API FServerDefaultsConfigLoader
{
public:
	/**
	 * Load configuration from ServerDefaults/DiscordChatBridge.txt
	 * @param OutConfig The configuration struct to fill
	 * @return true if configuration was loaded successfully, false otherwise
	 */
	static bool LoadFromServerDefaults(FDiscordBotConfig& OutConfig);

	/**
	 * Parse a TXT format configuration file
	 * @param FilePath Full path to the TXT file
	 * @param OutConfig The configuration struct to fill
	 * @return true if file was parsed successfully, false otherwise
	 */
	static bool ParseTxtConfig(const FString& FilePath, FDiscordBotConfig& OutConfig);

private:
	/**
	 * Parse a single line from the TXT config file
	 * @param Line The line to parse (KEY=VALUE format)
	 * @param OutKey The parsed key
	 * @param OutValue The parsed value
	 * @return true if line was parsed successfully, false if comment or invalid
	 */
	static bool ParseConfigLine(const FString& Line, FString& OutKey, FString& OutValue);

	/**
	 * Get the path to the ServerDefaults directory
	 * @return Full path to ServerDefaults directory
	 */
	static FString GetServerDefaultsPath();

	/**
	 * Set a configuration value based on key-value pair
	 * @param Key The configuration key
	 * @param Value The configuration value
	 * @param Config The configuration struct to update
	 */
	static void SetConfigValue(const FString& Key, const FString& Value, FDiscordBotConfig& Config);

	/**
	 * Convert string to boolean
	 * @param Value String value (true/false, yes/no, 1/0)
	 * @return Boolean value
	 */
	static bool ParseBool(const FString& Value);

	/**
	 * Convert string to float
	 * @param Value String value
	 * @return Float value
	 */
	static float ParseFloat(const FString& Value);

	/**
	 * Convert string to int32
	 * @param Value String value
	 * @return Integer value
	 */
	static int32 ParseInt(const FString& Value);
};
