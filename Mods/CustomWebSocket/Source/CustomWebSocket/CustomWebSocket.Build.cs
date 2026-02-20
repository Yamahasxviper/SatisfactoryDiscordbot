// Copyright (c) 2024 Yamahasxviper
// Custom WebSocket Plugin Build Configuration

using UnrealBuildTool;

public class CustomWebSocket : ModuleRules
{
	public CustomWebSocket(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
		DefaultBuildSettings = BuildSettingsVersion.V2;

		PublicDependencyModuleNames.AddRange(new string[] {
			"Core",
			"CoreUObject",
			"Engine"
		});

		// WebSockets headers (IWebSocket.h, WebSocketsModule.h) are needed for
		// compilation only. We do NOT link against UnrealEditor-WebSockets.lib
		// because it may be absent in the Satisfactory dev kit (LNK1181).
		// All WebSockets API calls go through virtual dispatch:
		//   - FWebSocketsModule::Get() is an inline function (no lib needed)
		//   - FWebSocketsModule::CreateWebSocket() is pure virtual
		//   - All IWebSocket methods are pure virtual
		// The module is loaded at runtime via FModuleManager in CustomWebSocket.cpp.
		PrivateIncludePathModuleNames.Add("WebSockets");
		DynamicallyLoadedModuleNames.Add("WebSockets");
	}
}
