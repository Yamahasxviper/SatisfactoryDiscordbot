// Copyright Coffee Stain Studios. All Rights Reserved.

using UnrealBuildTool;

public class LogViewer : ModuleRules
{
	public LogViewer(ReadOnlyTargetRules Target) : base(Target)
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
			// SML runtime dependency – ensures correct module load ordering.
			"SML",
		});

		PrivateDependencyModuleNames.AddRange(new string[]
		{
			// Unreal HTTP server module – provides IHttpRouter / FHttpServerModule used to
			// serve the log viewer endpoint.  Confirmed present in Satisfactory's UE build:
			// FactoryDedicatedServer.Build.cs lists it as a private dependency.
			"HTTPServer",
			// JSON serialisation (FJsonObject / TJsonReader) used by the config loader.
			"Json",
		});
	}
}
