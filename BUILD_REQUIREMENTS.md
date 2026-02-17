# Build Requirements

This document explains the requirements for building the Satisfactory Mod Loader and Discord Chat Bridge mod.

## Unreal Engine Requirements

### Required Unreal Engine Version

This project requires **Unreal Engine 5.3.2-CSS** (Coffee Stain Studios custom build).

The custom UE build is automatically downloaded during CI builds from the `satisfactorymodding/UnrealEngine` repository.

### Required Engine Plugins

The following Unreal Engine plugins must be present in your engine installation:

#### WebSockets Plugin

**Status**: ✅ Required (Project Plugin - Included in Repository)

The WebSockets plugin is a standard Unreal Engine plugin that provides WebSocket client functionality. It is used by the Discord Chat Bridge mod for Discord Gateway connections.

**Location and Build Process:**

The WebSockets plugin is **included in this project's repository** at `Plugins/WebSockets/` and builds as part of the project build process:

1. **Project Build Phase**: When building this project, Unreal Build Tool (UBT) compiles the WebSockets module from source
2. **Location**: `Plugins/WebSockets/` (within this repository)
3. **Module Linking**: DiscordChatBridge and other mods link against the WebSockets module during compilation

This approach ensures the WebSockets plugin is always available when building the project, eliminating dependency issues with engine installations.

**Configuration in this project:**

- ✅ **Plugins/WebSockets/WebSockets.uplugin**: Plugin descriptor enabling the module
- ✅ **Plugins/WebSockets/WebSockets.Build.cs**: Build rules for the WebSockets module
- ✅ **FactoryGame.uproject**: Enables WebSockets at project level (marked as optional)
- ✅ **DiscordChatBridge.uplugin**: Declares WebSockets dependency for the mod (marked as optional)
- ✅ **DiscordChatBridge.Build.cs**: Conditionally links the module against WebSockets if available

The WebSockets plugin is included directly in this repository, so no additional setup is required for local development or CI builds. The plugin will automatically compile when building the project.

## Other Required Plugins

In addition to WebSockets, this project requires several other plugins that are included in the CSS custom build or are part of this repository:

### Engine Plugins (Must be in UE installation)
- **ChaosVehiclesPlugin**: Vehicle physics
- **ReplicationGraph**: Network replication optimization
- **DTLSHandlerComponent**: Secure networking
- **ApexDestruction**: Destruction physics
- **SignificanceManager**: Actor significance management
- **EnhancedInput**: Modern input system
- **ControlRig**: Animation system
- **Various others**: See FactoryGame.uproject for complete list

### Project Plugins (Included in repository)
- **AbstractInstance**: Custom game plugin
- **FactoryGameUbtPlugin**: Build tool extensions
- **GameplayEvents**: Gameplay event system
- **InstancedSplines**: Spline rendering
- **Online/OnlineIntegration**: Online subsystem
- **ReliableMessaging**: Network messaging
- **SignificanceISPC**: ISPC-accelerated significance
- **WebSockets**: WebSocket client functionality for network communication

### Third-Party Plugins
- **Wwise**: Audio middleware (downloaded separately during build via B2 bucket)

## Installed Engine Build Compatibility

### BuildSettings Module

This project includes a custom **BuildSettings** module at `Source/BuildSettings/` to fix a precompiled manifest error that occurs when building with installed engine builds (engines distributed with `-installed` flag).

**Issue:** When using an installed engine build, the engine's BuildSettings module may not include precompiled manifests for all platforms and configurations. This causes build failures with:
```
Missing precompiled manifest for 'BuildSettings', 'Engine\Intermediate\Build\Linux\FactoryServer\Shipping\BuildSettings\BuildSettings.precompiled'
```

**Solution:** The project includes its own BuildSettings module with `PrecompileForTargets = PrecompileTargetsType.Any` in its Build.cs file. This tells Unreal Build Tool to compile the module from source instead of expecting precompiled binaries. The module is also explicitly declared in the FactoryGame.uproject file to ensure UBT prioritizes the project module over the engine module.

**Files:**
- `FactoryGame.uproject` - Explicitly declares BuildSettings module to override engine module
- `Source/BuildSettings/BuildSettings.Build.cs` - Build configuration with PrecompileForTargets setting
- `Source/BuildSettings/Public/BuildSettings.h` - Module interface
- `Source/BuildSettings/Private/BuildSettings.cpp` - Minimal module implementation

This module is automatically included in all build targets via `Source/FactoryShared.Target.cs`.

## Build Process Overview

The build happens in several phases:

### 1. **Engine Setup Phase**
   - Download and extract UE 5.3.2-CSS
   - Register engine installation

### 2. **Project Setup Phase**
   - Download Wwise plugin
   - Apply Wwise patches
   - UBT scans for available plugins including WebSockets (from project's Plugins folder)

### 3. **Module Build Phase**
   - Build FactoryGame modules
   - Build FactoryEditor modules
   - Build BuildSettings module (for installed engine compatibility)
   - Build project plugins (including WebSockets)
   - Build mod modules (including DiscordChatBridge which links against WebSockets)

### 4. **Packaging Phase**
   - Package mods for distribution
   - Create platform-specific builds (Win64, Linux)

## WebSockets Usage in Discord Chat Bridge

The WebSockets plugin is used exclusively by the **DiscordChatBridge mod** for:

### Discord Gateway Connection
- Establishes WebSocket connection to `wss://gateway.discord.gg`
- Maintains persistent connection for real-time bot presence updates
- Sends heartbeats to keep connection alive
- Handles reconnection logic
- Uses SSL/TLS for secure communication

### Files Using WebSockets
- `Mods/DiscordChatBridge/Source/DiscordChatBridge/Public/DiscordGateway.h`
- `Mods/DiscordChatBridge/Source/DiscordChatBridge/Private/DiscordGateway.cpp`

### Why Gateway/WebSockets?
The Discord Gateway allows the bot to have true presence status (shows "Playing with X players") instead of just posting messages to a channel. This is optional - the mod can work without Gateway mode using REST API only.

## Summary

**How does the WebSockets plugin build?**

The WebSockets plugin is **included in this repository** at `Plugins/WebSockets/` and builds automatically as part of the project compilation process. When you build this project:

1. ✅ UE 5.3.2-CSS is installed (engine)
2. ✅ WebSockets plugin source is present in `Plugins/WebSockets/` (part of this repository)
3. ✅ UBT compiles the WebSockets module along with other project plugins
4. ✅ DiscordChatBridge mod declares WebSockets as optional dependency
5. ✅ DiscordChatBridge.Build.cs detects WebSockets availability and links against it
6. ✅ Full Gateway/presence features are enabled in the DiscordChatBridge mod

**The WebSockets plugin is built once during the project build, then used by any mod that needs it (like DiscordChatBridge). No additional setup or engine plugin installation is required.**

## Additional Resources

- [Satisfactory Modding Documentation](https://docs.ficsit.app/)
- [Discord Chat Bridge Setup Guide](Mods/DiscordChatBridge/SETUP_GUIDE.md)
- [Discord Chat Bridge Dependency Explanation](Mods/DiscordChatBridge/DEPENDENCY_EXPLANATION.md)
- [Unreal Engine WebSockets Documentation](https://docs.unrealengine.com/5.3/en-US/API/Runtime/WebSockets/)
