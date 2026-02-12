// Copyright (c) 2024 Discord Chat Bridge Contributors

#include "DiscordChatGameInstanceModule.h"
#include "DiscordChatSubsystem.h"

void UDiscordChatGameInstanceModule::DispatchLifecycleEvent(ELifecyclePhase Phase)
{
	Super::DispatchLifecycleEvent(Phase);
	
	if (Phase == ELifecyclePhase::CONSTRUCTION)
	{
		UE_LOG(LogTemp, Log, TEXT("DiscordChatBridge: Registering subsystem"));
		
		// Register the Discord chat subsystem
		ModSubsystems.Add(ADiscordChatSubsystem::StaticClass());
	}
}
