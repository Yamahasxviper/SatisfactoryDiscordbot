// Copyright Epic Games, Inc. All Rights Reserved.

#include "SMLWebSocketGameInstanceModule.h"

USMLWebSocketGameInstanceModule::USMLWebSocketGameInstanceModule()
{
	// Mark this as the single root GameInstanceModule for the SMLWebSocket
	// plugin so that SML auto-discovers and instantiates it.
	bRootModule = true;
}
