using UnrealBuildTool;
using System.IO;
using System;

public class DiscordChatBridge : ModuleRules
{
    public DiscordChatBridge(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
        
        PublicDependencyModuleNames.AddRange(new string[] {
            "Core",
            "CoreUObject",
            "Engine",
            "InputCore",
            "OnlineSubsystem",
            "OnlineSubsystemUtils",
            "AssetRegistry",
            "NavigationSystem",
            "ReplicationGraph",
            "AIModule",
            "GameplayTags",
            "Json",
            "JsonUtilities",
            "HTTP",
            "WebSocketNetworking",
            "Sockets",
            "FactoryGame",
            "SML"
        });
    }
}
