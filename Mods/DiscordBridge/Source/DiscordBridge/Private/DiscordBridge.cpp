// Copyright Coffee Stain Studios. All Rights Reserved.

#include "DiscordBridge.h"
#include "Modules/ModuleManager.h"

void FDiscordBridgeModule::StartupModule()
{
	// Chat interception is handled by UDiscordBridgeSubsystem::OnNewChatMessage(),
	// which binds to AFGChatManager::OnChatMessageAdded once the world is ready.
	// This avoids funchook ("Too short instructions") issues with the short RPC stubs
	// (Server_SendChatMessage_Implementation, BroadcastChatMessage, etc.).
}

void FDiscordBridgeModule::ShutdownModule()
{
}

IMPLEMENT_MODULE(FDiscordBridgeModule, DiscordBridge)
