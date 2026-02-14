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
- ✅ **DiscordChatBridge.Build.cs**: Links the module against WebSockets
- ❌ **FactoryGame.Build.cs**: Does NOT include WebSockets (correctly - only mods use it)

**Troubleshooting "Unable to find plugin 'WebSockets'" error:**

**Note:** As of the latest update, WebSockets is marked as **optional** in `FactoryGame.uproject`. This means:
- The project can build successfully even without WebSockets
- Only the DiscordChatBridge mod will fail to load if WebSockets is missing
- The base game and other mods will work normally

If you encounter this error and want to use the DiscordChatBridge mod, it means the WebSockets plugin is not present in your Unreal Engine installation:

**For CI/Automated Builds:**
- The official CI workflow automatically downloads the correct UE build which includes WebSockets
- No action needed if using the standard CI workflow

**For Local Development:**

1. **Check if WebSockets exists in your engine:**
   ```bash
   # Check engine plugins directory
   # Replace <UE_ROOT> with your engine installation path
   ls "<UE_ROOT>/Engine/Plugins/Runtime/WebSockets/"
   # or
   ls "<UE_ROOT>/Engine/Plugins/Experimental/WebSockets/"
   ```

2. **If WebSockets is missing from your engine:**
   
   Option A: **Use the official CSS Unreal Engine build** (Recommended)
   - The CSS custom build includes all required plugins
   - Download from `satisfactorymodding/UnrealEngine` repository (requires authentication)
   - Follow the SML documentation for setting up the development environment

   Option B: **Use a standard UE 5.3.2 build**
   - WebSockets should be included by default in UE 5.3.2
   - If missing, you may need to enable it via the Epic Games Launcher
   - Note: Some CSS-specific features may not work with standard UE

   Option C: **Build WebSockets from source**
   - If you have UE source code, WebSockets should build automatically
   - Located in `Engine/Plugins/Runtime/WebSockets/`
   - Builds as part of the engine compile process

3. **If WebSockets is present but still getting errors:**
   - Ensure the plugin is enabled in your UE installation
   - Check that UnrealBuildTool can find the plugin
   - Verify no path issues in your engine installation
   - Clean and regenerate project files: Delete `.sln` files and `Intermediate/` folder, then regenerate

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

1. ✅ UE 5.3.2-CSS must already be built/installed (includes WebSockets)
2. ✅ This project references WebSockets in `.uproject` file
3. ✅ DiscordChatBridge mod declares WebSockets dependency
4. ✅ UBT links DiscordChatBridge against the existing WebSockets module
5. ✅ Your code can now use WebSocket functionality

**The WebSockets plugin is built once with the engine, then reused by any project that needs it.**

## Additional Resources

- [Satisfactory Modding Documentation](https://docs.ficsit.app/)
- [Discord Chat Bridge Setup Guide](Mods/DiscordChatBridge/SETUP_GUIDE.md)
- [Discord Chat Bridge Dependency Explanation](Mods/DiscordChatBridge/DEPENDENCY_EXPLANATION.md)
- [Unreal Engine WebSockets Documentation](https://docs.unrealengine.com/5.3/en-US/API/Runtime/WebSockets/)
