# WebSocketNetworking Dependency Explanation

## Question: Why use WebSocketNetworking instead of WebSockets?

**Answer:** The mod now uses the **WebSocketNetworking** plugin from the Plugins folder instead of the built-in WebSockets module.

## Why WebSocketNetworking?

### 1. Local Plugin Control

The WebSocketNetworking plugin is available in the project's Plugins folder:
- Allows for local modifications and customizations
- Provides access to libwebsockets for advanced features
- Can be version controlled with the project

### 2. WebSocketNetworking Configuration

The WebSocketNetworking dependency is properly configured in three places:

#### a) Project Level - `FactoryGame.uproject`
```json
{
    "Name": "WebSocketNetworking",
    "Enabled": true
}
```
This enables the WebSocketNetworking plugin for the entire Unreal Engine project.

#### b) Mod Plugin Level - `DiscordChatBridge.uplugin`
```json
{
    "Name": "WebSocketNetworking",
    "Enabled": true
}
```
This declares that the DiscordChatBridge mod requires the WebSocketNetworking plugin.

#### c) Mod Module Level - `DiscordChatBridge.Build.cs`
```csharp
PublicDependencyModuleNames.AddRange(new string[] {
    // ... other dependencies ...
    "WebSocketNetworking",
    "Sockets",
    // ... more dependencies ...
});
```
This tells Unreal Build Tool that the DiscordChatBridge module needs to link against the WebSocketNetworking module.

### 3. FactoryGame Doesn't Use WebSocketNetworking

The FactoryGame module itself does not use any WebSocketNetworking functionality:
- No `#include "INetworkingWebSocket.h"` or similar includes
- No `IWebSocketNetworkingModule::Get()` calls
- No WebSocket-related code

Only the **DiscordChatBridge mod** uses WebSocketNetworking for the Discord Gateway connection.

### 4. Proper Dependency Isolation

In Unreal Engine modding:
- The **base game module** (FactoryGame) should only include dependencies it directly uses
- **Mods** (like DiscordChatBridge) declare their own dependencies independently
- This keeps dependencies clean and prevents unnecessary coupling

## How WebSocketNetworking is Used

The DiscordChatBridge mod uses WebSocketNetworking for:

1. **Discord Gateway Connection** - Real-time communication with Discord
   - File: `DiscordGateway.h` and `DiscordGateway.cpp`
   - Purpose: Maintain persistent WebSocket connection for bot presence updates
   - Uses SSL/TLS support for secure WSS connections

2. **Bot Presence Status** - Shows "Playing with X players" in Discord
   - Uses WebSocket to send presence updates via Gateway API
   - Alternative: REST API polling (when Gateway is disabled)

## Build Configuration Summary

```
FactoryGame.uproject
└── Plugins (Project-level)
    ├── WebSockets ✓ Enabled (for other uses)
    ├── WebSocketNetworking ✓ Enabled
    └── ... other plugins

Plugins/
└── WebSocketNetworking/
    └── Custom URL and SSL support added

Mods/
└── DiscordChatBridge/
    ├── DiscordChatBridge.uplugin
    │   └── Plugins (Mod-level)
    │       └── WebSocketNetworking ✓ Declared
    └── Source/DiscordChatBridge/
        └── DiscordChatBridge.Build.cs
            └── PublicDependencyModuleNames
                └── "WebSocketNetworking" ✓ Included
                └── "Sockets" ✓ Included

Source/
└── FactoryGame/
    └── FactoryGame.Build.cs
        └── PublicDependencyModuleNames
            └── WebSocketNetworking ✗ Not needed (correctly excluded)
```

## Key Changes Made

1. **WebSocketNetworking Plugin Enhanced**
   - Added URL-based connection support (was IP-only)
   - Enabled SSL/TLS for WSS connections
   - Added `CreateConnection(URL, Protocol)` method

2. **DiscordGateway Updated**
   - Changed from `IWebSocket` to `INetworkingWebSocket`
   - Updated callback system (from delegates to callback setters)
   - Added manual ticking for WebSocket updates

3. **Ticking Chain Added**
   - `ADiscordChatSubsystem::Tick()` → `UDiscordAPI::Tick()` → `UDiscordGateway::Tick()` → `WebSocket->Tick()`
   - Required because WebSocketNetworking needs manual ticking

## Conclusion

The WebSocketNetworking dependency is **correctly configured** for use by the DiscordChatBridge mod. The plugin has been enhanced to support URL-based WSS connections needed for Discord's Gateway API.

**The current configuration is correct and fully functional.**
