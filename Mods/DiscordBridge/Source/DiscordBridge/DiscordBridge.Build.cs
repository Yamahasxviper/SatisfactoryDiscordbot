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
			// FactoryGame – provides AFGChatManager, AFGPlayerController, etc.
			"FactoryGame",
			// SML runtime dependency – ensures correct module load ordering.
			"SML",
			// Unreal HTTP module – confirmed present in Satisfactory's custom UE build.
			// Used for both Discord REST API polling (receiving messages) and posting.
			// Verified: FactoryGame.Build.cs lists "HTTP" in PublicDependencyModuleNames.
			"HTTP",
			// Unreal JSON serialisation module (FJsonObject / TJsonReader / TJsonWriter /
			// FJsonSerializer) – confirmed present in Satisfactory's custom UE build.
			"Json",
		});
	}
}
