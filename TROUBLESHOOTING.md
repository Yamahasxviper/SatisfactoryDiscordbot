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
cd C:\Path\To\SatisfactoryModLoader

# Build the editor target
& "C:\Path\To\UE_5.3.2-CSS\Engine\Build\BatchFiles\Build.bat" FactoryEditor Win64 Development -project="$PWD\FactoryGame.uproject"
```

**Linux:**
```bash
# Navigate to project directory
cd /path/to/SatisfactoryModLoader

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

### Unreal Engine closes without errors (silent crash)

**Problem:**

The Unreal Engine editor suddenly closes with no error dialog, crash reporter, or warning message. This is often called a "silent crash" or "silent exit."

**Common Causes:**

1. **Out of Memory (OOM)** - Most common cause
2. **Shader compilation issues** - Especially on first project open
3. **Corrupted project files** - Bad assets or configuration
4. **Plugin conflicts** - Incompatible or corrupted plugins
5. **Graphics driver issues** - Outdated or unstable GPU drivers
6. **Disk space exhausted** - Running out of space during compilation
7. **Antivirus interference** - Antivirus blocking or quarantining files

**Step-by-Step Troubleshooting:**

#### 1. Check Crash Logs

Even when UE closes "without errors", it usually writes crash logs. Check these locations:

**Windows:**
```
%localappdata%\UnrealEngine\<VERSION>\Saved\Logs\
%localappdata%\FactoryGame\Saved\Logs\
<ProjectDir>\Saved\Logs\
```

**Linux:**
```
~/.config/Epic/UnrealEngine/<VERSION>/Saved/Logs/
~/.config/Epic/FactoryGame/Saved/Logs/
<ProjectDir>/Saved/Logs/
```

Look for the most recent `.log` file. Search for keywords:
- `Error:`
- `Fatal:`
- `Assertion failed`
- `Out of memory`
- `Access violation`

#### 2. Check System Resources

**Memory:**
- **Minimum:** 16GB RAM
- **Recommended:** 32GB+ RAM for large projects
- Monitor RAM usage in Task Manager/System Monitor while opening project

**Disk Space:**
- Ensure at least **50GB free** on drive containing:
  - Project directory
  - Temp directory (`%TEMP%` on Windows, `/tmp` on Linux)
  - Derived Data Cache (`%localappdata%\UnrealEngine\Common\DerivedDataCache`)

**Close unnecessary applications** before opening Unreal Engine.

#### 3. Clean and Rebuild

Try a clean rebuild of the project:

```bash
# Navigate to project directory
cd /path/to/SatisfactoryModLoader

# Delete all generated files
rm -rf Intermediate/
rm -rf Saved/
rm -rf .vs/
rm -rf *.sln
rm -rf *.vcxproj*
rm -rf DerivedDataCache/

# Delete Derived Data Cache (safe to delete, will regenerate)
# Windows:
rm -rf "$env:LOCALAPPDATA\UnrealEngine\Common\DerivedDataCache"
# Linux:
rm -rf ~/.local/share/UnrealEngine/Common/DerivedDataCache

# Rebuild from command line instead of editor
# Windows:
& "C:\Path\To\UE_5.3.2-CSS\Engine\Build\BatchFiles\Build.bat" FactoryEditor Win64 Development -project="$(pwd)\FactoryGame.uproject"
# Linux:
/path/to/UE_5.3.2-CSS/Engine/Build/BatchFiles/Linux/Build.sh FactoryEditor Linux Development -project="$(pwd)/FactoryGame.uproject"
```

#### 4. Check for Corrupted Files

**Verify Project Integrity:**
```bash
# Check if .uproject file is valid JSON
# Windows PowerShell:
Get-Content FactoryGame.uproject | ConvertFrom-Json
# Linux:
python3 -m json.tool FactoryGame.uproject
```

**Check Plugin Integrity:**
- Ensure Wwise plugin is properly extracted in `Plugins/Wwise/`
- Verify all plugin `.uplugin` files are valid JSON
- Check that plugin binaries exist if pre-compiled

#### 5. Update Graphics Drivers

Outdated or buggy graphics drivers can cause silent crashes:

- **NVIDIA:** Download latest drivers from nvidia.com
- **AMD:** Download latest drivers from amd.com
- **Intel:** Update through Windows Update or intel.com

After updating, restart your computer.

#### 6. Disable Antivirus Temporarily

Some antivirus software interferes with UE compilation:

