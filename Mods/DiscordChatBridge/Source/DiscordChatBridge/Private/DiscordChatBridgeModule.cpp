// Copyright (c) 2024 Discord Chat Bridge Contributors

#include "DiscordChatBridgeModule.h"
#include "Patching/NativeHookManager.h"

#define LOCTEXT_NAMESPACE "FDiscordChatBridgeModule"

void FDiscordChatBridgeModule::StartupModule()
{
	// This code will execute after your module is loaded into memory
	UE_LOG(LogTemp, Log, TEXT("DiscordChatBridge: Module Started"));
	
#if WITH_WEBSOCKETS_SUPPORT
	// Pre-load WebSockets module to avoid runtime loading issues on Linux
	UE_LOG(LogTemp, Log, TEXT("DiscordChatBridge: Pre-loading WebSockets module..."));
	if (FModuleManager::Get().ModuleExists(TEXT("WebSockets")))
	{
		if (FModuleManager::Get().LoadModule(TEXT("WebSockets")))
		{
			UE_LOG(LogTemp, Log, TEXT("DiscordChatBridge: WebSockets module loaded successfully at startup"));
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("DiscordChatBridge: Failed to load WebSockets module at startup"));
			UE_LOG(LogTemp, Error, TEXT("DiscordChatBridge: Discord Gateway features will not be available"));
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("DiscordChatBridge: WebSockets module not found"));
		UE_LOG(LogTemp, Warning, TEXT("DiscordChatBridge: Discord Gateway features will not be available"));
	}
#else
	UE_LOG(LogTemp, Log, TEXT("DiscordChatBridge: Compiled without WebSockets support - Gateway features disabled"));
#endif
}

void FDiscordChatBridgeModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module
	UE_LOG(LogTemp, Log, TEXT("DiscordChatBridge: Module Shutdown"));
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FDiscordChatBridgeModule, DiscordChatBridge)
