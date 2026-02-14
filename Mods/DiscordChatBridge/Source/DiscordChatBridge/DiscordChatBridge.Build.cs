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
        
        // WebSockets is optional - only add if the plugin is available in the engine
        // The plugin is located in Engine/Plugins/Runtime/WebSockets or Engine/Plugins/Experimental/WebSockets
        string WebSocketsPluginPath1 = Path.Combine(EngineDirectory, "Plugins", "Runtime", "WebSockets");
        string WebSocketsPluginPath2 = Path.Combine(EngineDirectory, "Plugins", "Experimental", "WebSockets");
        
        if (Directory.Exists(WebSocketsPluginPath1) || Directory.Exists(WebSocketsPluginPath2))
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
