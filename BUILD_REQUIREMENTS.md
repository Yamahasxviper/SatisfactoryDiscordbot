# Build Requirements

This document explains the requirements for building the Satisfactory Mod Loader and Discord Chat Bridge mod.

## Unreal Engine Requirements

### Required Unreal Engine Version

This project requires **Unreal Engine 5.3.2-CSS** (Coffee Stain Studios custom build).

The custom UE build is automatically downloaded during CI builds from the `satisfactorymodding/UnrealEngine` repository.

### Required Engine Plugins

The following Unreal Engine plugins must be present in your engine installation:

#### WebSocketNetworking Plugin

**Status**: ✅ Required (Project Plugin - Included in repository)

The WebSocketNetworking plugin provides WebSocket client functionality using libwebsockets. It is used by the Discord Chat Bridge mod for Discord Gateway connections. This plugin is included in the project's `Plugins/` directory and has been enhanced to support:
- URL-based WebSocket connections (ws:// and wss://)
- SSL/TLS for secure connections
- Custom protocol support

**When does WebSocketNetworking build?**

The WebSocketNetworking plugin builds as part of **this project**, not as part of the Unreal Engine itself. Specifically:

1. **Project Build Phase**: WebSocketNetworking compiles when building this project
2. **Location**: The plugin resides in `Plugins/WebSocketNetworking/`
3. **Dependencies**: Requires OpenSSL, libWebSockets, and zlib (included in engine)

**Configuration in this project:**

- ✅ **Plugins/WebSocketNetworking/**: Source code for the plugin
- ✅ **FactoryGame.uproject** (Added): Enables WebSocketNetworking at project level
- ✅ **DiscordChatBridge.uplugin**: Declares WebSocketNetworking dependency for the mod
- ✅ **DiscordChatBridge.Build.cs**: Links the module against WebSocketNetworking and Sockets
- ❌ **FactoryGame.Build.cs**: Does NOT include WebSocketNetworking (correctly - only mods use it)

**Troubleshooting "Unable to find plugin 'WebSocketNetworking'" error:**

If you encounter this error, it means the WebSocketNetworking plugin is not being found in the project:

**For CI/Automated Builds:**
- The plugin is included in the repository under `Plugins/WebSocketNetworking/`
- Ensure the repository was cloned completely with all subdirectories
- No additional setup needed if using the standard CI workflow

**For Local Development:**

1. **Check if WebSocketNetworking exists in the project:**
   ```bash
   ls "Plugins/WebSocketNetworking/"
   ```

2. **If WebSocketNetworking is missing:**
   - The plugin should be in the repository
   - Ensure you have cloned the repository completely
   - Check that git didn't skip the Plugins directory

3. **If WebSocketNetworking is present but still getting errors:**
   - Clean and regenerate project files: Delete `.sln` files and `Intermediate/` folder
   - Regenerate project files using `GenerateProjectFiles.bat` (Windows) or `GenerateProjectFiles.sh` (Linux)
   - Ensure OpenSSL and libWebSockets are available in your engine installation

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
- **WebSocketNetworking**: WebSocket client with libwebsockets (enhanced for URL/SSL support)

### Third-Party Plugins
- **Wwise**: Audio middleware (downloaded separately during build via B2 bucket)

## Build Process Overview

The build happens in several phases:

### 1. **Engine Setup Phase**
   - Download and extract UE 5.3.2-CSS
   - Register engine installation
   - Engine includes required dependencies (OpenSSL, libWebSockets)

### 2. **Project Setup Phase**
   - Download Wwise plugin
   - Apply Wwise patches
   - UBT scans for available plugins including WebSocketNetworking

### 3. **Module Build Phase**
   - Build FactoryGame modules
   - Build FactoryEditor modules
   - Build project plugins (including WebSocketNetworking)
   - Build mod modules (including DiscordChatBridge)
   - **DiscordChatBridge links against WebSocketNetworking module**

### 4. **Packaging Phase**
   - Package mods for distribution
   - Create platform-specific builds (Win64, Linux)

## WebSocketNetworking Usage in Discord Chat Bridge

The WebSocketNetworking plugin is used exclusively by the **DiscordChatBridge mod** for:

### Discord Gateway Connection
- Establishes WebSocket connection to `wss://gateway.discord.gg`
- Maintains persistent connection for real-time bot presence updates
- Sends heartbeats to keep connection alive
- Handles reconnection logic
- Uses SSL/TLS for secure communication

### Files Using WebSocketNetworking
- `Mods/DiscordChatBridge/Source/DiscordChatBridge/Public/DiscordGateway.h`
- `Mods/DiscordChatBridge/Source/DiscordChatBridge/Private/DiscordGateway.cpp`
- `Plugins/WebSocketNetworking/Source/WebSocketNetworking/` (Enhanced with URL/SSL support)

### Why Gateway/WebSocketNetworking?
The Discord Gateway allows the bot to have true presence status (shows "Playing with X players") instead of just posting messages to a channel. This is optional - the mod can work without Gateway mode using REST API only.

### Enhancements Made
The WebSocketNetworking plugin was enhanced to support:
1. **URL-based connections**: Accepts full URLs (ws:// or wss://) instead of just IP addresses
2. **SSL/TLS support**: Enabled secure WebSocket connections required by Discord
3. **Protocol specification**: Allows specifying WebSocket sub-protocols
4. **Manual ticking**: Integrated into game loop for proper message handling

## Summary

**"At what point will the WebSocketNetworking build?"**

The WebSocketNetworking plugin builds **during this project's compilation**, as a project plugin. When you build this project:

1. ✅ UE 5.3.2-CSS must be installed (provides OpenSSL, libWebSockets dependencies)
2. ✅ WebSocketNetworking plugin is in the `Plugins/` directory
3. ✅ This project references WebSocketNetworking in `.uproject` file
4. ✅ DiscordChatBridge mod declares WebSocketNetworking dependency
5. ✅ UBT builds WebSocketNetworking plugin first
6. ✅ UBT links DiscordChatBridge against WebSocketNetworking module
7. ✅ Your code can now use enhanced WebSocket functionality with URL/SSL support

**The WebSocketNetworking plugin builds as part of the project, providing enhanced WebSocket capabilities for the Discord integration.**

## Additional Resources

- [Satisfactory Modding Documentation](https://docs.ficsit.app/)
- [Discord Chat Bridge Setup Guide](Mods/DiscordChatBridge/SETUP_GUIDE.md)
- [Discord Chat Bridge Dependency Explanation](Mods/DiscordChatBridge/DEPENDENCY_EXPLANATION.md)
- [Unreal Engine WebSockets Documentation](https://docs.unrealengine.com/5.3/en-US/API/Runtime/WebSockets/)
