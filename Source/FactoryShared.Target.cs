// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.

using EpicGames.Core;
using UnrealBuildTool;
using Microsoft.Extensions.Logging;

// Target preset class shared by both game and server configurations. Editor still has it's own configuration not derived from this class.
public class FactorySharedTarget : TargetRules
{
	/** Allows overriding whenever the checks in the shipping builds should be used from the commandline */
	[CommandLine("-UseChecksInShipping")]
	public bool UseChecksInShippingOverride = true; // MODDING EDIT: we want checks in shipping by default

	/** Allows overriding the build type from the command line */
	[CommandLine("-Monolithic", Value = "Monolithic")] 
	[CommandLine("-Modular", Value = "Modular")]
	public TargetLinkType LinkTypeOverride = TargetLinkType.Default;

	public FactorySharedTarget(TargetInfo Target) : base(Target)
	{
		// Marked this as game because the project file generator was trying to initialize this class and failing because it was abstract. What a not very smart tool.
		Type = UnrealBuildTool.TargetType.Game;
		
		// Default to shared build environment and modular build on desktop platforms
		// Exception: Use Monolithic for Linux Server builds to avoid runtime .so loading issues
		var defaultLinkTypeForPlatform = Target.Platform.IsInGroup(UnrealPlatformGroup.Desktop) ? TargetLinkType.Modular : TargetLinkType.Monolithic;
		
		// Override to Monolithic for Linux Server builds to statically link all modules (including WebSockets)
		// This prevents "dlopen failed: libFactoryServer-WebSockets-Linux-Shipping.so" errors
		// Check Target.Name instead of Type because Type gets set in derived class constructor (after this runs)
		if (Target.Platform == UnrealTargetPlatform.Linux && Target.Name.Contains("Server"))
		{
			defaultLinkTypeForPlatform = TargetLinkType.Monolithic;
		}

		BuildEnvironment = TargetBuildEnvironment.Shared;
		LinkType = LinkTypeOverride != TargetLinkType.Default ? LinkTypeOverride : defaultLinkTypeForPlatform;

		// Shared build settings
		IncludeOrderVersion = EngineIncludeOrderVersion.Latest;
		bOverrideAppNameForSharedBuild = true;
		bOverrideBuildEnvironment = true;
		bUseLoggingInShipping = true;
		bAllowGeneratedIniWhenCooked = true;
		// Enable diagnostics when disabled plugins are being pulled back into the build as dependencies for other plugins
		DisablePluginsConflictWarningLevel = WarningLevel.Warning;

		// [ZolotukhinN:10/07/2023] Enabled Network Push Model support in normal game builds, it's disabled in non-editor by default
		bWithPushModel = true;
		// [ZolotukhinN:04/05/2023] Allow cheat manager initialization in Shipping builds to allow cheating even in shipping builds when compiling with WITH_CHEATS=1
		if (true /*Target.Platform != UnrealTargetPlatform.PS5 && Target.Platform != UnrealTargetPlatform.XSX*/)
		{
		GlobalDefinitions.Add("UE_WITH_CHEAT_MANAGER=1");
		}
		// Allow checks in shipping depending on the command line configuration
		bUseChecksInShipping = UseChecksInShippingOverride;
		
		// Disable precompiled headers when using installed engine builds to avoid
		// "Missing precompiled manifest for 'BuildSettings'" error on Linux builds
		// This is required when building with -installed flag in RunUAT
		if (Target.bUsePrecompiled)
		{
			bPrecompile = false;
		}
		
		// Common module names for the game targets
		ExtraModuleNames.AddRange(new[] {
			"FactoryGame"
		});
		if (UseChecksInShippingOverride || LinkTypeOverride != TargetLinkType.Default)
		{
			Logger.LogInformation($"{Target.Name}: Build configuration: ChecksInShipping: {bUseChecksInShipping} (default: true); LinkType: {LinkType} (default: {defaultLinkTypeForPlatform})");
		}
	}
}
