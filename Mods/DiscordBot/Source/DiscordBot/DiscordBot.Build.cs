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
            "WebSockets"
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

        // SocketIO Client Plugin dependency (alternative option)
        PublicDependencyModuleNames.AddRange(new[] {
            "SocketIOClient",
            "SocketIOLib",
            "SIOJson"
        });
    }
}
