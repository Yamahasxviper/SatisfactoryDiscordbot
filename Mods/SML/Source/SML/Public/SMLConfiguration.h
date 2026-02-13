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

    // ===== Discord Integration Settings =====
    
    /**
     * Whether to enable Discord integration for chat relay
     */
    bool bEnableDiscordIntegration;

    /**
     * Discord webhook URL for sending messages to Discord
     * Example: https://discord.com/api/webhooks/123456789/abcdefghijk
     */
    FString DiscordWebhookURL;

    /**
     * Optional Discord bot token for advanced features
     * Leave empty if using webhook-only mode
     */
    FString DiscordBotToken;

    /**
     * Optional Discord channel ID for receiving messages from Discord
     * Leave empty if not receiving messages from Discord
     */
    FString DiscordChannelID;

    // ===== In-Game Chat Settings =====

    /**
     * Whether to relay in-game chat messages to Discord
     */
    bool bRelayChatToDiscord;

    /**
     * Whether to relay Discord messages to in-game chat
     */
    bool bRelayDiscordToChat;

    /**
     * Prefix to add to messages sent to Discord
     * Example: "[Game]" or "[Satisfactory]"
     */
    FString ChatToDiscordPrefix;

    /**
     * Suffix to add to messages sent to Discord
     */
    FString ChatToDiscordSuffix;

    /**
     * Prefix to add to Discord messages shown in-game
     * Example: "[Discord]" or "[Discord User]"
     */
    FString DiscordToChatPrefix;

    /**
     * Suffix to add to Discord messages shown in-game
     */
    FString DiscordToChatSuffix;

    /**
     * Whether to filter system messages from being sent to Discord
     * If true, only player messages will be relayed
     */
    bool bFilterSystemMessages;

    /**
     * Whether to include player name in Discord messages
     */
    bool bIncludePlayerNameInDiscord;

    /**
     * Custom format for chat messages sent to Discord
     * Placeholders: {PlayerName}, {Message}, {Timestamp}
     * Example: "**{PlayerName}**: {Message}"
     */
    FString DiscordMessageFormat;
public:
    /** Deserializes configuration from JSON object */
    static void ReadFromJson(const TSharedPtr<class FJsonObject>& Json, FSMLConfiguration& OutConfiguration, bool* OutIsMissingSections = NULL);

    /** Serializes configuration into JSON object */
    static void WriteToJson(const TSharedPtr<class FJsonObject>& OutJson, const FSMLConfiguration& Configuration);
};