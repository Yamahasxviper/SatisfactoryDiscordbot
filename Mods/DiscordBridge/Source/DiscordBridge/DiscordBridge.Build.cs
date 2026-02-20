// Copyright Coffee Stain Studios. All Rights Reserved.

using UnrealBuildTool;

public class DiscordBridge : ModuleRules
{
	public DiscordBridge(ReadOnlyTargetRules Target) : base(Target)
	{
		CppStandard = CppStandardVersion.Cpp20;
		DefaultBuildSettings = BuildSettingsVersion.Latest;
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
		bLegacyPublicIncludePaths = false;

		PublicDependencyModuleNames.AddRange(new string[]
		{
			"Core",
			"CoreUObject",
			"Engine",
			// Header stubs for APIs not present in Satisfactory's custom UE build.
			// Required by all Alpakit C++ mods so UBT can resolve engine headers at mod compile time.
			"DummyHeaders",
			// SML runtime dependency – ensures correct module load ordering
			"SML",
			// Custom WebSocket client with SSL/OpenSSL support
			"SMLWebSocket",
			// HTTP client for Discord REST API
			"HTTP",
			// JSON serialization/deserialization
			"Json",
			"JsonUtilities",
			// Developer settings for config class
			"DeveloperSettings",
		});
	}
}
