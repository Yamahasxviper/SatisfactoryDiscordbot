#pragma once
#include "CoreMinimal.h"

/**
 * Hooks directly into AFGGameMode::PostLogin using SML's native hook system
 * (SUBSCRIBE_UOBJECT_METHOD_AFTER) to enforce the server whitelist.
 *
 * This is completely self-contained and independent of SMLNetworkManager —
 * the same pattern used by FPlayerCheatManagerHandler for cheat protection.
 *
 * When the whitelist is enabled (FWhitelistConfigManager::GetConfig().bEnableWhitelist),
 * any remote player whose in-game name is not in FSMLWhitelistManager is
 * rejected immediately after PostLogin via UModNetworkHandler::CloseWithFailureMessage,
 * which sends the reason string to the client before closing the connection.
 */
class SML_API FWhitelistEnforcer {
public:
    /** Registers the AFGGameMode::PostLogin hook. Call once during mod initialisation. */
    static void RegisterHandler();
};
