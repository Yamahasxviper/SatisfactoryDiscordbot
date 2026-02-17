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
        
        // Use Target.ProjectFile to get the project directory reliably
        string projectDir = Target.ProjectFile != null ? Path.GetDirectoryName(Target.ProjectFile.FullName) : Path.GetFullPath(Path.Combine(ModuleDirectory, "..", "..", "..", ".."));
        string WebSocketsPluginPath3 = Path.Combine(projectDir, "Plugins", "WebSockets");
        
        Console.WriteLine("[DiscordChatBridge] Build.cs: Checking for WebSockets plugin...");
        Console.WriteLine($"[DiscordChatBridge]   Path 1 (Engine/Runtime): {WebSocketsPluginPath1} - {(Directory.Exists(WebSocketsPluginPath1) ? "FOUND" : "NOT FOUND")}");
        Console.WriteLine($"[DiscordChatBridge]   Path 2 (Engine/Experimental): {WebSocketsPluginPath2} - {(Directory.Exists(WebSocketsPluginPath2) ? "FOUND" : "NOT FOUND")}");
        Console.WriteLine($"[DiscordChatBridge]   Path 3 (Project/Plugins): {WebSocketsPluginPath3} - {(Directory.Exists(WebSocketsPluginPath3) ? "FOUND" : "NOT FOUND")}");
        
        if (Directory.Exists(WebSocketsPluginPath1) || Directory.Exists(WebSocketsPluginPath2) || Directory.Exists(WebSocketsPluginPath3))
        {
            Console.WriteLine("[DiscordChatBridge] Build.cs: WebSockets plugin FOUND - Enabling WebSocket support");
            PublicDependencyModuleNames.Add("WebSockets");
            PublicDefinitions.Add("WITH_WEBSOCKETS_SUPPORT=1");
        }
        else
        {
            Console.WriteLine("[DiscordChatBridge] Build.cs: WARNING - WebSockets plugin NOT FOUND in any location");
            Console.WriteLine("[DiscordChatBridge] Build.cs: Discord Gateway features will be DISABLED");
            Console.WriteLine("[DiscordChatBridge] Build.cs: The mod will still work with REST API only (no presence/status updates)");
            PublicDefinitions.Add("WITH_WEBSOCKETS_SUPPORT=0");
        }
    }
}
