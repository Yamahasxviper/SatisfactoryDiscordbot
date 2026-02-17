// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

/**
 * BuildSettings module
 * 
 * This module exists to provide a precompile target override for the engine's BuildSettings module.
 * It fixes the "Missing precompiled manifest for 'BuildSettings'" error that occurs when building
 * with installed engine builds that don't include precompiled manifests for all engine modules.
 */
class FBuildSettingsModule : public IModuleInterface
{
public:
    /** IModuleInterface implementation */
    virtual void StartupModule() override;
    virtual void ShutdownModule() override;
};
