// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class SMLWebSocket : ModuleRules
{
	public SMLWebSocket(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
		CppStandard = CppStandardVersion.Cpp20;
		DefaultBuildSettings = BuildSettingsVersion.Latest;
		bLegacyPublicIncludePaths = false;

		PublicDependencyModuleNames.AddRange(new string[]
		{
			"Core",
			"CoreUObject",
			"Engine",
			// Low-level TCP socket access (replaces the missing WebSockets module)
			"Sockets",
			"Networking",
			// JSON serialisation used by the Discord Gateway protocol handler
			"Json",
		});

		PrivateDependencyModuleNames.AddRange(new string[]
		{
			// For SHA-1 (SecureHash.h) and Base64 (Base64.h) used in the WebSocket handshake
			"Core",
		});
	}
}
