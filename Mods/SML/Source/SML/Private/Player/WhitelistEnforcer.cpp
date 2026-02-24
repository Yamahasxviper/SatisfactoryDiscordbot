#include "Player/WhitelistEnforcer.h"
#include "FGGameMode.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/PlayerState.h"
#include "Engine/NetConnection.h"
#include "Network/NetworkHandler.h"
#include "Patching/NativeHookManager.h"
#include "Player/SMLWhitelistManager.h"
#include "Player/WhitelistConfig.h"

DEFINE_LOG_CATEGORY_STATIC(LogWhitelistEnforcer, Log, All);

void FWhitelistEnforcer::RegisterHandler() {
    // Hook AFGGameMode::PostLogin directly using SML's native hook system.
    // SUBSCRIBE_UOBJECT_METHOD_AFTER runs our lambda after the original PostLogin
    // body has fully completed, so PlayerState and the net connection are ready.
    SUBSCRIBE_UOBJECT_METHOD_AFTER(AFGGameMode, PostLogin,
        [](AFGGameMode* GameMode, APlayerController* Controller) {

            // Skip local (listen-server host) controllers — they have no net connection.
            if (!Controller || Controller->IsLocalController()) {
                return;
            }

            // Only enforce when the whitelist is enabled.
            const FWhitelistConfig Config = FWhitelistConfigManager::GetConfig();
            if (!Config.bEnableWhitelist) {
                return;
            }

            const APlayerState* PS = Controller->GetPlayerState<APlayerState>();
            const FString PlayerName = PS ? PS->GetPlayerName() : FString();

            if (FSMLWhitelistManager::IsPlayerWhitelisted(PlayerName)) {
                return; // Player is allowed — nothing to do.
            }

            UE_LOG(LogWhitelistEnforcer, Warning,
                TEXT("Whitelist: rejecting non-whitelisted player '%s'"), *PlayerName);

            // CloseWithFailureMessage sends the reason string to the client before
            // closing the connection — the player sees the message on their screen.
            UNetConnection* NetConnection = Cast<UNetConnection>(Controller->Player);
            if (NetConnection) {
                UModNetworkHandler::CloseWithFailureMessage(NetConnection,
                    TEXT("You are not on this server's whitelist. Contact an admin to be added."));
            }
        });
}
