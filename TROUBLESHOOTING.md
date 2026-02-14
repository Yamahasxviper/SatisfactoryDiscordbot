# Troubleshooting Guide

Quick solutions to common build and setup issues.

## Build Errors

### "The following modules are missing or built with a different engine version"

**Error Message:**
```
The following modules are missing or built with a different engine version:

  FactoryGame
  FactoryEditor
  FactoryPreEarlyLoadingScreen
  FactoryDedicatedServer
  FactoryDedicatedClient
  DummyHeaders
  InstancedSplinesComponent
  GameplayEvents
  AbstractInstance
  SignificanceISPC
  ReliableMessaging
  ReliableMessagingEOSP2P
  ReliableMessagingSteamP2P
  ReliableMessagingTCP
  Wwise
  (+25 others, see log for details)

Would you like to rebuild them now?
```

**What This Means:**

This dialog appears when you try to open `FactoryGame.uproject` in Unreal Engine and the compiled binaries are either:
- Missing (first time opening the project or after cleaning)
- Built with a different engine version (after updating Unreal Engine)
- Out of date (after code changes)

**Solution:**

✅ **Yes, click "Yes" to rebuild!** This is the normal and expected behavior for Unreal Engine projects.

**What happens when you click "Yes":**
1. Unreal Engine will compile all project modules
2. This process can take 10-30 minutes depending on your system
3. You'll see a progress bar showing compilation status
4. Once complete, the editor will open automatically

**Requirements before rebuilding:**

Before clicking "Yes", ensure you have:
- ✅ **Unreal Engine 5.3.2-CSS** installed and registered
- ✅ **Visual Studio 2022** with C++ development tools (Windows) or appropriate build tools (Linux)
- ✅ **Wwise plugin** downloaded and placed in `Plugins/Wwise/` directory
- ✅ At least **50GB** of free disk space for build artifacts
- ✅ A **stable internet connection** (for downloading dependencies)

**Alternative: Build from Command Line**

If you prefer to build from the command line instead of through the Unreal Editor dialog:

**Windows:**
```powershell
# Navigate to project directory
cd C:\Path\To\SatisfactoryDiscordbot

# Build the editor target
& "C:\Path\To\UE_5.3.2-CSS\Engine\Build\BatchFiles\Build.bat" FactoryEditor Win64 Development -project="$PWD\FactoryGame.uproject"
```

**Linux:**
```bash
# Navigate to project directory
cd /path/to/SatisfactoryDiscordbot

# Build the editor target
/path/to/UE_5.3.2-CSS/Engine/Build/BatchFiles/Linux/Build.sh FactoryEditor Linux Development -project="$(pwd)/FactoryGame.uproject"
```

**After building from command line**, you can open the `.uproject` file normally and it won't ask to rebuild.

**CI/Automated Builds:**

For automated builds, see the [CI workflow](.github/workflows/build.yml) which:
1. Downloads and sets up UE 5.3.2-CSS automatically
2. Downloads Wwise from B2 bucket
3. Builds all modules using UnrealBuildTool
4. Packages the SML mod

**Common Issues:**

- **"Cannot find Visual Studio"** → Install Visual Studio 2022 with "Game Development with C++" workload
- **"Wwise plugin not found"** → See [Wwise plugin section](#cannot-find-wwise-plugin) below
- **Build fails with errors** → Check that you're using the correct engine version (5.3.2-CSS)
- **Out of memory during build** → Close other applications and ensure at least 16GB RAM available
- **Build takes forever** → This is normal for first builds; subsequent builds are much faster

**Detailed Information:** See [BUILD_REQUIREMENTS.md](BUILD_REQUIREMENTS.md) for complete build setup instructions.

### "Unable to find plugin 'WebSockets'"

**Error Message:**
```
Unable to find plugin 'WebSockets' (referenced via FactoryGame.uproject). 
Install it and try again, or remove it from the required plugin list.
```

**Solution:**

This error means the WebSockets plugin is not found in your Unreal Engine installation. WebSockets is a built-in engine plugin that should be present.

**Quick Fixes:**

1. **For CI/Automated Builds:** This should not happen if using the official workflow. The workflow downloads UE 5.3.2-CSS which includes WebSockets.

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
| Modules missing/different engine version | Click "Yes" to rebuild - this is normal! |
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
