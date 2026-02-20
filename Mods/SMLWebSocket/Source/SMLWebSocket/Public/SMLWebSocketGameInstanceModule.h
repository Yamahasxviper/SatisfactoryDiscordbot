// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Module/GameInstanceModule.h"
#include "SMLWebSocketGameInstanceModule.generated.h"

/**
 * Root GameInstanceModule for the SMLWebSocket plugin.
 *
 * SML auto-discovers this class because bRootModule = true (set in the constructor).
 */
UCLASS()
class SMLWEBSOCKET_API USMLWebSocketGameInstanceModule : public UGameInstanceModule
{
	GENERATED_BODY()

public:
	USMLWebSocketGameInstanceModule();
};
