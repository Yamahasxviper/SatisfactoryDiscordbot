# How to Use WebSockets with Discord Chat Bridge

## ✅ Good News: WebSockets Are Already Included!

**The WebSockets plugin is already present in this repository and ready to use.**

If you're seeing this guide because you think WebSockets are "missing" - they're not! This document will help you understand what you have and how to use it.

---

## Quick Status Check

Run this command to verify WebSockets are available in your setup:

```bash
./scripts/verify_websocket_build_compatibility.sh
```

Expected result: **✅ BUILD COMPATIBLE** (26/26 checks passed)

---

## What You Have

### 1. **WebSocket Plugin Location**
```
Plugins/WebSockets/
├── WebSockets.uplugin      ← Plugin descriptor
├── Source/
│   └── WebSockets/
│       ├── WebSockets.Build.cs   ← Build configuration
│       ├── Public/               ← Header files
│       └── Private/              ← Implementation
└── ThirdParty/
    └── libWebSockets/            ← Third-party library
```

✅ **Status**: Included in this repository

### 2. **Project Configuration**
- ✅ Enabled in `FactoryGame.uproject` (line 81-84)
- ✅ Declared in `DiscordChatBridge.uplugin` (line 46-49)
- ✅ Auto-detected by `DiscordChatBridge.Build.cs`

### 3. **Third-Party Dependencies**
```
ThirdParty/libWebSockets/libwebsockets/
├── include/     ← Header files for all platforms
└── lib/         ← Pre-compiled libraries (Windows, Linux, Mac)
```

✅ **Status**: Included in this repository

---

## How to Build With WebSockets

### Automatic Build (Recommended)

The WebSocket plugin builds automatically when you build the project:

```bash
# The CI workflow does this automatically
# See .github/workflows/build.yml

# For local development, build normally:
# 1. Open FactoryGame.uproject in Unreal Editor
# 2. Build → Build FactoryEditor
# 3. WebSockets builds as a project plugin
```

The build system will:
1. ✅ Detect WebSocket plugin at `Plugins/WebSockets/`
2. ✅ Compile WebSocket module for your target platform
3. ✅ Link DiscordChatBridge against WebSockets
4. ✅ Set `WITH_WEBSOCKETS_SUPPORT=1`
5. ✅ Enable full Gateway/presence features

### Verify the Build

After building, check the build output:

```
Expected output:
✅ Building WebSockets (Win64, Development)...
✅ Building DiscordChatBridge (Win64, Development)...
✅ WITH_WEBSOCKETS_SUPPORT=1
```

---

## What WebSockets Enable

### With WebSockets (Default)
✅ **Discord Gateway Connection**: Real-time bot presence  
✅ **Activity Status**: Shows "Playing with X players"  
✅ **Instant Updates**: Real-time status changes  
✅ **Reconnection Logic**: Automatic reconnect on disconnect  
✅ **Heartbeat System**: Keeps connection alive  

### Without WebSockets (Fallback)
⚠️ **REST API Only**: Basic chat functionality  
⚠️ **No Presence**: Bot won't show online status  
⚠️ **No Activity**: No "Playing with X players" status  
⚠️ **Polling Only**: Less efficient, no real-time updates  

---

## Configuration

To use WebSocket features (Gateway), ensure these settings in your config:

### Config File Location
**Primary** (Recommended):
```
Mods/DiscordChatBridge/config/DiscordChatBridge.txt
```

**Runtime Locations**:
- Windows: `%localappdata%\FactoryGame\Saved\Config\WindowsServer\DiscordChatBridge.ini`
- Linux: `~/.config/Epic/FactoryGame/Saved/Config/LinuxServer/DiscordChatBridge.ini`

### Required Settings for Gateway

```ini
# Basic configuration (required)
[/Script/DiscordChatBridge.DiscordConfig]
BotToken=YOUR_BOT_TOKEN_HERE
ChannelId=YOUR_CHANNEL_ID_HERE

# Enable Gateway features (optional, default: true)
UseGatewayForPresence=true
EnableBotActivity=true
```

### Discord Bot Setup

Your Discord bot needs these **Gateway Intents** enabled:

1. Go to https://discord.com/developers/applications
2. Select your application
3. Go to **Bot** section
4. Scroll down to **Privileged Gateway Intents**
5. Enable:
   - ✅ **MESSAGE CONTENT INTENT** (required for chat)
   - ✅ **PRESENCE INTENT** (required for Gateway/activity)
   - ✅ **SERVER MEMBERS INTENT** (optional, for member info)

---

## Troubleshooting

### "I think WebSockets are missing"

**Check these locations:**

1. **Is the plugin present?**
   ```bash
   ls -la Plugins/WebSockets/
   # Should show: WebSockets.uplugin, Source/, etc.
   ```

2. **Is it enabled in the project?**
   ```bash
   grep -A 2 "WebSockets" FactoryGame.uproject
   # Should show: "Enabled": true
   ```

3. **Run the verification script:**
   ```bash
   ./scripts/verify_websocket_build_compatibility.sh
   # Should show: ✅ BUILD COMPATIBLE
   ```

If all these pass, **WebSockets ARE available** - you don't need to do anything!

### "Discord Gateway not connecting"

