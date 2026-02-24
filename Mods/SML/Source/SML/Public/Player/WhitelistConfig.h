#pragma once
#include "CoreMinimal.h"

/**
 * All configurable settings for the server whitelist and Discord integration.
 *
 * Stored in <ProjectDir>/Configs/SML_WhitelistConfig.json — completely
 * independent of the SML core config (Configs/SML.cfg).
 *
 * Edit the JSON file directly to pre-configure the server before startup, or
 * use the in-game /whitelist on|off commands to toggle at runtime (the file is
 * updated immediately and persists across restarts).
 */
struct SML_API FWhitelistConfig {
public:
    FWhitelistConfig();

    // -----------------------------------------------------------------------
    // Whitelist behaviour
    // -----------------------------------------------------------------------

    /** Whether the whitelist is active. When true, only whitelisted players can join. */
    bool bEnableWhitelist;

    /**
     * Fallback role name used when Discord is not configured.
     * Only players whose in-game name matches this value can manage the whitelist.
     * Ignored when DiscordBotToken / DiscordGuildId / DiscordWhitelistRoleId are set.
     */
    FString WhitelistRole;

    /**
     * Informational in-game channel name shown in help text.
     * Does not enforce any restriction by itself.
     */
    FString WhitelistChannel;

    // -----------------------------------------------------------------------
    // Discord integration
    // -----------------------------------------------------------------------

    /**
     * Discord bot token (without the "Bot " prefix — it is added automatically).
     * Obtain from the Discord Developer Portal → Your App → Bot.
     * Leave empty to disable Discord role checking.
     */
    FString DiscordBotToken;

    /** Snowflake ID of the Discord guild (server) to check membership in. */
    FString DiscordGuildId;

    /**
     * Snowflake ID of the Discord role whose members may manage the whitelist.
     * Enable Developer Mode in Discord, then right-click the role → Copy ID.
     */
    FString DiscordWhitelistRoleId;

    /**
     * Snowflake ID of the Discord channel where players should run /whitelist link.
     * Shown in hint messages; does not restrict actual command usage.
     */
    FString DiscordWhitelistChannelId;

public:
    /** Deserialises config from a JSON object. Sets *OutIsMissingSections = true if any field was absent. */
    static void ReadFromJson(const TSharedPtr<class FJsonObject>& Json, FWhitelistConfig& OutConfig, bool* OutIsMissingSections = nullptr);

    /** Serialises config into a JSON object. */
    static void WriteToJson(const TSharedPtr<class FJsonObject>& OutJson, const FWhitelistConfig& Config);
};

/**
 * Manages the dedicated whitelist configuration file.
 *
 * The config is stored at <ProjectDir>/Configs/SML_WhitelistConfig.json.
 * Call LoadConfig() on server startup; afterwards use GetConfig() anywhere.
 * Mutations should be applied via SetConfig() + SaveConfig() so they persist.
 */
class SML_API FWhitelistConfigManager {
public:
    /** Loads config from disk. Creates the file with defaults if it does not exist. */
    static void LoadConfig();

    /** Writes the current in-memory config to disk immediately. */
    static void SaveConfig();

    /** Returns the current in-memory whitelist config. */
    static FWhitelistConfig GetConfig();

    /** Replaces the in-memory config. Call SaveConfig() to persist. */
    static void SetConfig(const FWhitelistConfig& NewConfig);

private:
    static FString GetConfigFilePath();
    static FWhitelistConfig ConfigPrivate;
};
