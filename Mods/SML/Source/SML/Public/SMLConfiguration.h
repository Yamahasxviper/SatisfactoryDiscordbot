#pragma once
#include "CoreMinimal.h"

struct SML_API FSMLConfiguration {
public:
    /** Constructor setting configuration defaults */
    FSMLConfiguration();

    /**
    * List of fully qualified chat command names that won't be usable by players in the game
    * Full command name is mod_reference:command_name
    * Note that this setting will only affect players, other command senders will still be able to use it
    */
    TArray<FString> DisabledChatCommands;

    /**
     * Whether to enable funchook logging
     */
    bool bEnableFunchookLogging;

    /**
     * Format string for displaying Discord names in in-game chat
     * Use {name} as placeholder for the actual name
     * Example: "[Discord] {name}" will display as "[Discord] Username"
     */
    FString InGameChatNameFormat;

    /**
     * Format string for displaying in-game names in Discord
     * Use {name} as placeholder for the actual name
     * Example: "[Game] {name}" will display as "[Game] PlayerName"
     */
    FString DiscordNameFormat;

    /**
     * Whether to enable name formatting for chat and Discord integration
     */
    bool bEnableNameFormatting;
public:
    /** Deserializes configuration from JSON object */
    static void ReadFromJson(const TSharedPtr<class FJsonObject>& Json, FSMLConfiguration& OutConfiguration, bool* OutIsMissingSections = NULL);

    /** Serializes configuration into JSON object */
    static void WriteToJson(const TSharedPtr<class FJsonObject>& OutJson, const FSMLConfiguration& Configuration);
};