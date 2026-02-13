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
     * Whether to enable Discord chat synchronization
     * When enabled, in-game chat messages will be sent to Discord and vice versa
     */
    bool bEnableDiscordChatSync;

    /**
     * Discord webhook URL for sending in-game chat messages to Discord
     * If empty, Discord chat sync will be disabled
     */
    FString DiscordWebhookURL;

    /**
     * Discord bot token for receiving messages from Discord
     * Required for bidirectional chat synchronization
     */
    FString DiscordBotToken;

    /**
     * Discord channel ID to sync chat messages with
     * If empty, messages will be sent to the default webhook channel
     */
    FString DiscordChannelID;

    /**
     * Prefix to add to Discord messages when showing in-game
     * Default: "[Discord]"
     */
    FString DiscordMessagePrefix;

    /**
     * Prefix to add to in-game messages when sending to Discord
     * Default: "[Game]"
     */
    FString GameMessagePrefix;

    /**
     * Whether to sync system messages (e.g., player joined/left) to Discord
     */
    bool bSyncSystemMessages;

    /**
     * Whether to sync player chat messages to Discord
     */
    bool bSyncPlayerMessages;

    /**
     * List of player names whose messages should not be synced to Discord
     * Useful for filtering out bot messages or specific players
     */
    TArray<FString> DiscordSyncBlacklist;
public:
    /** Deserializes configuration from JSON object */
    static void ReadFromJson(const TSharedPtr<class FJsonObject>& Json, FSMLConfiguration& OutConfiguration, bool* OutIsMissingSections = NULL);

    /** Serializes configuration into JSON object */
    static void WriteToJson(const TSharedPtr<class FJsonObject>& OutJson, const FSMLConfiguration& Configuration);
};