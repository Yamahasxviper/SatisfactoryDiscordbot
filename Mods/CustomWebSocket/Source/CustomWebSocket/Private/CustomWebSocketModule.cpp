// Copyright (c) 2024 Yamahasxviper
// Custom WebSocket Plugin Module Implementation

#include "CustomWebSocketModule.h"
#include "CustomWebSocket.h"
#include "Modules/ModuleManager.h"

#define LOCTEXT_NAMESPACE "FCustomWebSocketModule"

void FCustomWebSocketModule::StartupModule()
{
	// This code will execute after your module is loaded into memory
	UE_LOG(LogCustomWebSocket, Log, TEXT("CustomWebSocket module loaded"));
}

void FCustomWebSocketModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module
	UE_LOG(LogCustomWebSocket, Log, TEXT("CustomWebSocket module unloaded"));
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FCustomWebSocketModule, CustomWebSocket)
