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
            "FactoryGame",
            "SML"
        });
        
        // WebSockets is optional - only add if the plugin is available
        // Check both engine and project plugin locations
        string WebSocketsPluginPath1 = Path.Combine(EngineDirectory, "Plugins", "Runtime", "WebSockets");
        string WebSocketsPluginPath2 = Path.Combine(EngineDirectory, "Plugins", "Experimental", "WebSockets");
        string WebSocketsPluginPath3 = Path.Combine(ModuleDirectory, "..", "..", "..", "..", "Plugins", "WebSockets");
        
        if (Directory.Exists(WebSocketsPluginPath1) || Directory.Exists(WebSocketsPluginPath2) || Directory.Exists(WebSocketsPluginPath3))
        {
            PublicDependencyModuleNames.Add("WebSockets");
            PublicDefinitions.Add("WITH_WEBSOCKETS_SUPPORT=1");
        }
        else
        {
            PublicDefinitions.Add("WITH_WEBSOCKETS_SUPPORT=0");
        }
    }
}
