#pragma once
#include "CoreMinimal.h"

/**
 * Manages the SML server whitelist, including persistent storage across server restarts.
 *
 * The whitelist is stored in <ProjectDir>/Configs/SML_Whitelist.json and is loaded on
 * startup. Modifications (add/remove) are written to disk immediately so they survive
 * server restarts.
 *
 * Whitelist enforcement (kicking players who are not listed) is gated behind the
 * FSMLConfiguration::bEnableWhitelist flag.  Role-based management access and the
 * dedicated in-game channel are read from FSMLConfiguration::WhitelistRole /
 * WhitelistChannel.
 */
class SML_API FSMLWhitelistManager {
public:
    /** Loads the whitelist from disk.  Creates an empty file if none exists. */
    static void LoadWhitelist();

    /** Saves the current in-memory whitelist to disk. */
    static void SaveWhitelist();

    /** Returns true if the named player is on the whitelist (case-insensitive). */
    static bool IsPlayerWhitelisted(const FString& PlayerName);

    /**
     * Adds a player to the whitelist and persists the change.
     * Returns false if the player was already listed.
     */
    static bool AddPlayer(const FString& PlayerName);

    /**
     * Removes a player from the whitelist and persists the change.
     * Returns false if the player was not listed.
     */
    static bool RemovePlayer(const FString& PlayerName);

    /** Returns a copy of the current whitelist. */
    static TArray<FString> GetWhitelistedPlayers();

private:
    /** Path to the whitelist JSON file on disk. */
    static FString GetWhitelistFilePath();

    /** In-memory set of whitelisted player names (stored lower-case for comparison). */
    static TArray<FString> WhitelistedPlayers;
};
