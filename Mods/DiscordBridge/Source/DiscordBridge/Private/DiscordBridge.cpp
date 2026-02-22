// Copyright Coffee Stain Studios. All Rights Reserved.

#include "DiscordBridge.h"
#include "DiscordBridgeSubsystem.h"
#include "Modules/ModuleManager.h"
#include "Patching/NativeHookManager.h"
#include "FGPlayerController.h"

// Stored so the hook can be removed cleanly in ShutdownModule.
static FDelegateHandle GChatHookHandle;

void FDiscordBridgeModule::StartupModule()
{
	// Hook AFGPlayerController::Server_SendChatMessage_Implementation to forward player
	// chat messages to Discord.  This is the server-side RPC handler called when a player
	// submits a chat message; it has a non-trivial body that funchook can safely instrument.
	// AddChatMessageToReceived, BroadcastChatMessage, and
	// Multicast_BroadcastChatMessage_Implementation are all too short for funchook
	// ("Too short instructions").  The player name is obtained from the PlayerController's
	// PlayerState.  The FDiscordBridgeModule friend declaration required to access this
	// private method is configured in Config/AccessTransformers.ini.
	GChatHookHandle = SUBSCRIBE_UOBJECT_METHOD(AFGPlayerController, Server_SendChatMessage_Implementation,
		([](auto& Call, AFGPlayerController* Self, const FString& messageText)
		{
			// Call the original implementation first.
			Call(Self, messageText);

			UGameInstance* GameInstance = Self->GetGameInstance();
			if (!GameInstance) return;

			UDiscordBridgeSubsystem* Bridge =
				GameInstance->GetSubsystem<UDiscordBridgeSubsystem>();
			if (!Bridge) return;

			APlayerState* PlayerState = Self->GetPlayerState<APlayerState>();
			const FString PlayerName = PlayerState
				? PlayerState->GetPlayerName()
				: TEXT("Unknown");

			Bridge->SendGameMessageToDiscord(PlayerName, messageText);
		}));
}

void FDiscordBridgeModule::ShutdownModule()
{
	UNSUBSCRIBE_UOBJECT_METHOD(AFGPlayerController, Server_SendChatMessage_Implementation, GChatHookHandle);
}

IMPLEMENT_MODULE(FDiscordBridgeModule, DiscordBridge)
