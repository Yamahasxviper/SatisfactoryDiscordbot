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
     * Whether the server whitelist is enabled.
     * When enabled, only players listed in the whitelist file may join the server.
     */
    bool bEnableWhitelist;

    /**
     * Role name required to manage the whitelist via in-game commands (e.g. "admin").
     * Players whose name matches this role can add/remove whitelist entries.
     */
    FString WhitelistRole;

    /**
     * In-game chat channel name reserved for whitelist management commands.
     * Set to an empty string to allow whitelist commands from any context.
     */
    FString WhitelistChannel;

    // -------------------------------------------------------------------------
    // Discord integration
    // -------------------------------------------------------------------------

    /**
     * Discord bot token used to authenticate API calls.
     * Obtain this from the Discord Developer Portal under your application's Bot page.
     * Example: "Bot MTExxx.YYY.ZZZ"  (the "Bot " prefix is added automatically)
     */
    FString DiscordBotToken;

    /**
     * Discord guild (server) ID where role membership is checked.
     */
    FString DiscordGuildId;

    /**
     * Discord role ID whose members are authorised to manage the whitelist.
     * Copy the role ID from Discord (Developer Mode → right-click role → Copy ID).
     */
    FString DiscordWhitelistRoleId;

    /**
     * Discord channel ID reserved for whitelist commands.
     * When non-empty, the /whitelist link subcommand will tell players to use this channel.
     */
    FString DiscordWhitelistChannelId;
public:
    /** Deserializes configuration from JSON object */
    static void ReadFromJson(const TSharedPtr<class FJsonObject>& Json, FSMLConfiguration& OutConfiguration, bool* OutIsMissingSections = NULL);

    /** Serializes configuration into JSON object */
    static void WriteToJson(const TSharedPtr<class FJsonObject>& OutJson, const FSMLConfiguration& Configuration);
};