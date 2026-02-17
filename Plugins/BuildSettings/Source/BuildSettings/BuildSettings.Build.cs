// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class BuildSettings : ModuleRules
{
	public BuildSettings(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
		
		// Explicitly tell UBT not to expect precompiled binaries, forcing compilation from source
		// This is needed when using installed engine builds that may not have precompiled manifests
		// for all platforms and configurations
		bPrecompile = false;

		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core"
			}
		);

		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"CoreUObject",
				"Engine"
			}
		);
	}
}
