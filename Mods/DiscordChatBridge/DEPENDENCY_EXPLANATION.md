# WebSockets Dependency Explanation

## Question: Why use the built-in WebSockets module?

**Answer:** The mod uses the **built-in WebSockets module** from Unreal Engine instead of the custom WebSocketNetworking plugin.

## Why Built-in WebSockets?

### 1. Stability and Maintenance

The built-in WebSockets module is:
- Maintained and tested by Epic Games
- Well-integrated with Unreal Engine
- Widely used in production projects
- Automatically updated with engine updates

### 2. Simpler API

The built-in WebSockets module provides:
- Delegate-based event system (simpler than callbacks)
- Automatic connection management
- Built-in ticking (no manual tick required)
- String-based message handling (easier for JSON)

### 3. WebSockets Configuration

The WebSockets dependency is properly configured in three places:

#### a) Project Level - `FactoryGame.uproject`
```json
{
    "Name": "WebSockets",
    "Enabled": true
}
```
This enables the WebSockets plugin for the entire Unreal Engine project.

#### b) Mod Plugin Level - `DiscordChatBridge.uplugin`
```json
{
    "Name": "WebSockets",
    "Enabled": true
}
```
This declares that the DiscordChatBridge mod requires the WebSockets plugin.

#### c) Mod Module Level - `DiscordChatBridge.Build.cs`
```csharp
PublicDependencyModuleNames.AddRange(new string[] {
    // ... other dependencies ...
    "WebSockets",
    // ... more dependencies ...
});
```
This tells Unreal Build Tool that the DiscordChatBridge module needs to link against the WebSockets module.

### 4. FactoryGame Doesn't Use WebSockets

The FactoryGame module itself does not use any WebSockets functionality:
- No `#include "IWebSocket.h"` or similar includes
- No `FWebSocketsModule::Get()` calls
- No WebSocket-related code

Only the **DiscordChatBridge mod** uses WebSockets for the Discord Gateway connection.

### 5. Proper Dependency Isolation

In Unreal Engine modding:
- The **base game module** (FactoryGame) should only include dependencies it directly uses
- **Mods** (like DiscordChatBridge) declare their own dependencies independently
- This keeps dependencies clean and prevents unnecessary coupling

## How WebSockets is Used

The DiscordChatBridge mod uses WebSockets for:

1. **Discord Gateway Connection** - Real-time communication with Discord
   - File: `DiscordGateway.h` and `DiscordGateway.cpp`
   - Purpose: Maintain persistent WebSocket connection for bot presence updates
   - Uses built-in SSL/TLS support for secure WSS connections

2. **Bot Presence Status** - Shows "Playing with X players" in Discord
   - Uses WebSocket to send presence updates via Gateway API
   - Alternative: REST API polling (when Gateway is disabled)

## Build Configuration Summary

```
FactoryGame.uproject
└── Plugins (Project-level)
    ├── WebSockets ✓ Enabled
    ├── WebSocketNetworking ✓ Available (not used by DiscordChatBridge)
    └── ... other plugins

Mods/
└── DiscordChatBridge/
    ├── DiscordChatBridge.uplugin
    │   └── Plugins (Mod-level)
    │       └── WebSockets ✓ Declared
    └── Source/DiscordChatBridge/
        └── DiscordChatBridge.Build.cs
            └── PublicDependencyModuleNames
                └── "WebSockets" ✓ Included

Source/
└── FactoryGame/
    └── FactoryGame.Build.cs
        └── PublicDependencyModuleNames
            └── WebSockets ✗ Not needed (correctly excluded)
```

## Advantages of Built-in WebSockets

1. **Simpler API**
   - Delegate-based callbacks (OnConnected, OnMessage, OnClosed, OnConnectionError)
   - Automatic message handling (no manual tick required)
   - String-based messaging (perfect for JSON)

2. **Better Integration**
   - Works seamlessly with Unreal Engine's event system
   - Automatic connection lifecycle management
   - Built-in error handling and reconnection support

3. **Production Ready**
   - Used in many shipped Unreal Engine titles
   - Thoroughly tested by Epic Games
   - Regular updates and bug fixes

## Conclusion

The WebSockets dependency is **correctly configured** for use by the DiscordChatBridge mod. The built-in module provides stable, well-tested WebSocket functionality for Discord's Gateway API.

**The current configuration uses the industry-standard, production-ready WebSockets module from Unreal Engine.**
