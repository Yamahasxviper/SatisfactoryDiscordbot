// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class BuildSettings : ModuleRules
{
    public BuildSettings(ReadOnlyTargetRules Target) : base(Target)
    {
        // This module is required to fix the "Missing precompiled manifest for 'BuildSettings'" error
        // when building with an installed engine build (e.g., UE 5.3.2-CSS).
        // 
        // The BuildSettings module is an engine module that may not have precompiled manifests
        // in installed builds. By creating this plugin module and setting
        // PrecompileForTargets = PrecompileTargetsType.Any, we tell UBT to compile this module
        // from source rather than expecting precompiled binaries.
        //
        // bPrecompile = false explicitly tells UBT not to expect precompiled binaries for this module.
        //
        // Note: This is implemented as a plugin (not a project module) to avoid module hierarchy
        // violations where Engine modules would appear to reference Project modules.
        // The LoadingPhase is set to "PostConfigInit" in BuildSettings.uplugin to ensure this
        // module loads at engine initialization level, preventing UBT from categorizing it as a
        // standard project module which would trigger hierarchy violations.
        
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
        PrecompileForTargets = PrecompileTargetsType.Any;
        bPrecompile = false;
        
        PublicDependencyModuleNames.AddRange(new string[] {
            "Core",
            "CoreUObject",
            "Engine"
        });
    }
}
