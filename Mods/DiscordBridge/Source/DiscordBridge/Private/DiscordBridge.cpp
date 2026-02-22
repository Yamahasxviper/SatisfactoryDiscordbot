// Copyright Coffee Stain Studios. All Rights Reserved.

#include "DiscordBridge.h"
#include "DiscordBridgeSubsystem.h"
#include "Modules/ModuleManager.h"
#include "Patching/NativeHookManager.h"
#include "FGChatManager.h"
#include "GameFramework/PlayerState.h"

// Stored so the hook can be removed cleanly in ShutdownModule.
static FDelegateHandle GChatHookHandle;

void FDiscordBridgeModule::StartupModule()
{
	// Hook AFGChatManager::Multicast_BroadcastChatMessage_Implementation to forward
	// player chat messages to Discord.  BroadcastChatMessage itself is too short for
	// funchook to instrument ("Too short instructions"), so we hook the NetMulticast
	// implementation instead.  On the server UE executes the _Implementation locally
	// before replicating to clients, so every message is still captured exactly once.
	// We filter to CMT_PlayerMessage only to avoid echo-looping our own Discord→game
	// relay messages (which are posted as CMT_SystemMessage).
	GChatHookHandle = SUBSCRIBE_UOBJECT_METHOD(AFGChatManager, Multicast_BroadcastChatMessage_Implementation,
		([](auto& Call, AFGChatManager* Self,
		    const FChatMessageStruct& NewMessage,
		    APlayerController* InstigatorController)
		{
			// Call the original implementation first.
			Call(Self, NewMessage, InstigatorController);

			// Only forward player-typed messages to Discord.
			if (NewMessage.MessageType != EFGChatMessageType::CMT_PlayerMessage) return;
			if (!InstigatorController) return;

			UGameInstance* GameInstance = InstigatorController->GetGameInstance();
			if (!GameInstance) return;

			UDiscordBridgeSubsystem* Bridge =
				GameInstance->GetSubsystem<UDiscordBridgeSubsystem>();
			if (!Bridge) return;

			APlayerState* PS = InstigatorController->GetPlayerState<APlayerState>();
			const FString PlayerName = PS ? PS->GetPlayerName() : TEXT("Unknown Player");
			const FString MessageText = NewMessage.MessageText.ToString();

			Bridge->SendGameMessageToDiscord(PlayerName, MessageText);
		}));
}

void FDiscordBridgeModule::ShutdownModule()
{
	UNSUBSCRIBE_UOBJECT_METHOD(AFGChatManager, Multicast_BroadcastChatMessage_Implementation, GChatHookHandle);
}

IMPLEMENT_MODULE(FDiscordBridgeModule, DiscordBridge)
