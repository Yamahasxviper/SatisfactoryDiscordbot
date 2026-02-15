// Copyright (c) 2024 Discord Chat Bridge Contributors

#include "DiscordChatGameWorldModule.h"
#include "DiscordChatSubsystem.h"

UDiscordChatGameWorldModule::UDiscordChatGameWorldModule()
{
	// Mark this as a root module so it's discovered by SML
	bRootModule = true;
	
	// Register the Discord chat subsystem
	ModSubsystems.Add(ADiscordChatSubsystem::StaticClass());
}
