# WebSocket Support for Custom Unreal Engine Builds

## Problem Statement

If you're working with a **custom Unreal Engine build** (such as the Coffee Stain Studios custom build for Satisfactory) that doesn't include the WebSockets plugin, you have several options to add WebSocket support.

## Understanding the Issue

The Discord Chat Bridge mod uses WebSockets for:
- **Discord Gateway Connection**: Real-time communication with Discord
- **Bot Presence Updates**: Shows "Playing with X players" status
- **Real-time event handling**: Instant updates when Discord messages arrive

However, **the mod will still work without WebSockets**, it just won't have these real-time features and will fall back to REST API polling.

## Quick Decision Guide

**Do you NEED WebSocket features?**
- ✅ YES → Choose Option 1, 2, or 3 below
- ❌ NO → The mod works without WebSockets! See "Option 4: Use Without WebSockets"

## Option 1: Copy WebSocket Plugin from Standard Unreal Engine (Easiest)

If you have access to a standard Unreal Engine 5.3.2 installation, you can copy the WebSocket plugin to your custom build.

### Steps:

1. **Locate WebSockets in Standard UE:**
   ```bash
   # Standard UE location (Windows):
   C:\Program Files\Epic Games\UE_5.3\Engine\Plugins\Runtime\WebSockets\
   
   # Or (Linux):
   ~/UnrealEngine-5.3/Engine/Plugins/Runtime/WebSockets/
   ```

2. **Copy to Custom Build:**
   ```bash
   # Windows:
   xcopy "C:\Program Files\Epic Games\UE_5.3\Engine\Plugins\Runtime\WebSockets" ^
         "C:\Path\To\CustomUE\Engine\Plugins\Runtime\WebSockets" /E /I
   
   # Linux:
   cp -r ~/UnrealEngine-5.3/Engine/Plugins/Runtime/WebSockets \
         ~/CustomUE/Engine/Plugins/Runtime/WebSockets
   ```

3. **Verify Plugin Files:**
   Ensure these files exist in the copied directory:
   - `WebSockets.uplugin`
   - `Source/WebSockets/` (source code)
   - `Binaries/` (if pre-built)

4. **Rebuild Custom Engine** (if needed):
   ```bash
   # Only needed if binaries are not included
   cd /Path/To/CustomUE
   
   # Windows:
   Engine\Build\BatchFiles\Build.bat UnrealEditor Win64 Development
   
   # Linux:
   Engine/Build/BatchFiles/Linux/Build.sh UnrealEditor Linux Development
   ```

5. **Verify Installation:**
   ```bash
   # Check if plugin directory exists
   ls Engine/Plugins/Runtime/WebSockets/
   
   # Should show: WebSockets.uplugin, Source/, Binaries/, etc.
   ```

**Advantages:**
- ✅ Fastest method
- ✅ No compilation required (if binaries included)
- ✅ Exact same version as standard UE

**Disadvantages:**
- ⚠️ Requires access to standard UE installation
- ⚠️ May need to rebuild if binaries not compatible

## Option 2: Build WebSocket Plugin from Unreal Engine Source

If you have the Unreal Engine source code, you can build the WebSocket plugin yourself.

### Steps:

1. **Clone Unreal Engine Source** (if not already done):
   ```bash
   git clone https://github.com/EpicGames/UnrealEngine.git
   cd UnrealEngine
   git checkout 5.3.2-release
   ```

2. **Setup Engine:**
   ```bash
   # Windows:
   Setup.bat
   GenerateProjectFiles.bat
   
   # Linux:
   ./Setup.sh
   ./GenerateProjectFiles.sh
   ```

3. **Build WebSocket Plugin:**
   ```bash
   # The WebSocket plugin will be built as part of the engine
   # Windows:
   Engine\Build\BatchFiles\Build.bat UnrealEditor Win64 Development
   
   # Linux:
   Engine/Build/BatchFiles/Linux/Build.sh UnrealEditor Linux Development
   ```

4. **Copy Built Plugin to Custom Build:**
   ```bash
   # After building, copy from source build to custom build
   cp -r Engine/Plugins/Runtime/WebSockets \
         /Path/To/CustomUE/Engine/Plugins/Runtime/WebSockets
   ```

**Advantages:**
- ✅ Complete control over build
- ✅ Can customize if needed
- ✅ Latest source code

**Disadvantages:**
- ⚠️ Requires UE source access
- ⚠️ Time-consuming (engine build can take hours)
- ⚠️ Requires significant disk space

## Option 3: Use Prebuilt WebSocket Plugin Binary

Download a prebuilt WebSocket plugin for your platform.

### Steps:

1. **Download from Community:**
   - Check Unreal Engine forums
   - Check GitHub repositories for prebuilt UE 5.3 plugins
   - Ask in Satisfactory modding Discord

2. **Verify Compatibility:**
   - Must be for UE 5.3.x
   - Must match your platform (Win64/Linux)
   - Should be from a trusted source

3. **Install:**
   ```bash
   # Extract to engine plugins directory
   unzip WebSockets-UE5.3.zip -d CustomUE/Engine/Plugins/Runtime/
   ```

**Advantages:**
- ✅ No building required
- ✅ Quick setup

**Disadvantages:**
- ⚠️ Need to find trusted source
- ⚠️ May have version compatibility issues
- ⚠️ Security concern with untrusted binaries

## Option 4: Use Without WebSockets (Recommended for Most Users)

The Discord Chat Bridge mod is designed to work **without WebSockets**. The mod automatically detects if WebSockets is available and adjusts functionality accordingly.

