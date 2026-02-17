// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.

#include "BuildSettings.h"

#define LOCTEXT_NAMESPACE "FBuildSettingsModule"

void FBuildSettingsModule::StartupModule()
{
    // This plugin module is intentionally minimal. It exists only to provide a precompile target
    // override via BuildSettings.Build.cs to fix the missing precompiled manifest error.
}

void FBuildSettingsModule::ShutdownModule()
{
    // No cleanup needed
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FBuildSettingsModule, BuildSettings)
