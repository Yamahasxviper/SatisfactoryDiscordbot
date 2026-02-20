// Copyright SatisfactoryDiscordbot. All Rights Reserved.

#include "DiscordBotWorldModule.h"
#include "Module/WorldModuleManager.h"
#include "SatisfactoryDiscordBotModule.h"

UDiscordBotWorldModule::UDiscordBotWorldModule()
{
	// Mark this as the root module so SML auto-discovers and instantiates it
	// for every game world without requiring any manual registration.
	bRootModule = true;
}

void UDiscordBotWorldModule::DispatchLifecycleEvent(ELifecyclePhase Phase)
{
	// Always call Super first so UGameWorldModule can register schematics,
	// research trees, chat commands, etc. that Blueprint subclasses may have set.
	Super::DispatchLifecycleEvent(Phase);

	if (Phase == ELifecyclePhase::INITIALIZATION)
	{
		// Create the server UObject owned by this module.
		WebSocketServer = NewObject<UCustomWebSocketServer>(this);

		// Apply TLS configuration (set in Blueprint Class Defaults) before starting.
		WebSocketServer->bUseTLS           = bUseTLS;
		WebSocketServer->TLSCertificatePath = TLSCertificatePath;
		WebSocketServer->TLSPrivateKeyPath  = TLSPrivateKeyPath;

		if (WebSocketServer->StartListening(WebSocketPort))
		{
			UE_LOG(LogSatisfactoryDiscordBot, Log,
				TEXT("DiscordBotWorldModule: WebSocket server started on port %d"), WebSocketPort);
		}
		else
		{
			UE_LOG(LogSatisfactoryDiscordBot, Error,
				TEXT("DiscordBotWorldModule: Failed to start WebSocket server on port %d"), WebSocketPort);
		}
	}
}

void UDiscordBotWorldModule::BeginDestroy()
{
	if (WebSocketServer)
	{
		WebSocketServer->StopListening();
		WebSocketServer = nullptr;
	}

	Super::BeginDestroy();
}

UDiscordBotWorldModule* UDiscordBotWorldModule::GetModule(UObject* WorldContext)
{
	if (!WorldContext)
	{
		return nullptr;
	}

	const UWorld* World = GEngine->GetWorldFromContextObject(
		WorldContext, EGetWorldErrorMode::ReturnNull);
	if (!World)
	{
		return nullptr;
	}

	UWorldModuleManager* ModuleManager =
		World->GetSubsystem<UWorldModuleManager>();
	if (!ModuleManager)
	{
		return nullptr;
	}

	UWorldModule* Module =
		ModuleManager->FindModule(TEXT("SatisfactoryDiscordBot"));

	return Cast<UDiscordBotWorldModule>(Module);
}
