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
	// Hook AFGChatManager::BroadcastChatMessage to forward player chat messages to
	// Discord.  This is the server-side broadcast that fires for every chat message
	// regardless of how it originated, so it works whether the message came from the
	// in-game chat UI or from a console command.
	// We filter to CMT_PlayerMessage only to avoid echo-looping our own Discord→game
	// relay messages (which are posted as CMT_SystemMessage).
	GChatHookHandle = SUBSCRIBE_UOBJECT_METHOD(AFGChatManager, BroadcastChatMessage,
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
	UNSUBSCRIBE_UOBJECT_METHOD(AFGChatManager, BroadcastChatMessage, GChatHookHandle);
}

IMPLEMENT_MODULE(FDiscordBridgeModule, DiscordBridge)
