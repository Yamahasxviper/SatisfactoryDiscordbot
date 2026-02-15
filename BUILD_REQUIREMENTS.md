# Build Requirements

This document explains the requirements for building the Satisfactory Mod Loader and Discord Chat Bridge mod.

## Unreal Engine Requirements

### Required Unreal Engine Version

This project requires **Unreal Engine 5.3.2-CSS** (Coffee Stain Studios custom build).

The custom UE build is automatically downloaded during CI builds from the `satisfactorymodding/UnrealEngine` repository.

### Required Engine Plugins

The following Unreal Engine plugins must be present in your engine installation:

#### WebSockets Plugin

**Status**: ✅ Required (Built-in Engine Plugin)

The WebSockets plugin is a standard Unreal Engine plugin that provides WebSocket client functionality. It is used by the Discord Chat Bridge mod for Discord Gateway connections.

**When does WebSockets build?**

The WebSockets plugin builds as part of the Unreal Engine itself, **not** as part of this project. Specifically:

1. **Engine Build Phase**: WebSockets is compiled when Unreal Engine is built/installed
2. **Location**: The plugin resides in `Engine/Plugins/Runtime/WebSockets/` or `Engine/Plugins/Experimental/WebSockets/` depending on the UE version
3. **Project Build Phase**: When building this project, Unreal Build Tool (UBT) links against the already-built WebSockets module

**Configuration in this project:**

- ✅ **FactoryGame.uproject**: Enables WebSockets at project level (marked as optional)
- ✅ **DiscordChatBridge.uplugin**: Declares WebSockets dependency for the mod
- ✅ **DiscordChatBridge.Build.cs**: Conditionally links the module against WebSockets only if available
- ❌ **FactoryGame.Build.cs**: Does NOT include WebSockets (correctly - only mods use it)

**Important Update:** As of the latest version, `DiscordChatBridge.Build.cs` now **automatically detects** whether the WebSockets plugin is available in your engine installation and only includes it as a dependency if found. This means:
- The mod will **build successfully** even if WebSockets is not available in the engine
- If WebSockets is not found during build, the mod compiles with `WITH_WEBSOCKETS_SUPPORT=0`
- Gateway/presence features will be disabled at runtime if WebSockets was not available at compile time
- If WebSockets is found during build, full Gateway functionality is enabled

**If your custom Unreal Engine build doesn't have WebSockets:**

📖 **See [WEBSOCKET_ALTERNATIVES.md](WEBSOCKET_ALTERNATIVES.md) for a comprehensive guide on:**
- How to add WebSockets to custom Unreal Engine builds
- Alternative options when WebSockets is not available
- Using the mod without WebSockets (REST-only mode)

**Quick Summary:**

The DiscordChatBridge mod **works without WebSockets** - it will automatically use REST API instead of Gateway. The only missing feature without WebSockets is real-time bot presence ("Playing with X players" status).

**For CI/Automated Builds:**
- The official CI workflow automatically downloads the correct UE build which includes WebSockets
- No action needed if using the standard CI workflow

**For Local Development - Quick Options:**

1. **Use without WebSockets (Recommended for most users)**
   - No additional setup required
   - All core features work (chat, notifications, commands)
   - Only missing: real-time presence updates
   - See [WEBSOCKET_ALTERNATIVES.md](WEBSOCKET_ALTERNATIVES.md#option-4-use-without-websockets-recommended-for-most-users)

2. **Copy WebSockets from standard UE (If you need presence features)**
   - Copy `Engine/Plugins/Runtime/WebSockets/` from standard UE 5.3.2 to your custom build
   - Fastest way to enable WebSockets
   - See [WEBSOCKET_ALTERNATIVES.md](WEBSOCKET_ALTERNATIVES.md#option-1-copy-websocket-plugin-from-standard-unreal-engine-easiest)

3. **Build from source**
   - Build WebSockets plugin from UE source code
   - See [WEBSOCKET_ALTERNATIVES.md](WEBSOCKET_ALTERNATIVES.md#option-2-build-websocket-plugin-from-unreal-engine-source)

**Verify WebSockets Installation:**
```bash
# Check if plugin exists
ls "<UE_ROOT>/Engine/Plugins/Runtime/WebSockets/"
# or
ls "<UE_ROOT>/Engine/Plugins/Experimental/WebSockets/"
```

**Still having issues?** Check [TROUBLESHOOTING.md](TROUBLESHOOTING.md) or see the detailed guide in [WEBSOCKET_ALTERNATIVES.md](WEBSOCKET_ALTERNATIVES.md).

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

### Third-Party Plugins
- **Wwise**: Audio middleware (downloaded separately during build via B2 bucket)

## Build Process Overview

The build happens in several phases:

### 1. **Engine Setup Phase**
   - Download and extract UE 5.3.2-CSS
   - Register engine installation
   - **WebSockets is already built at this point** (part of engine)

### 2. **Project Setup Phase**
   - Download Wwise plugin
   - Apply Wwise patches
   - UBT scans for available plugins including WebSockets

### 3. **Module Build Phase**
   - Build FactoryGame modules
   - Build FactoryEditor modules
   - Build project plugins
   - Build mod modules (including DiscordChatBridge)
   - **DiscordChatBridge links against pre-built WebSockets module**

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

**"At what point will the WebSockets build?"**

The WebSockets plugin builds **during Unreal Engine compilation**, not during this project's build. When you build this project:

1. ✅ UE 5.3.2-CSS should be built/installed (ideally includes WebSockets)
2. ✅ This project references WebSockets in `.uproject` file (marked optional)
3. ✅ DiscordChatBridge mod declares WebSockets dependency (marked optional)
4. ✅ **NEW:** DiscordChatBridge.Build.cs automatically detects WebSockets availability
5. ✅ If found: UBT links DiscordChatBridge against the existing WebSockets module
6. ✅ If not found: Mod builds without WebSockets support (Gateway features disabled)

**The WebSockets plugin is built once with the engine, then reused by any project that needs it. If it's not available, the DiscordChatBridge mod will still build successfully but without Gateway/presence features.**

## Additional Resources

- [Satisfactory Modding Documentation](https://docs.ficsit.app/)
- [Discord Chat Bridge Setup Guide](Mods/DiscordChatBridge/SETUP_GUIDE.md)
- [Discord Chat Bridge Dependency Explanation](Mods/DiscordChatBridge/DEPENDENCY_EXPLANATION.md)
- [Unreal Engine WebSockets Documentation](https://docs.unrealengine.com/5.3/en-US/API/Runtime/WebSockets/)
