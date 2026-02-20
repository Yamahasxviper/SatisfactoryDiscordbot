// Copyright (c) 2024 Yamahasxviper
// Custom WebSocket Plugin Build Configuration

using UnrealBuildTool;

public class CustomWebSocket : ModuleRules
{
	public CustomWebSocket(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
		DefaultBuildSettings = BuildSettingsVersion.V2;

		PublicDependencyModuleNames.AddRange(new string[] {
			"Core",
			"CoreUObject",
			"Engine"
		});

		PrivateDependencyModuleNames.Add("WebSockets");
	}
}
