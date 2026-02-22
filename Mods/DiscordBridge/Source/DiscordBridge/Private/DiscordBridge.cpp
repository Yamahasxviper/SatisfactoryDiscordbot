// Copyright Coffee Stain Studios. All Rights Reserved.

#include "DiscordBridge.h"
#include "DiscordBridgeSubsystem.h"
#include "Modules/ModuleManager.h"
#include "Patching/NativeHookManager.h"
#include "FGChatManager.h"

// Stored so the hook can be removed cleanly in ShutdownModule.
static FDelegateHandle GChatHookHandle;

void FDiscordBridgeModule::StartupModule()
{
	// Hook AFGChatManager::AddChatMessageToReceived to forward player chat messages to
	// Discord.  Both BroadcastChatMessage and Multicast_BroadcastChatMessage_Implementation
	// are too short for funchook to instrument ("Too short instructions"), so we hook
	// AddChatMessageToReceived instead.  This function has a larger body (TArray append)
	// that funchook can safely instrument.  The sender name is carried directly in the
	// FChatMessageStruct::MessageSender field, so no InstigatorController is needed.
	// We filter to CMT_PlayerMessage only to avoid echo-looping our own Discord→game
	// relay messages (which are posted as CMT_SystemMessage).
	GChatHookHandle = SUBSCRIBE_UOBJECT_METHOD(AFGChatManager, AddChatMessageToReceived,
		([](auto& Call, AFGChatManager* Self, const FChatMessageStruct& inMessage)
		{
			// Call the original implementation first.
			Call(Self, inMessage);

			// Only forward player-typed messages to Discord.
			if (inMessage.MessageType != EFGChatMessageType::CMT_PlayerMessage) return;

			UGameInstance* GameInstance = Self->GetGameInstance();
			if (!GameInstance) return;

			UDiscordBridgeSubsystem* Bridge =
				GameInstance->GetSubsystem<UDiscordBridgeSubsystem>();
			if (!Bridge) return;

			const FString PlayerName = inMessage.MessageSender.ToString();
			const FString MessageText = inMessage.MessageText.ToString();

			Bridge->SendGameMessageToDiscord(PlayerName, MessageText);
		}));
}

void FDiscordBridgeModule::ShutdownModule()
{
	UNSUBSCRIBE_UOBJECT_METHOD(AFGChatManager, AddChatMessageToReceived, GChatHookHandle);
}

IMPLEMENT_MODULE(FDiscordBridgeModule, DiscordBridge)
