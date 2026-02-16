# Troubleshooting Guide

Quick solutions to common build and setup issues.

## Build Errors

### Expected Warnings: Missing Profiling DLLs

**Warning Messages:**
```
LogWindows: Failed to load 'aqProf.dll' (GetLastError=126)
LogWindows: File 'aqProf.dll' does not exist
LogWindows: Failed to load 'VtuneApi.dll' (GetLastError=126)
LogWindows: File 'VtuneApi.dll' does not exist
LogWindows: Failed to load 'VtuneApi32e.dll' (GetLastError=126)
LogWindows: File 'VtuneApi32e.dll' does not exist
```

**Status:** ✅ **This is normal and safe to ignore**

**Explanation:**

These warnings appear during Unreal Engine startup and are completely harmless. They occur because:

- **aqProf.dll**: AutoDesk/AQTime profiler integration (optional)
- **VtuneApi.dll / VtuneApi32e.dll**: Intel VTune performance profiler integration (optional)

Unreal Engine automatically attempts to load these optional profiling tools at startup. If they're not installed (which is the normal case), the engine logs these warnings and continues normally. The warnings do not indicate any problem with your build or installation.

**Action Required:** None. These warnings can be safely ignored. The engine, editor, and all mods will function perfectly without these optional profiling tools.

### "Unable to find plugin 'WebSockets'" or "cannot open input file UnrealEditor-WebSockets.lib"

**Error Messages:**
```
Unable to find plugin 'WebSockets' (referenced via FactoryGame.uproject). 
Install it and try again, or remove it from the required plugin list.
```
or
```
cannot open input file '..\Intermediate\Build\Win64\x64\UnrealEditor\Development\WebSockets\UnrealEditor-WebSockets.lib'
```

**Solution:**

**Note:** As of the latest update, this error is significantly less likely to occur. The DiscordChatBridge.Build.cs now automatically detects WebSockets availability and only includes it if present. If you still encounter this error, please verify you have the latest code and check for configuration issues, corrupted build files, or path problems.

**What Changed:**
- WebSockets is marked as **optional** in `FactoryGame.uproject` and `DiscordChatBridge.uplugin`
- The build system now **automatically detects** if WebSockets plugin exists in the engine
- If WebSockets is not found, the mod builds successfully **without Gateway/presence features**
- If WebSockets is found, full functionality is enabled

**For Full Gateway Features:**

If you want to use Discord Gateway/presence features, ensure WebSockets is in your engine installation:

1. **For CI/Automated Builds:** The official workflow downloads UE 5.3.2-CSS which includes WebSockets.

2. **For Local Development:**
   
   a. Verify you're using **Unreal Engine 5.3.2-CSS** (the Coffee Stain Studios custom build)
   
   b. Check if WebSockets exists:
      - Windows: `<UE_ROOT>\Engine\Plugins\Runtime\WebSockets\`
      - Linux/Mac: `<UE_ROOT>/Engine/Plugins/Runtime/WebSockets/`
   
   c. If missing, you need to:
      - Use the official CSS UE build (recommended)
      - Or use standard UE 5.3.2 from Epic (WebSockets should be included)
      - Or build UE from source (WebSockets will compile automatically)

**Detailed Information:** See [BUILD_REQUIREMENTS.md](BUILD_REQUIREMENTS.md) for complete details about when and how WebSockets builds.

### "Cannot find Wwise plugin"

**Solution:** Wwise is downloaded separately during the build process from a B2 bucket. Ensure you have the correct credentials configured in the CI workflow or download Wwise manually for local builds.

### Build files are out of date

**Solution:**
```bash
# Delete generated files
rm -rf Intermediate/ Saved/ *.sln *.vcxproj*

