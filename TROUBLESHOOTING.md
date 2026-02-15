# Troubleshooting Guide

Quick solutions to common build and setup issues.

## Build Errors

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

**Quick Solution:**

**Note:** As of the latest update, this error is significantly less likely to occur. The DiscordChatBridge.Build.cs now automatically detects WebSockets availability and only includes it if present. If you still encounter this error, please verify you have the latest code and check for configuration issues, corrupted build files, or path problems.

📖 **For comprehensive solutions, see [WEBSOCKET_ALTERNATIVES.md](WEBSOCKET_ALTERNATIVES.md)**

**Quick Options:**

1. **Use without WebSockets** (Recommended - easiest)
   - The mod works fine without WebSockets!
   - All core features function normally (chat, notifications, commands)
   - Only missing: real-time bot presence updates
   - No additional setup required
   - See [WEBSOCKET_ALTERNATIVES.md](WEBSOCKET_ALTERNATIVES.md#option-4-use-without-websockets-recommended-for-most-users)

2. **Add WebSockets to your custom build** (If you need presence)
   - Copy from standard UE 5.3.2 → See [WEBSOCKET_ALTERNATIVES.md](WEBSOCKET_ALTERNATIVES.md#option-1-copy-websocket-plugin-from-standard-unreal-engine-easiest)
   - Build from source → See [WEBSOCKET_ALTERNATIVES.md](WEBSOCKET_ALTERNATIVES.md#option-2-build-websocket-plugin-from-unreal-engine-source)

**What Changed:**
- WebSockets is marked as **optional** in `FactoryGame.uproject` and `DiscordChatBridge.uplugin`
- The build system now **automatically detects** if WebSockets plugin exists in the engine
- If WebSockets is not found, the mod builds successfully **without Gateway/presence features**
- If WebSockets is found, full functionality is enabled

**Verify Your Setup:**

Check if WebSockets exists in your engine:
```bash
# Windows:
dir "<UE_ROOT>\Engine\Plugins\Runtime\WebSockets\"

# Linux/Mac:
ls "<UE_ROOT>/Engine/Plugins/Runtime/WebSockets/"
```

**For More Help:**
- Complete guide: [WEBSOCKET_ALTERNATIVES.md](WEBSOCKET_ALTERNATIVES.md)
- Build details: [BUILD_REQUIREMENTS.md](BUILD_REQUIREMENTS.md)

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

- **WebSocket Solutions:** [WEBSOCKET_ALTERNATIVES.md](WEBSOCKET_ALTERNATIVES.md) ← Start here for WebSocket issues
- **Build Requirements:** [BUILD_REQUIREMENTS.md](BUILD_REQUIREMENTS.md)
- **Discord Chat Bridge Setup:** [Mods/DiscordChatBridge/SETUP_GUIDE.md](Mods/DiscordChatBridge/SETUP_GUIDE.md)
- **Discord Chat Bridge README:** [Mods/DiscordChatBridge/README.md](Mods/DiscordChatBridge/README.md)
- **WebSockets Dependency Details:** [Mods/DiscordChatBridge/DEPENDENCY_EXPLANATION.md](Mods/DiscordChatBridge/DEPENDENCY_EXPLANATION.md)
- **Satisfactory Modding Docs:** [docs.ficsit.app](https://docs.ficsit.app/)
- **Discord Community:** [discord.gg/QzcG9nX](https://discord.gg/QzcG9nX)

## Quick Reference

| Issue | Solution |
|-------|----------|
| WebSockets build error | See [WEBSOCKET_ALTERNATIVES.md](WEBSOCKET_ALTERNATIVES.md) - mod works without it! |
| Custom Unreal missing WebSockets | See [WEBSOCKET_ALTERNATIVES.md](WEBSOCKET_ALTERNATIVES.md) for 4 options |
| Gateway features not working | Either add WebSockets or use REST-only mode (see guide) |
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
