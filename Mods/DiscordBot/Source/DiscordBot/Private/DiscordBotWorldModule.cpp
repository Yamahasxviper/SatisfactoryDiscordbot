// Copyright Epic Games, Inc. All Rights Reserved.

#include "DiscordBotWorldModule.h"
#include "DiscordBotSubsystem.h"

UDiscordBotWorldModule::UDiscordBotWorldModule()
{
	// Mark this as the single root module for the DiscordBot plugin.
	bRootModule = true;

	// Register our server-side subsystem so SML spawns it during world init.
	ModSubsystems.Add(ADiscordBotSubsystem::StaticClass());
}
