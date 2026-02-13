// Copyright (c) 2024 Discord Chat Bridge Contributors

#include "DiscordChatGameInstanceModule.h"

void UDiscordChatGameInstanceModule::DispatchLifecycleEvent(ELifecyclePhase Phase)
{
	Super::DispatchLifecycleEvent(Phase);
	
	if (Phase == ELifecyclePhase::CONSTRUCTION)
	{
		UE_LOG(LogTemp, Log, TEXT("DiscordChatBridge: GameInstanceModule initialized"));
	}
}
