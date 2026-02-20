// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Module/GameWorldModule.h"
#include "DiscordBotWorldModule.generated.h"

/**
 * Root game-world module for the DiscordBot mod.
 *
 * SML discovers this class automatically (it extends UGameWorldModule and sets
 * bRootModule = true) and calls DispatchLifecycleEvent during world load, which
 * in turn registers ADiscordBotSubsystem via the parent class machinery.
 */
UCLASS()
class DISCORDBOT_API UDiscordBotWorldModule : public UGameWorldModule
{
	GENERATED_BODY()

public:
	UDiscordBotWorldModule();
};