### What Works Without WebSockets:

✅ **All Core Features Work:**
- ✅ Two-way chat synchronization (Discord <-> Game)
- ✅ Player join/leave notifications
- ✅ Server start/stop notifications
- ✅ All Discord bot commands
- ✅ Message formatting and filtering
- ✅ Configuration through INI files


### What Doesn't Work Without WebSockets:

❌ **Gateway Features Only:**
- ❌ Real-time bot presence (shows "Playing with X players")
- ❌ Instant Discord message reception (uses polling instead)
- ❌ Discord Gateway events (READY, RESUMED, etc.)

### How Mod Behaves Without WebSockets:

When compiled without WebSockets support (`WITH_WEBSOCKETS_SUPPORT=0`):

1. **Chat Still Works**: Uses Discord REST API for sending/receiving messages
2. **Polling Used**: Checks for new messages every 5-10 seconds
3. **No Presence**: Bot shows as "online" but doesn't show activity
4. **Clear Logs**: Server logs will show:
   ```
   DiscordGateway: WebSockets plugin not available - Gateway features disabled
   DiscordChatBridge: Running in REST-only mode (no Gateway)
   ```

### Configuration for REST-Only Mode:

In your `DiscordChatBridge.ini`:
```ini
[/Script/DiscordChatBridge.DiscordChatSubsystem]
; Disable Gateway/presence features
EnableBotActivity=false
UseGatewayForPresence=false

; Core features still work
BotToken=your_bot_token_here
ChannelId=your_channel_id_here
EnableServerNotifications=true
```

**Advantages:**
- ✅ No additional setup required
- ✅ All essential features work
- ✅ Simpler deployment
- ✅ No extra dependencies

**Disadvantages:**
- ⚠️ No real-time presence updates
- ⚠️ Slight delay in message reception (5-10 seconds)

## Verification: Is WebSockets Available?

### Check During Build:

```bash
# Windows:
dir "CustomUE\Engine\Plugins\Runtime\WebSockets"

# Linux:
ls CustomUE/Engine/Plugins/Runtime/WebSockets
```

### Check Build Logs:

When building the DiscordChatBridge mod, look for:

**WITH WebSockets:**
```
Building DiscordChatBridge (WITH_WEBSOCKETS_SUPPORT=1)
Including module: WebSockets
```

**WITHOUT WebSockets:**
```
Building DiscordChatBridge (WITH_WEBSOCKETS_SUPPORT=0)
WebSockets plugin not found - building without Gateway support
```

### Check at Runtime:

In server logs, look for:

**WITH WebSockets:**
```
DiscordGateway: Initialized
DiscordGateway: Connecting to Discord Gateway...
DiscordGateway: Connected successfully
```

**WITHOUT WebSockets:**
```
DiscordChatBridge: WebSockets not available - Gateway features disabled
DiscordChatBridge: Running in REST-only mode
```

## Recommendations by Use Case

### For Development/Testing:
**- Use Option 4** (without WebSockets)
- Faster setup
- All core features work
- Focus on mod development, not infrastructure

### For Production Server:
**- Use Option 1** (copy from standard UE) or **Option 4**
- Option 1 if you want presence features
- Option 4 if you don't need presence (simpler)

### For CI/Automated Builds:
**- Use Option 1** or **ensure UE build includes WebSockets**
- Script the plugin copy process
- Or use standard UE with WebSockets included

### For Learning/Understanding:
**- Use Option 2** (build from source)
- Learn how UE plugins work
- Understand WebSocket implementation



## Common Questions

### Q: Will the mod crash without WebSockets?
**A:** No! The mod detects WebSocket availability at compile time and runtime. It gracefully disables Gateway features and continues working with REST API.

### Q: Can I add WebSockets after building?
**A:** Not easily. WebSockets must be available during compilation. If you add it later, you need to rebuild the DiscordChatBridge mod.

### Q: Is REST-only mode slower?
**A:** Slightly. Messages may have 5-10 second delay instead of instant. For most use cases, this is acceptable.

### Q: Can I use a different WebSocket library?
**A:** Not recommended. The mod is designed for Unreal Engine's built-in WebSockets module. Using a different library would require significant code changes.

## Getting Help

If you're still having issues:

1. **Check server logs** for WebSocket-related messages
2. **Verify plugin location**: `Engine/Plugins/Runtime/WebSockets/`
3. **Check build logs** for `WITH_WEBSOCKETS_SUPPORT` definition
4. **Ask in Discord**: [Satisfactory Modding Discord](https://discord.gg/QzcG9nX)

## Related Documentation

- [BUILD_REQUIREMENTS.md](BUILD_REQUIREMENTS.md) - Full build requirements
- [TROUBLESHOOTING.md](TROUBLESHOOTING.md) - Common build issues
- [Mods/DiscordChatBridge/DEPENDENCY_EXPLANATION.md](Mods/DiscordChatBridge/DEPENDENCY_EXPLANATION.md) - Why we use WebSockets
- [Mods/DiscordChatBridge/SETUP_GUIDE.md](Mods/DiscordChatBridge/SETUP_GUIDE.md) - Mod setup instructions

## Summary

**Most users should use Option 4 (without WebSockets).** The Discord Chat Bridge mod works perfectly fine without WebSocket support. The only missing feature is real-time presence updates, which most server operators don't need.

If you absolutely need presence features, **Option 1 (copy from standard UE)** is the easiest path to enable WebSockets in your custom build.
