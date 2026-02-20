using UnrealBuildTool;
using System.IO;
using System;

public class DiscordBot : ModuleRules
{
	public DiscordBot(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
		CppStandard = CppStandardVersion.Cpp20;

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

		// Networking — all three layers are included so the mod can use
		// whichever networking path is available at runtime:
		//   • Sockets  — UE platform-agnostic TCP/UDP socket API
		//   • SSL      — UE TLS wrapper (ISSLSocket / ISSLModule)
		//   • OpenSSL  — direct OpenSSL calls used by the custom WebSocket client
		PrivateDependencyModuleNames.AddRange(new string[]
		{
			"Sockets",   // UE socket abstraction (FSocket / ISocketSubsystem)
			"SSL",       // UE TLS wrapper (ISSLModule / ISSLSocket)
			"OpenSSL",   // Raw OpenSSL — used by FDiscordWebSocketClient directly
		});

		// WinSock2 for raw TCP on Windows (used by the custom WebSocket client)
		if (Target.Platform == UnrealTargetPlatform.Win64)
		{
			PublicSystemLibraries.Add("ws2_32.lib");
		}
	}
}

