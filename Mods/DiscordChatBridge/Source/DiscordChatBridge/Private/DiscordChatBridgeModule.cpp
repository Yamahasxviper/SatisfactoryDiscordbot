// Copyright (c) 2024 Discord Chat Bridge Contributors

#include "DiscordChatBridgeModule.h"
#include "Patching/NativeHookManager.h"

#define LOCTEXT_NAMESPACE "FDiscordChatBridgeModule"

void FDiscordChatBridgeModule::StartupModule()
{
	// This code will execute after your module is loaded into memory
	UE_LOG(LogTemp, Log, TEXT("DiscordChatBridge: Module Started"));
}

void FDiscordChatBridgeModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module
	UE_LOG(LogTemp, Log, TEXT("DiscordChatBridge: Module Shutdown"));
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FDiscordChatBridgeModule, DiscordChatBridge)