1. Temporarily disable antivirus
2. Add exclusions for:
   - Unreal Engine installation directory
   - Project directory
   - Visual Studio directory
   - Temp directories

#### 7. Check Windows Event Viewer (Windows Only)

Windows Event Viewer might have additional information:

1. Open Event Viewer (`eventvwr.msc`)
2. Go to **Windows Logs → Application**
3. Look for errors around the time UE crashed
4. Check for entries from "UnrealEditor" or "Application Error"

#### 8. Run Editor with Logging

Start the editor from command line to see live output:

**Windows:**
```powershell
& "C:\Path\To\UE_5.3.2-CSS\Engine\Binaries\Win64\UnrealEditor.exe" "$(pwd)\FactoryGame.uproject" -log -stdout
```

**Linux:**
```bash
/path/to/UE_5.3.2-CSS/Engine/Binaries/Linux/UnrealEditor "$(pwd)/FactoryGame.uproject" -log -stdout
```

Watch the console output for any errors or warnings before the crash.

#### 9. Verify Engine Installation

Ensure UE 5.3.2-CSS is correctly installed:

```bash
# Check if engine is registered
# Windows:
reg query "HKEY_CURRENT_USER\Software\Epic Games\Unreal Engine\Builds"
# Linux:
cat ~/.config/Epic/UnrealEngine/Install.ini
```

If engine registration is missing, re-run the registration script:
- Windows: `<UE_ROOT>\SetupScripts\Register.bat`
- Linux: `<UE_ROOT>/SetupScripts/Register.sh`

#### 10. Check for Known Issues

**Wwise-related crashes:**
- Ensure Wwise plugin is version-compatible with UE 5.3.2
- Try disabling Wwise temporarily to isolate the issue
- Check that Wwise patches have been applied (see PreBuildSteps in .uproject)

**Shader compilation crashes:**
- First project open compiles thousands of shaders
- Can take 30+ minutes and use significant RAM
- If crashing during shader compilation, increase virtual memory/swap

#### 11. Reduce Initial Load

If crashing on project open, try:

1. **Start with empty level:**
   ```bash
   UnrealEditor.exe FactoryGame.uproject -game -EmptyLevel
   ```

2. **Skip shader compilation:**
   ```bash
   UnrealEditor.exe FactoryGame.uproject -noshadercompile
   ```
   Note: This is temporary troubleshooting only

3. **Disable plugins one by one:**
   Edit `FactoryGame.uproject` and temporarily disable plugins to isolate conflicts

**Quick Fixes Summary:**

| Symptom | Likely Cause | Solution |
|---------|--------------|----------|
| Crashes after a few minutes | Out of memory | Close other apps, upgrade to 32GB RAM |
| Crashes during shader compilation | Insufficient resources | Wait longer, increase virtual memory |
| Crashes immediately on open | Corrupted files | Clean rebuild, verify integrity |
| Crashes after recent changes | New code/asset issue | Revert recent changes, check logs |
| Crashes on specific hardware | Driver issues | Update graphics drivers |
| Crashes with antivirus warning | Antivirus blocking | Add exclusions |

**Still Crashing?**

If none of the above solutions work:

1. **Check the crash logs** - They usually contain the answer
2. **Ask in Discord** - [discord.gg/QzcG9nX](https://discord.gg/QzcG9nX)
3. **Check Satisfactory Modding Docs** - [docs.ficsit.app](https://docs.ficsit.app/)
4. **Verify hardware meets requirements** - UE5 is demanding!

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
| "Modules are missing or built with a different engine version" | Click "Yes" to rebuild - this is normal! |
| Unreal Engine closes without errors | Check crash logs, verify RAM (16GB+), clean rebuild |
| WebSockets plugin missing | Use UE 5.3.2-CSS or ensure plugin is in engine |
| Build files outdated | Delete Intermediate/, regenerate project |
| Discord messages not working | Check bot token, channel ID, permissions |
| Gateway not working | Enable Presence Intent, check WebSockets |
| Wwise missing | CI downloads it automatically, or download manually |
| Cannot build mod | Ensure SML is present and built first |
| Editor crashes during shader compilation | Be patient (30+ min first time), increase RAM/swap |
| Out of memory during build/open | Close other apps, upgrade to 32GB RAM |

## Tips

- Always check the server logs for detailed error messages
- Verify your configuration file has no extra spaces or quotes
- Use the example configurations in `Mods/DiscordChatBridge/Config/ExampleConfigs/`
- Test bot permissions in Discord first before troubleshooting the mod
- For build issues, try a clean build after deleting generated files
