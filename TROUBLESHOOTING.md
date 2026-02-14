# Troubleshooting Guide

Quick solutions to common build and setup issues.

## Build Errors

### "Unable to find plugin 'WebSockets'"

**Error Message:**
```
Unable to find plugin 'WebSockets' (referenced via FactoryGame.uproject). 
Install it and try again, or remove it from the required plugin list.
```

**What this error means:**

This error means the WebSockets plugin cannot be found in your Unreal Engine installation. This is **NOT** a problem with this project - it's an issue with your Unreal Engine setup.

**❓ "Where do I download WebSockets?"**

**You don't!** WebSockets is a built-in Unreal Engine plugin that comes with the engine installation. There is no separate download.

**Solution Steps:**

#### Step 1: Verify Your Unreal Engine Installation

Check if WebSockets exists in your engine:

**Windows:**
```cmd
dir "C:\Program Files\Epic Games\UE_5.3\Engine\Plugins\Runtime\WebSockets"
REM Or wherever your UE is installed
```

**Linux/Mac:**
```bash
ls "<UE_ROOT>/Engine/Plugins/Runtime/WebSockets/"
```

**Note:** In older UE versions, WebSockets may be in `Engine/Plugins/Experimental/WebSockets/`. For UE 5.3.2, it should be in the `Runtime` directory.

**If the directory exists:** Go to Step 3 (plugin exists but isn't being detected)

**If the directory doesn't exist:** Continue to Step 2 (plugin is missing from engine)

#### Step 2: Fix Missing WebSockets Plugin

If WebSockets is missing from your engine, your UE installation is incomplete. Choose one of these solutions:

**Option A: Install UE 5.3.2-CSS (Recommended for this project)**
- The Coffee Stain Studios custom build includes all required plugins
- Follow the setup guide in [Satisfactory Modding Documentation](https://docs.ficsit.app/)
- **For CI builds:** No action needed - the workflow handles this automatically
- **For local development:** You'll need access to the `satisfactorymodding/UnrealEngine` repository

**Option B: Install/Repair Standard UE 5.3.2**
- Open Epic Games Launcher
- Go to Library > Engine Versions
- If UE 5.3.2 is installed: Click the three dots and select "Verify" to repair
- If not installed: Install UE 5.3.2 from the launcher
- WebSockets should be included by default in standard UE installations
- ⚠️ Note: Some CSS-specific features may not work with standard UE

**Option C: Build UE from Source**
- Clone the UE 5.3.2 source code
- Build the engine (WebSockets will compile automatically as part of engine build)
- WebSockets is located in `Engine/Plugins/Runtime/WebSockets/` in the source

#### Step 3: Fix Plugin Detection Issues

If WebSockets exists but still getting the error:

1. **Clean and regenerate project files:**
   
   **Windows (PowerShell):**
   ```powershell
   # Delete generated files
   Remove-Item -Recurse -Force Intermediate/, Saved/, *.sln, *.vcxproj*, .vs/
   
   # Regenerate project files
   # Right-click FactoryGame.uproject > "Generate Visual Studio project files"
   ```
   
   **Linux/Mac (Bash):**
   ```bash
   # Delete generated files
   rm -rf Intermediate/ Saved/ *.sln *.vcxproj* .vs/
   
   # Regenerate project files
   # Use UnrealVersionSelector or generate from IDE
   ```

2. **Verify the plugin is enabled in your engine:**
   - Check that `WebSockets.uplugin` exists in the plugin directory
   - The plugin should have `"Enabled": true` in the plugin descriptor

3. **Check UnrealBuildTool can find the plugin:**
   
   **Windows:**
   ```powershell
   # Clear UBT cache
   Remove-Item -Recurse -Force "$env:LOCALAPPDATA\UnrealBuildTool"
   ```
   
   **Linux/Mac:**
   ```bash
   # Clear UBT cache
   rm -rf ~/.local/share/UnrealBuildTool
   ```

4. **Verify no path issues:**
   - Ensure your engine path doesn't contain special characters
   - Check that UE_ROOT environment variable (if set) points to correct location

#### For CI/Automated Builds

If you're seeing this error in CI:
1. ✅ Ensure the workflow downloads the correct UE build
2. ✅ Check that UE extraction completed successfully
3. ✅ Verify engine registration step succeeded
4. ✅ Look for earlier errors in the workflow logs

The official CI workflow automatically downloads UE 5.3.2-CSS which includes WebSockets, so this error shouldn't occur in properly configured CI environments.

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
| WebSockets plugin missing | Use UE 5.3.2-CSS or ensure plugin is in engine |
| Build files outdated | Delete Intermediate/, regenerate project |
| Discord messages not working | Check bot token, channel ID, permissions |
| Gateway not working | Enable Presence Intent, check WebSockets |
| Wwise missing | CI downloads it automatically, or download manually |
| Cannot build mod | Ensure SML is present and built first |

## Tips

- Always check the server logs for detailed error messages
- Verify your configuration file has no extra spaces or quotes
- Use the example configurations in `Mods/DiscordChatBridge/Config/ExampleConfigs/`
- Test bot permissions in Discord first before troubleshooting the mod
- For build issues, try a clean build after deleting generated files
