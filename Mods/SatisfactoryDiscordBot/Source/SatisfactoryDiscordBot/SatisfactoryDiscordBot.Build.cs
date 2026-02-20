// Copyright SatisfactoryDiscordbot. All Rights Reserved.

using UnrealBuildTool;

public class SatisfactoryDiscordBot : ModuleRules
{
	public SatisfactoryDiscordBot(ReadOnlyTargetRules Target) : base(Target)
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
			// Raw TCP socket access – used by the custom WebSocket implementation
			// because the CSS custom engine does not ship the WebSockets module.
			"Sockets",
		});

		PrivateDependencyModuleNames.AddRange(new string[]
		{
			"SML",
			"FactoryGame",
		});
	}
}
