using UnrealBuildTool;

public class SatisfactoryDiscordBot : ModuleRules
{
	public SatisfactoryDiscordBot(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
		CppStandard = CppStandardVersion.Cpp20;
		DefaultBuildSettings = BuildSettingsVersion.Latest;
		bLegacyPublicIncludePaths = false;

		PublicDependencyModuleNames.AddRange(new string[] {
			"Core",
			"CoreUObject",
			"Engine",
			"WebSocketNetworking",
		});

		PrivateDependencyModuleNames.AddRange(new string[] {
			"SML",
			"Projects",
		});
	}
}
