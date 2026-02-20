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

		// WebSockets is only needed internally; keeping it private avoids
		// forcing downstream dependents (e.g. DiscordBot) to link against
		// UnrealEditor-WebSockets.lib, which may not be present in the dev kit.
		PrivateDependencyModuleNames.AddRange(new string[] {
			"WebSockets"
		});
	}
}
