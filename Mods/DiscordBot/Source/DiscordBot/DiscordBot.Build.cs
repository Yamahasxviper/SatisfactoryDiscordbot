using UnrealBuildTool;
using System.IO;
using System;

public class DiscordBot : ModuleRules
{
	public DiscordBot(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
		CppStandard = CppStandardVersion.Cpp20;
		DefaultBuildSettings = BuildSettingsVersion.Latest;
		bLegacyPublicIncludePaths = false;

		// FactoryGame transitive dependencies
		PublicDependencyModuleNames.AddRange(new string[] {
			"Core", "CoreUObject",
			"Engine",
			"InputCore",
			"SlateCore", "Slate", "UMG",
			"NetCore",
			"GameplayTags",
			"Json", "JsonUtilities"
		});

		// Header stubs
		PublicDependencyModuleNames.Add("DummyHeaders");

		// SML and FactoryGame
		PublicDependencyModuleNames.AddRange(new string[] { "FactoryGame", "SML" });

		// Networking — promoted to Public so any mod that depends on DiscordBot
		// can also include and use UE socket / TLS APIs without re-declaring them.
		PublicDependencyModuleNames.AddRange(new string[]
		{
			"Sockets",   // UE socket abstraction (FSocket / ISocketSubsystem)
			"SSL",       // UE TLS wrapper (ISSLModule / ISSLSocket)
		});

		// OpenSSL is kept Private: only FDiscordWebSocketClient uses it directly.
		// Other mods should go through the higher-level UDiscordBotWebSocket API.
		PrivateDependencyModuleNames.Add("OpenSSL");

		// WinSock2 for raw TCP on Windows (used by the custom WebSocket client)
		if (Target.Platform == UnrealTargetPlatform.Win64)
		{
			PublicSystemLibraries.Add("ws2_32.lib");
		}
	}
}