# Regenerate project files
# Right-click on FactoryGame.uproject and select "Generate Visual Studio project files"
# Or use UnrealBuildTool to regenerate
```

## Discord Chat Bridge Issues

### Server won't start or mod doesn't load

**Symptoms:**
- Server fails to start after installing the mod
- Mod loads but doesn't work
- Warning messages in server logs

**Solution:**

The mod requires configuration before it will function. If `BotToken` and `ChannelId` are not set in the configuration file, the mod will load but remain inactive.

1. **Check server logs** for these messages:
   - `"DiscordChatSubsystem: Initialized but not configured - mod will not be active"`
   - `"DiscordChatSubsystem: Configuration incomplete - BotToken and ChannelId must be set"`

2. **Configure the mod:**
   - Create/edit the configuration file (see config file location below)
   - Set your `BotToken` (from Discord Developer Portal)
   - Set your `ChannelId` (from Discord, with Developer Mode enabled)
   - See [QUICKSTART.md](Mods/DiscordChatBridge/QUICKSTART.md) for detailed setup instructions

3. **Restart the server** after configuration

**Important:** The mod will NOT work without valid `BotToken` and `ChannelId`. The server will start successfully but the Discord bridge will be inactive until properly configured.

### Messages not sending/receiving

**Checklist:**
- ✅ Bot token is correct in config file
- ✅ Channel ID is correct
- ✅ Bot has been invited to your Discord server
- ✅ Bot has "Message Content Intent" enabled
- ✅ Bot has permissions: Send Messages, Read Message History, View Channels

**Config File Location:**
- Windows: `%localappdata%/FactoryGame/Saved/Config/WindowsServer/DiscordChatBridge.ini`
- Linux: `~/.config/Epic/FactoryGame/Saved/Config/LinuxServer/DiscordChatBridge.ini`

### Gateway/WebSocket presence not working

**Requirements:**
- ✅ `EnableBotActivity=true` in config
- ✅ `UseGatewayForPresence=true` in config
- ✅ "Presence Intent" enabled in Discord Developer Portal
- ✅ WebSockets plugin available (see build error above)

**Check logs for:**
- "Gateway connected successfully" - Connection is working
- WebSocket errors - Check firewall/network
- "Unable to connect to gateway" - Verify bot token and intents

### Server notifications not appearing

**Checklist:**
- ✅ `EnableServerNotifications=true`
- ✅ Bot has permissions in notification channel (if using separate channel)
- ✅ Check server logs for notification-related messages
- ⚠️ Server stop notifications may not send if server crashes

## Development Setup Issues

### Cannot find FactoryGame modules

**Solution:** Ensure you've cloned the full SatisfactoryModLoader repository, not just the Discord Chat Bridge mod. The mod depends on the base game modules.

### UnrealBuildTool errors

**Solution:**
```bash
# Clean UBT cache
rm -rf "$LOCALAPPDATA/UnrealBuildTool"  # Windows
rm -rf ~/.local/share/UnrealBuildTool   # Linux

# Regenerate project files
```

### Missing SML dependency

**Solution:** The Discord Chat Bridge mod requires SML 3.11.3 or later. Ensure SML is present in the `Mods/` directory and is built before building Discord Chat Bridge.

## Getting Help

Still having issues? Here are additional resources:

- **Build Requirements:** [BUILD_REQUIREMENTS.md](BUILD_REQUIREMENTS.md)
- **Discord Chat Bridge Setup:** [Mods/DiscordChatBridge/SETUP_GUIDE.md](Mods/DiscordChatBridge/SETUP_GUIDE.md)
- **Discord Chat Bridge README:** [Mods/DiscordChatBridge/README.md](Mods/DiscordChatBridge/README.md)
- **WebSockets Dependency Details:** [Mods/DiscordChatBridge/DEPENDENCY_EXPLANATION.md](Mods/DiscordChatBridge/DEPENDENCY_EXPLANATION.md)
- **Satisfactory Modding Docs:** [docs.ficsit.app](https://docs.ficsit.app/)
- **Discord Community:** [discord.gg/QzcG9nX](https://discord.gg/QzcG9nX)

## Quick Reference

| Issue | Solution |
|-------|----------|
| Missing profiling DLLs (aqProf, VtuneApi) | ✅ Safe to ignore - optional profiling tools |
| WebSockets build error | Should auto-resolve now; builds without WebSockets if not available |
| Gateway features not working | Ensure WebSockets plugin is in engine (optional) |
| Build files outdated | Delete Intermediate/, regenerate project |
| Discord messages not working | Check bot token, channel ID, permissions |
| Wwise missing | CI downloads it automatically, or download manually |
| Cannot build mod | Ensure SML is present and built first |

## Tips

- Always check the server logs for detailed error messages
- Verify your configuration file has no extra spaces or quotes
- Use the example configurations in `Mods/DiscordChatBridge/Config/ExampleConfigs/`
- Test bot permissions in Discord first before troubleshooting the mod
- For build issues, try a clean build after deleting generated files