**Checklist:**
- ✅ BotToken configured correctly
- ✅ UseGatewayForPresence=true in config
- ✅ PRESENCE INTENT enabled in Discord Developer Portal
- ✅ Server has internet connection
- ✅ No firewall blocking wss://gateway.discord.gg

**Check server logs for:**
```
✅ Good: "Gateway connected successfully"
❌ Bad: "Failed to connect to gateway"
❌ Bad: "WebSocket connection error"
```

### "Build says WebSockets not found"

**This is extremely rare** with the current setup. If you see this:

1. **Verify plugin exists:**
   ```bash
   ls -la Plugins/WebSockets/WebSockets.uplugin
   ```

2. **Check Build.cs detection:**
   Build output should show:
   ```
   [DiscordChatBridge] Build.cs: Checking for WebSockets plugin...
   [DiscordChatBridge]   Path 3 (Project/Plugins): <path> - FOUND
   [DiscordChatBridge] Build.cs: WebSockets plugin FOUND
   ```

3. **Clean and rebuild:**
   ```bash
   # Delete generated files
   rm -rf Intermediate/ Saved/ *.sln *.vcxproj*
   
   # Regenerate and rebuild
   # Right-click FactoryGame.uproject → Generate Visual Studio project files
   # Then build normally
   ```

### "WebSocket features not working at runtime"

**Possible causes:**

1. **Configuration missing**: See the [Quick Start Guide](Mods/DiscordChatBridge/help/QUICKSTART.md)

2. **Discord intents not enabled**: Check Discord Developer Portal

3. **Firewall blocking connection**: Allow outbound connections to `wss://gateway.discord.gg`

4. **Check logs**: Look for Gateway-related messages in server logs

---

## Platform Support

The WebSocket plugin supports all major platforms:

| Platform | Implementation | Status |
|----------|----------------|--------|
| Windows (Win64) | WinHttp | ✅ Fully supported (Windows 8.1+) |
| Linux (x86_64) | libwebsockets | ✅ Fully supported |
| Linux (ARM64) | libwebsockets | ✅ Fully supported |
| macOS | libwebsockets | ✅ Fully supported |
| iOS | libwebsockets | ✅ Supported |
| Android | libwebsockets | ✅ Supported |

**Note**: Satisfactory dedicated servers primarily use **Windows** and **Linux** platforms.

---

## Understanding "Custom Unreal Engine"

### SML Custom Build (5.3.2-CSS)

The Coffee Stain Studios custom Unreal Engine build is used for Satisfactory modding. This build:

- ✅ Is based on standard Unreal Engine 5.3.2
- ✅ Includes all standard engine plugins
- ✅ Has additional Coffee Stain modifications
- ✅ **Does NOT remove WebSockets** - it's still there!

### "My engine doesn't have WebSockets"

**This is not about the engine installation.** The WebSocket plugin in this repository is a **project plugin**, not an engine plugin.

**What this means:**
- ❌ You don't need WebSockets in your engine installation
- ✅ WebSockets builds from the `Plugins/WebSockets/` directory in this repository
- ✅ It's part of the project, not the engine
- ✅ It works with any UE 5.3.2 installation (including CSS custom build)

---

## Additional Resources

### Documentation
- [Build Requirements](BUILD_REQUIREMENTS.md) - Complete build setup
- [Troubleshooting Guide](TROUBLESHOOTING.md) - Common issues
- [Quick Start Guide](Mods/DiscordChatBridge/help/QUICKSTART.md) - Getting started
- [Setup Guide](Mods/DiscordChatBridge/help/SETUP_GUIDE.md) - Detailed configuration

### Verification Scripts
- `scripts/verify_websocket_build_compatibility.sh` - Build compatibility
- `scripts/validate_websocket_integration.sh` - Runtime validation
- `scripts/test_websocket_linux_compatibility.sh` - Linux testing

### Discord Chat Bridge Docs
- [README](Mods/DiscordChatBridge/README.md)
- [Configuration Examples](Mods/DiscordChatBridge/help/EXAMPLES.md)
- [Technical Architecture](Mods/DiscordChatBridge/help/TECHNICAL_ARCHITECTURE.md)
- [Dependency Explanation](Mods/DiscordChatBridge/help/DEPENDENCY_EXPLANATION.md)

### Community
- [Satisfactory Modding Docs](https://docs.ficsit.app/)
- [Discord Community](https://discord.gg/QzcG9nX)

---

## Summary

### The Bottom Line

**✅ WebSockets are already included and working in this repository.**

You don't need to:
- ❌ Install any engine plugins
- ❌ Download additional dependencies
- ❌ Modify any build files
- ❌ Copy plugins from other locations

You just need to:
- ✅ Build the project normally
- ✅ Configure your Discord bot token
- ✅ Enable Gateway intents in Discord Developer Portal
- ✅ Run your server

That's it! The WebSocket plugin will build automatically and enable full Discord Gateway features.

---

## Still Having Issues?

If you've read this guide and still believe WebSockets are missing or not working:

1. **Run the verification script** and share the output
2. **Check your build logs** for WebSocket-related messages
3. **Review server logs** for Gateway connection messages
4. **Join the Discord community** for help: https://discord.gg/QzcG9nX
5. **Open a GitHub issue** with:
   - Verification script output
   - Build log excerpt
   - Server log excerpt
   - Your Unreal Engine version

We're here to help! 🚀
