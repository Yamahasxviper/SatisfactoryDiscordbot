// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.

#include "BuildSettingsModule.h"

#define LOCTEXT_NAMESPACE "FBuildSettingsModule"

void FBuildSettingsModule::StartupModule()
{
	// This module is intentionally minimal - it exists primarily to configure build settings
	// No runtime functionality is needed
}

void FBuildSettingsModule::ShutdownModule()
{
	// No cleanup needed
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FBuildSettingsModule, BuildSettings)
