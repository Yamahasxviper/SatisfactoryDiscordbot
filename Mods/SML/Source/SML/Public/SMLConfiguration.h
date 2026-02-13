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
     * Prefix character(s) for chat commands
     * Default: "/"
     */
    FString ChatCommandPrefix;

    /**
     * Cooldown duration in seconds between chat commands per player
     * Set to 0 to disable cooldown
     * Default: 0
     */
    float ChatCommandCooldown;

    /**
     * Maximum length for chat messages
     * Set to 0 for unlimited
     * Default: 0 (unlimited)
     */
    int32 MaxChatMessageLength;

    /**
     * Whether to enable chat command logging to file
     * Default: false
     */
    bool bEnableChatLogging;

    /**
     * Whether to enable Discord integration
     * Default: false
     */
    bool bEnableDiscordIntegration;

    /**
     * Discord webhook URL for sending chat messages and notifications
     * Required if Discord integration is enabled
     */
    FString DiscordWebhookURL;

    /**
     * Discord bot token for advanced bot features (optional)
     * Leave empty to use webhook-only mode
     */
    FString DiscordBotToken;

    /**
     * Discord channel ID for receiving messages
     * Required if using bot token
     */
    FString DiscordChannelID;

    /**
     * Template for Discord messages sent from in-game chat
     * Available placeholders: {player}, {message}, {timestamp}
     * Default: "**{player}**: {message}"
     */
    FString DiscordMessageFormat;
public:
    /** Deserializes configuration from JSON object */
    static void ReadFromJson(const TSharedPtr<class FJsonObject>& Json, FSMLConfiguration& OutConfiguration, bool* OutIsMissingSections = NULL);

    /** Serializes configuration into JSON object */
    static void WriteToJson(const TSharedPtr<class FJsonObject>& OutJson, const FSMLConfiguration& Configuration);
};