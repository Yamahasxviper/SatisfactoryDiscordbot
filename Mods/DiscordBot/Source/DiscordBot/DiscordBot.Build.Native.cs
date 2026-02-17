// Copyright (c) 2024 Yamahasxviper
// Alternative Build Configuration using Native WebSockets module

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
            "WebSockets"  // Native Unreal WebSocket module - Compatible with CSS UE 5.3.2
        });

        // FactoryGame and SML dependencies
        PublicDependencyModuleNames.AddRange(new[] {
            "FactoryGame",
            "SML"
        });

        // OPTION 1: Use Native WebSockets (RECOMMENDED for CSS UE 5.3.2)
        // - Guaranteed compatibility with custom engine builds
        // - Proper WebSocket protocol for Discord Gateway
        // - No third-party dependencies
        // This is the default and recommended approach.

        // OPTION 2: Use SocketIO Client Plugin (NOT RECOMMENDED for Discord)
        // - Only enable if you specifically need Socket.IO protocol (not Discord Gateway)
        // - Discord Gateway requires native WebSocket, not Socket.IO
        // Uncomment these lines only if you need Socket.IO:
        /*
        PublicDependencyModuleNames.AddRange(new[] {
            "SocketIOClient",
            "SocketIOLib",
            "SIOJson"
        });
        */
    }
}
