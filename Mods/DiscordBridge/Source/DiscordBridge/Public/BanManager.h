#pragma once
#include "CoreMinimal.h"

/**
 * Manages the server ban list for the DiscordBridge mod.
 *
 * Config is stored at <ProjectSavedDir>/ServerBanlist.json.
 * The file is created with defaults on first use and written to disk
 * immediately on every change so bans survive server restarts.
 *
 * No additional dependency beyond Core/Json — uses UE4 Json + FFileHelper.
 *
 * Example file:
 *   {
 *     "enabled": true,
 *     "players": ["alice", "badguy"]
 *   }
 */
class DISCORDBRIDGE_API FBanManager
{
public:
	/** Load (or create) the ban list file from disk. Call once at startup. */
	static void Load();

	/** Persist the current state to disk immediately. */
	static void Save();

	/** Returns true if the ban system is currently active. */
	static bool IsEnabled();

	/** Enable or disable the ban system and save. */
	static void SetEnabled(bool bEnabled);

	/**
	 * Returns true if the given player name is banned.
	 * Comparison is case-insensitive.
	 */
	static bool IsBanned(const FString& PlayerName);

	/**
	 * Bans a player and saves. Returns false if already banned.
	 */
	static bool BanPlayer(const FString& PlayerName);

	/**
	 * Unbans a player and saves. Returns false if not found.
	 */
	static bool UnbanPlayer(const FString& PlayerName);

	/** Returns a copy of the current ban list. */
	static TArray<FString> GetAll();

private:
	static FString GetFilePath();

	static bool bEnabled;
	static TArray<FString> Players; // stored lower-case for comparison
};
