# WebSockets Dependency Explanation

## Question: Does FactoryGame.Build.cs need the WebSockets dependency?

**Answer: NO** - The WebSockets dependency does NOT need to be added to `FactoryGame.Build.cs`.

## Why Not?

### 1. WebSockets is Already Properly Configured

The WebSockets dependency is correctly configured in three places:

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

### 2. FactoryGame Doesn't Use WebSockets

The FactoryGame module itself does not use any WebSockets functionality:
- No `#include "IWebSocket.h"` or similar includes
- No `FWebSocketsModule::Get()` calls
- No WebSocket-related code

Only the **DiscordChatBridge mod** uses WebSockets for the Discord Gateway connection.

### 3. Proper Dependency Isolation

In Unreal Engine modding:
- The **base game module** (FactoryGame) should only include dependencies it directly uses
- **Mods** (like DiscordChatBridge) declare their own dependencies independently
- This keeps dependencies clean and prevents unnecessary coupling

## How WebSockets is Used

The DiscordChatBridge mod uses WebSockets for:

1. **Discord Gateway Connection** - Real-time communication with Discord
   - File: `DiscordGateway.h` and `DiscordGateway.cpp`
   - Purpose: Maintain persistent WebSocket connection for bot presence updates

2. **Bot Presence Status** - Shows "Playing with X players" in Discord
   - Uses WebSocket to send presence updates
   - Alternative: REST API polling (when Gateway is disabled)

## Build Configuration Summary

```
FactoryGame.uproject
└── Plugins (Project-level)
    ├── WebSockets ✓ Enabled
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

## Conclusion

The WebSockets dependency is **already correctly configured** where it's needed. Adding it to `FactoryGame.Build.cs` would be:

1. ❌ **Unnecessary** - FactoryGame doesn't use WebSockets
2. ❌ **Against best practices** - Pollutes the base game module with unused dependencies
3. ❌ **Incorrect architecture** - Breaks dependency isolation between game and mods

**The current configuration is correct and requires no changes.**
