// Copyright (c) 2024 Yamahasxviper

using UnrealBuildTool;
using System.IO;

public class DiscordBot : ModuleRules
{
    public DiscordBot(ReadOnlyTargetRules Target) : base(Target)
    {
        CppStandard = CppStandardVersion.Cpp20;
        DefaultBuildSettings = BuildSettingsVersion.Latest;
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
        bLegacyPublicIncludePaths = false;

        // Core Unreal dependencies
        PublicDependencyModuleNames.AddRange(new[] {
            "Core",
            "CoreUObject",
            "Engine",
            "Json",
            "JsonUtilities",
            "HTTP",
            "Sockets",        // For ISocketSubsystem (FCustomWebSocket)
            "Networking",     // For networking utilities
            "OpenSSL"         // For TLS/SSL support (wss://)
        });

        // FactoryGame and SML dependencies
        PublicDependencyModuleNames.AddRange(new[] {
            "FactoryGame",
            "SML"
        });

        // Custom WebSocket Plugin (for platform-agnostic WebSocket support)
        PublicDependencyModuleNames.AddRange(new[] {
            "CustomWebSocket"
        });
    }
}
