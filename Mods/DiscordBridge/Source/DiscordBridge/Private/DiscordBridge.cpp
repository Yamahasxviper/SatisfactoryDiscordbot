// Copyright Coffee Stain Studios. All Rights Reserved.

#include "DiscordBridge.h"
#include "Modules/ModuleManager.h"

#include "DiscordBridgeSubsystem.h"
#include "FGChatManager.h"
#include "Engine/GameInstance.h"
#include "Patching/NativeHookManager.h"

void FDiscordBridgeModule::StartupModule()
{
	// Hook AFGChatManager::AddChatMessageToReceived to capture every CMT_PlayerMessage
	// added to the server's chat history.  This is more reliable than the previous
	// OnChatMessageAdded + GetReceivedChatMessages approach, which suffered from:
	//  • Potential timing races where the delegate fires before the struct lands in
	//    mReceivedMessages (or where mReceivedMessages is not populated server-side).
	//  • Rolling-buffer edge cases where the message count never increases.
	//
	// AddChatMessageToReceived is NOT an RPC stub (unlike BroadcastChatMessage /
	// Server_SendChatMessage_Implementation that were "too short" for funchook).
	// It contains real logic (rolling-buffer management + OnChatMessageAdded
	// broadcast), which guarantees the function body is large enough to hook.
	//
	// The lambda looks up the UDiscordBridgeSubsystem dynamically through the
	// ChatManager's world, so no static pointer / friendship is required.
	SUBSCRIBE_UOBJECT_METHOD_AFTER(AFGChatManager, AddChatMessageToReceived,
		[](AFGChatManager* ChatMgr, const FChatMessageStruct& Msg)
		{
			// We only care about player-originated messages.
			if (Msg.MessageType != EFGChatMessageType::CMT_PlayerMessage)
			{
				return;
			}

			UWorld* World = ChatMgr->GetWorld();
			if (!World)
			{
				return;
			}

			UGameInstance* GI = World->GetGameInstance();
			if (!GI)
			{
				return;
			}

			UDiscordBridgeSubsystem* Sub = GI->GetSubsystem<UDiscordBridgeSubsystem>();
			if (!Sub)
			{
				return;
			}

			const FString PlayerName  = Msg.MessageSender.ToString().TrimStartAndEnd();
			const FString MessageText = Msg.MessageText.ToString().TrimStartAndEnd();

			Sub->HandleIncomingChatMessage(PlayerName, MessageText);
		});
}

void FDiscordBridgeModule::ShutdownModule()
{
}

IMPLEMENT_MODULE(FDiscordBridgeModule, DiscordBridge)
