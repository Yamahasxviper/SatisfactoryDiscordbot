#pragma once
#include "CoreMinimal.h"

/**
 * Utility for verifying Discord role membership from the game server.
 *
 * Two responsibilities:
 *  1. Persist a player-name ↔ Discord-user-ID mapping in
 *     <ProjectDir>/Configs/SML_DiscordLinks.json so the association survives
 *     server restarts.
 *  2. Perform async HTTP calls to the Discord REST API
 *     (GET /guilds/{guildId}/members/{userId}) to check whether a linked user
 *     currently holds a specific role.
 *
 * Usage flow:
 *  - A player runs `/whitelist link <discordUserId>` in-game which calls
 *    LinkPlayer().
 *  - When a whitelist management command is issued, the command handler looks
 *    up the sender's Discord ID via GetDiscordUserId(), then calls
 *    CheckMemberHasRole() with a callback that either proceeds or rejects.
 */
class SML_API FDiscordRoleChecker {
public:
    /** Loads player↔Discord-ID links from disk. Creates an empty file if none exists. */
    static void LoadLinks();

    /** Saves player↔Discord-ID links to disk. */
    static void SaveLinks();

    /**
     * Associates an in-game player name with a Discord user ID.
     * Overwrites any previous mapping for this player.
     */
    static void LinkPlayer(const FString& PlayerName, const FString& DiscordUserId);

    /**
     * Returns the Discord user ID linked to the given player name, or an empty
     * string if no link exists.
     */
    static FString GetDiscordUserId(const FString& PlayerName);

    /**
     * Asynchronously checks whether the given Discord user holds the specified
     * role in the specified guild.
     *
     * @param DiscordUserId   Snowflake ID of the Discord user.
     * @param GuildId         Snowflake ID of the Discord guild.
     * @param RoleId          Snowflake ID of the role to check.
     * @param BotToken        Raw bot token (without the "Bot " prefix).
     * @param Callback        Called on the game thread with true if the role is
     *                        held, false otherwise (including on HTTP errors).
     */
    static void CheckMemberHasRole(
        const FString& DiscordUserId,
        const FString& GuildId,
        const FString& RoleId,
        const FString& BotToken,
        TFunction<void(bool bHasRole)> Callback);

private:
    /** Path to the JSON links file. */
    static FString GetLinksFilePath();

    /** PlayerName (lower-case) → Discord user ID map. */
    static TMap<FString, FString> PlayerToDiscordId;
};
