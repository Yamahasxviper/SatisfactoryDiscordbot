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

		// Platform-specific configurations
		// Note: Platform-specific defines are already provided by Unreal Engine
		// (e.g., PLATFORM_WINDOWS, PLATFORM_LINUX, PLATFORM_MAC, PLATFORM_ANDROID, PLATFORM_IOS)
		// We use Unreal's cross-platform socket subsystem which works on all platforms
		
		// Additional platform-specific settings can be added here if needed
		if (Target.Platform == UnrealTargetPlatform.Win64)
		{
			// Windows-specific settings
		}
		else if (Target.Platform == UnrealTargetPlatform.Linux)
		{
			// Linux-specific settings
		}
		else if (Target.Platform == UnrealTargetPlatform.Mac)
		{
			// Mac-specific settings
		}
		else if (Target.Platform == UnrealTargetPlatform.Android)
		{
			// Android-specific settings
		}
		else if (Target.Platform == UnrealTargetPlatform.IOS)
		{
			// iOS-specific settings
		}
	}
}
