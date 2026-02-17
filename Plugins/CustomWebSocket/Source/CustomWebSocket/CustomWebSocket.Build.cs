// Copyright (c) 2024 Yamahasxviper
// Custom WebSocket Plugin Build Configuration

using UnrealBuildTool;
using System.IO;

public class CustomWebSocket : ModuleRules
{
	public CustomWebSocket(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
		
		// Required for cross-platform compilation
		bEnableExceptions = false;
		bUseRTTI = false;

		PublicDependencyModuleNames.AddRange(new string[] {
			"Core",
			"CoreUObject",
			"Engine",
			"Sockets",        // For TCP socket implementation
			"Networking",     // For networking utilities
			"OpenSSL"         // For TLS/SSL support (wss://)
		});

		PrivateDependencyModuleNames.AddRange(new string[] {
			// Add private dependencies here if needed
		});

		// Platform-specific includes
		if (Target.Platform == UnrealTargetPlatform.Win64)
		{
			// Windows-specific
			PublicDefinitions.Add("PLATFORM_WINDOWS=1");
		}
		else if (Target.Platform == UnrealTargetPlatform.Linux)
		{
			// Linux-specific
			PublicDefinitions.Add("PLATFORM_LINUX=1");
		}
		else if (Target.Platform == UnrealTargetPlatform.Mac)
		{
			// Mac-specific
			PublicDefinitions.Add("PLATFORM_MAC=1");
		}
	}
}
