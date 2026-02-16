# Troubleshooting Guide

Quick solutions to common build and setup issues.

## ⚠️ IMPORTANT: Server Starts But Discord Doesn't Work?

**This is normal if you haven't configured the mod yet!**

### Symptoms:
- Server starts successfully with no errors
- Players can join and play normally
- Discord messages don't sync
- You see warnings in logs about "NOT CONFIGURED" or "INACTIVE"

### Solution:

**The DiscordChatBridge mod requires configuration before it will work.** The server will start normally, but Discord integration will be inactive until you configure it.

**What you need to do:**

1. **Create a Discord bot** at https://discord.com/developers/applications
2. **Get your bot token** (from the Bot section)
3. **Get your channel ID** (enable Developer Mode in Discord, right-click channel, Copy ID)
4. **Configure the mod** using ONE of these methods:
   
   **Method 1: config TXT format (Recommended)**
   - Edit: `Mods/DiscordChatBridge/config/DiscordChatBridge.txt`
   - Set: `BotToken=YOUR_TOKEN_HERE`
   - Set: `ChannelId=YOUR_CHANNEL_ID_HERE`
   
   **Method 2: INI format (Legacy)**
   - Windows: `%localappdata%\FactoryGame\Saved\Config\WindowsServer\DiscordChatBridge.ini`
   - Linux: `~/.config/Epic/FactoryGame/Saved/Config/LinuxServer/DiscordChatBridge.ini`
   - Add configuration section and settings (see QUICKSTART.md)

5. **Restart the server** - Discord integration will now be active!

**See the [Quick Start Guide](Mods/DiscordChatBridge/help/QUICKSTART.md) for step-by-step setup instructions.**

---

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

### Server starts but Discord integration doesn't work

**This is the #1 most common issue - see the section at the top of this document.**

**Symptoms:**
- Server starts successfully (no errors!)
- Players can join and play normally
- Discord messages don't sync
- Warnings in logs: "NOT CONFIGURED" or "INACTIVE"

**Root Cause:** The mod requires configuration before it works. Without configuration, the server starts normally but Discord integration is inactive.

**Solution:** Follow the configuration steps at the top of this troubleshooting guide or see [QUICKSTART.md](Mods/DiscordChatBridge/help/QUICKSTART.md).

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
