// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleInterface.h"
#include "Modules/ModuleManager.h"

/**
 * BuildSettings module
 * This module exists to provide build configuration settings for the Unreal Build Tool.
 * It configures PrecompileForTargets to ensure the module can be compiled for all target types.
 */
class FBuildSettingsModule : public IModuleInterface
{
public:
	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
};
