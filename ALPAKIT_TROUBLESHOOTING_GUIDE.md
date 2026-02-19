# Alpakit Troubleshooting Guide - DiscordBot & CustomWebSocket

**Last Updated:** February 19, 2026  
**Status:** Complete Solution Guide

---

## Table of Contents

1. [Quick Fix Checklist](#quick-fix-checklist)
2. [Understanding the Problem](#understanding-the-problem)
3. [Prerequisites](#prerequisites)
4. [Step-by-Step Verification](#step-by-step-verification)
5. [Common Issues & Solutions](#common-issues--solutions)
6. [Using Alpakit](#using-alpakit)
7. [Alternative Build Methods](#alternative-build-methods)
8. [Getting Help](#getting-help)

---

## Quick Fix Checklist

If Alpakit is not compiling your Discord bot or WebSocket projects, verify these items:

✅ **1. Alpakit.ini Files Exist**
```bash
# Check if these files exist:
ls -la Mods/DiscordBot/Config/Alpakit.ini
ls -la Plugins/CustomWebSocket/Config/Alpakit.ini
ls -la Mods/SML/Config/Alpakit.ini
```

✅ **2. Plugins Registered in FactoryGame.uproject**
```bash
# Look for these entries in FactoryGame.uproject:
grep -A 2 '"Name": "CustomWebSocket"' FactoryGame.uproject
grep -A 2 '"Name": "SML"' FactoryGame.uproject
grep -A 2 '"Name": "DiscordBot"' FactoryGame.uproject
```

✅ **3. Plugin Files Exist**
```bash
# Verify plugin descriptor files exist:
ls -la Mods/DiscordBot/DiscordBot.uplugin
ls -la Plugins/CustomWebSocket/CustomWebSocket.uplugin
ls -la Mods/SML/SML.uplugin
```

✅ **4. Correct Unreal Engine Version**
- Required: Unreal Engine 5.3.2-CSS (Custom Satisfactory version)
- Check: Open FactoryGame.uproject and look for `"EngineAssociation": "5.3.2-CSS"`

✅ **5. All Dependencies Installed**
- Wwise Audio Plugin (see CI workflow for download)
- Visual Studio 2022 (Windows) or appropriate build tools (Linux)

---

## Understanding the Problem

### What is Alpakit?

Alpakit is an Unreal Engine editor plugin that simplifies packaging Satisfactory mods:
- Automatically compiles mods for multiple platforms
- Creates ZIP files ready for distribution
- Integrates with the Satisfactory Mod Manager
- Provides a user-friendly GUI in the Unreal Editor

### Why Wasn't It Working?

**Problem:** Alpakit couldn't see the DiscordBot mod or CustomWebSocket plugin.

**Root Cause:** Missing configuration files. Alpakit requires a `Config/Alpakit.ini` file in each plugin directory to:
1. Know the plugin exists and should be packaged
2. Determine which target platforms to compile for (Windows, WindowsServer, LinuxServer)

**Solution Applied:** Added `Alpakit.ini` files to both DiscordBot and CustomWebSocket directories.

---

## Prerequisites

### Required Software

1. **Unreal Engine 5.3.2-CSS**
   - Custom Satisfactory version
   - Download from Coffee Stain Studios
   - Not the standard Epic Games Launcher version

2. **Wwise Audio Plugin**
   - Required for Satisfactory audio
   - See `.github/workflows/build.yml` for download URL
   - Install before opening the project

3. **Build Tools**
   - **Windows:** Visual Studio 2022 with C++ workload
   - **Linux:** Clang 13 or later, build-essential

4. **Git**
   - For cloning the repository
   - Git LFS enabled (for large binary files)

### System Requirements

- **Disk Space:** 100+ GB free (Unreal Engine + Project)
- **RAM:** 16 GB minimum, 32 GB recommended
- **CPU:** 8+ cores recommended for faster compilation
- **OS:** Windows 10/11 or Ubuntu 20.04+

---

## Step-by-Step Verification

### Step 1: Verify Repository Structure

```bash
cd /path/to/SatisfactoryDiscordbot

# Expected directory structure:
tree -L 2 -d
# Should show:
# ├── Mods/
# │   ├── DiscordBot/
# │   ├── SML/
# │   └── ExampleMod/
# ├── Plugins/
# │   ├── CustomWebSocket/
# │   └── AbstractInstance/
# ├── Source/
# └── Content/
```

### Step 2: Verify Alpakit Configuration Files

```bash
# Check DiscordBot Alpakit.ini
cat Mods/DiscordBot/Config/Alpakit.ini
# Expected output:
# [ModTargets]
# Targets=Windows
# Targets=WindowsServer
# Targets=LinuxServer

# Check CustomWebSocket Alpakit.ini
cat Plugins/CustomWebSocket/Config/Alpakit.ini
# Expected output:
# [ModTargets]
# Targets=Windows
# Targets=WindowsServer
# Targets=LinuxServer

# Check SML Alpakit.ini (for reference)
cat Mods/SML/Config/Alpakit.ini
# Expected output:
# [ModTargets]
# Targets=Windows
# Targets=WindowsServer
# Targets=LinuxServer
```

**If files are missing:** They should already be present in this repository. If not, create them with the content shown above.

### Step 3: Verify Plugin Registration

```bash
# Check FactoryGame.uproject for plugin entries
grep -A 3 '"Name": "CustomWebSocket"' FactoryGame.uproject
# Expected output:
# {
#   "Name": "CustomWebSocket",
#   "Enabled": true
# }

grep -A 3 '"Name": "SML"' FactoryGame.uproject
# Expected output:
# {
#   "Name": "SML",
#   "Enabled": true
# }

grep -A 3 '"Name": "DiscordBot"' FactoryGame.uproject
# Expected output:
# {
#   "Name": "DiscordBot",
#   "Enabled": true
# }
```

**If plugins are not registered:** Add them to the `"Plugins"` array in `FactoryGame.uproject`.

### Step 4: Verify Plugin Descriptor Files

```bash
# Check DiscordBot.uplugin
cat Mods/DiscordBot/DiscordBot.uplugin | grep -A 5 '"Modules"'
# Should show module configuration with PlatformAllowList

# Check CustomWebSocket.uplugin
cat Plugins/CustomWebSocket/CustomWebSocket.uplugin | grep -A 5 '"Modules"'
# Should show module configuration with PlatformAllowList

# Verify dependencies in DiscordBot.uplugin
cat Mods/DiscordBot/DiscordBot.uplugin | grep -A 10 '"Plugins"'
# Should show dependencies on SML and CustomWebSocket
```

### Step 5: Test Compilation (Without Alpakit)

Before using Alpakit, verify that the plugins compile with Unreal Build Tool:

```bash
# Generate project files (Windows)
"C:\Program Files\Unreal Engine\UE_5.3.2-CSS\Engine\Build\BatchFiles\Build.bat" ^
  -projectfiles -project="%CD%\FactoryGame.uproject" -game -engine

# Build the editor (Windows)
"C:\Program Files\Unreal Engine\UE_5.3.2-CSS\Engine\Build\BatchFiles\Build.bat" ^
  FactoryEditor Win64 Development ^
  -project="%CD%\FactoryGame.uproject"

# On Linux:
/path/to/UnrealEngine/Engine/Build/BatchFiles/Linux/Build.sh \
  FactoryEditor Linux Development \
  -project="/path/to/FactoryGame.uproject"
```

**Expected output:**
```
Building FactoryGame...
Building CustomWebSocket...
Building SML...
Building DiscordBot...
...
Build succeeded
```

**If compilation fails:** See [Common Issues & Solutions](#common-issues--solutions) below.

---

## Using Alpakit

### Opening Alpakit in Unreal Editor

1. **Launch the Unreal Editor:**
   ```bash
   # Windows
   "C:\Program Files\Unreal Engine\UE_5.3.2-CSS\Engine\Binaries\Win64\UnrealEditor.exe" ^
     "%CD%\FactoryGame.uproject"
   
   # Linux
   /path/to/UnrealEngine/Engine/Binaries/Linux/UnrealEditor \
     /path/to/FactoryGame.uproject
   ```

2. **Wait for the editor to load** (may take several minutes the first time)

3. **Open Alpakit:**
   - Click **Tools** in the menu bar
   - Select **Alpakit** from the dropdown
   - The Alpakit window will open

### Verifying Plugins Appear in Alpakit

In the Alpakit window, you should see:

✅ **SML** - Satisfactory Mod Loader  
✅ **DiscordBot** - Discord Bot Integration  
✅ **CustomWebSocket** - WebSocket Plugin  
✅ **ExampleMod** - Example Mod (optional)

**If plugins are missing:**
1. Close the editor
2. Verify Alpakit.ini files exist (Step 2 above)
3. Regenerate project files
4. Reopen the editor

### Packaging with Alpakit

1. **Select the mods to package:**
   - Check the boxes for **DiscordBot** and **CustomWebSocket**
   - (Optional) Also select **SML** if needed

2. **Choose target platforms:**
   - Each mod shows available targets (Windows, WindowsServer, LinuxServer)
   - Select the platforms you want to build for

3. **Click "Alpakit!" button**

4. **Wait for compilation:**
   - Progress will be shown in the Output Log
   - May take 5-30 minutes depending on your system

5. **Verify output:**
   ```bash
   # Check for packaged files:
   ls -la Saved/ArchivedPlugins/DiscordBot/
   ls -la Saved/ArchivedPlugins/CustomWebSocket/
   ls -la Saved/ArchivedPlugins/SML/
   
   # Each should contain ZIP files for each platform
   ```

### Expected Output Files

After successful packaging, you should have:

```
Saved/ArchivedPlugins/
├── DiscordBot/
│   ├── DiscordBot_Windows.zip
│   ├── DiscordBot_WindowsServer.zip
│   └── DiscordBot_LinuxServer.zip
├── CustomWebSocket/
│   ├── CustomWebSocket_Windows.zip
│   ├── CustomWebSocket_WindowsServer.zip
│   └── CustomWebSocket_LinuxServer.zip
└── SML/
    ├── SML_Windows.zip
    ├── SML_WindowsServer.zip
    └── SML_LinuxServer.zip
```

---

## Common Issues & Solutions

### Issue 1: "Alpakit doesn't show DiscordBot or CustomWebSocket"

**Symptoms:**
- Alpakit window opens but only shows SML and ExampleMod
- DiscordBot and CustomWebSocket are missing from the list

**Solution:**
```bash
# 1. Verify Alpakit.ini files exist
ls -la Mods/DiscordBot/Config/Alpakit.ini
ls -la Plugins/CustomWebSocket/Config/Alpakit.ini

# 2. If missing, create them:
mkdir -p Mods/DiscordBot/Config
cat > Mods/DiscordBot/Config/Alpakit.ini << 'EOF'
[ModTargets]
Targets=Windows
Targets=WindowsServer
Targets=LinuxServer
EOF

mkdir -p Plugins/CustomWebSocket/Config
cat > Plugins/CustomWebSocket/Config/Alpakit.ini << 'EOF'
[ModTargets]
Targets=Windows
Targets=WindowsServer
Targets=LinuxServer
EOF

# 3. Restart the Unreal Editor
```

---

### Issue 2: "Plugin 'SML' not found" or "Plugin 'CustomWebSocket' not found"

**Symptoms:**
- Compilation errors about missing dependencies
- DiscordBot fails to compile

**Solution:**
```bash
# 1. Verify plugins are enabled in FactoryGame.uproject
grep -A 2 '"Name": "SML"' FactoryGame.uproject
grep -A 2 '"Name": "CustomWebSocket"' FactoryGame.uproject

# Both should show "Enabled": true

# 2. If not enabled, edit FactoryGame.uproject and add:
# {
#   "Name": "SML",
#   "Enabled": true
# },
# {
#   "Name": "CustomWebSocket",
#   "Enabled": true
# },
# {
#   "Name": "DiscordBot",
#   "Enabled": true
# }

# 3. Regenerate project files
# Windows:
UnrealEngine\Engine\Build\BatchFiles\Build.bat -projectfiles -project="FactoryGame.uproject"

# Linux:
UnrealEngine/Engine/Build/BatchFiles/Linux/Build.sh -projectfiles -project="FactoryGame.uproject"
```

---

### Issue 3: "Could not find module 'CustomWebSocket'"

**Symptoms:**
- DiscordBot compilation fails with module not found error
- CustomWebSocket compiles but DiscordBot doesn't see it

**Solution:**

This is a **build order issue**. CustomWebSocket must compile before DiscordBot.

```bash
# 1. Check dependency in DiscordBot.uplugin
cat Mods/DiscordBot/DiscordBot.uplugin | grep -A 5 '"Plugins"'

# Should show:
# "Plugins": [
#   {
#     "Name": "SML",
#     "Enabled": true,
#     "SemVersion": "^3.11.3"
#   },
#   {
#     "Name": "CustomWebSocket",
#     "Enabled": true,
#     "SemVersion": "^1.0.0"
#   }
# ]

# 2. Verify LoadingPhase in plugin descriptors:
# CustomWebSocket should have: "LoadingPhase": "PreDefault"
# DiscordBot should have: "LoadingPhase": "PostDefault"

# 3. Build in correct order manually:
# First build CustomWebSocket, then DiscordBot
```

---

### Issue 4: "Wrong Unreal Engine version"

**Symptoms:**
- Project won't open
- Error: "The project was created with Unreal Engine 5.3.2-CSS"

**Solution:**

You need the **custom Satisfactory version** of Unreal Engine, not the standard version.

1. **Uninstall standard Unreal Engine 5.3** (if installed from Epic Games Launcher)
2. **Download Unreal Engine 5.3.2-CSS** from Coffee Stain Studios
3. **Install to a separate directory**
4. **Update your environment:**
   ```bash
   # Windows: Update PATH to point to custom engine
   # Linux: Update symlinks or aliases
   ```

---

### Issue 5: "Missing Wwise plugin"

**Symptoms:**
- Compilation errors about missing Wwise headers
- "WwiseAudioComponent" not found

**Solution:**

```bash
# 1. Download Wwise plugin for Unreal Engine 5.3
# See .github/workflows/build.yml for download URL

# 2. Install Wwise plugin:
# Windows:
# - Run Wwise Launcher
# - Install Wwise 2022.1.x
# - Install Unreal Engine Integration
# - Copy to: UnrealEngine/Engine/Plugins/Wwise/

# Linux:
# - Extract Wwise tarball
# - Copy to: UnrealEngine/Engine/Plugins/Wwise/

# 3. Verify installation:
ls -la "C:\Program Files\Unreal Engine\UE_5.3.2-CSS\Engine\Plugins\Wwise"
# or
ls -la /path/to/UnrealEngine/Engine/Plugins/Wwise

# 4. Regenerate project files
```

---

### Issue 6: "Out of memory during compilation"

**Symptoms:**
- Compilation stops with "fatal error C1060: compiler is out of heap space"
- System becomes unresponsive during build

**Solution:**

```bash
# Option 1: Reduce parallel compilation
# Edit: Engine/Saved/UnrealBuildTool/BuildConfiguration.xml
# Add:
# <Configuration>
#   <MaxParallelActions>4</MaxParallelActions>
# </Configuration>

# Option 2: Increase system virtual memory
# Windows: System Properties > Advanced > Performance Settings > Advanced > Virtual Memory
# Increase pagefile size to 32 GB

# Option 3: Close other applications
# Close browser, IDE, and other memory-intensive apps

# Option 4: Build in stages
# Build CustomWebSocket first:
Build.bat CustomWebSocket Win64 Development
# Then build DiscordBot:
Build.bat DiscordBot Win64 Development
```

---

### Issue 7: "Access denied" or "Permission denied" during packaging

**Symptoms:**
- Alpakit fails to create ZIP files
- Error about file permissions

**Solution:**

```bash
# Windows: Run Unreal Editor as Administrator
# Right-click UnrealEditor.exe > Run as administrator

# Linux: Check directory permissions
chmod -R u+w Saved/
chmod -R u+w Mods/
chmod -R u+w Plugins/

# Also check antivirus isn't blocking file creation
# Add exception for Saved/ArchivedPlugins/ directory
```

---

### Issue 8: "Packaged mod doesn't include config files"

**Symptoms:**
- Mod packages successfully but config files are missing from ZIP
- Server can't find configuration options

**Solution:**

Verify `FilterPlugin.ini` includes config files:

```bash
# Check DiscordBot FilterPlugin.ini
cat Mods/DiscordBot/Config/FilterPlugin.ini

# Should include:
# [FilterPlugin]
# /Config/...
# /Config/Alpakit.ini
# /Config/FilterPlugin.ini

# If missing, add these lines to each FilterPlugin.ini
```

---

## Alternative Build Methods

If Alpakit continues to have issues, you can package plugins manually using UAT (Unreal Automation Tool).

### Method 1: UAT Command Line (Windows)

```powershell
# Set variables
$EnginePath = "C:\Program Files\Unreal Engine\UE_5.3.2-CSS"
$ProjectPath = "C:\Path\To\FactoryGame.uproject"

# Package CustomWebSocket
& "$EnginePath\Engine\Build\BatchFiles\RunUAT.bat" `
  -ScriptsForProject="$ProjectPath" `
  PackagePlugin `
  -Project="$ProjectPath" `
  -dlcname=CustomWebSocket `
  -merge -build -server `
  -clientconfig=Shipping `
  -serverconfig=Shipping `
  -platform=Win64 `
  -serverplatform=Win64+Linux `
  -nocompileeditor `
  -installed

# Package DiscordBot (depends on CustomWebSocket and SML)
& "$EnginePath\Engine\Build\BatchFiles\RunUAT.bat" `
  -ScriptsForProject="$ProjectPath" `
  PackagePlugin `
  -Project="$ProjectPath" `
  -dlcname=DiscordBot `
  -merge -build -server `
  -clientconfig=Shipping `
  -serverconfig=Shipping `
  -platform=Win64 `
  -serverplatform=Win64+Linux `
  -nocompileeditor `
  -installed
```

### Method 2: UAT Command Line (Linux)

```bash
# Set variables
ENGINE_PATH="/path/to/UnrealEngine"
PROJECT_PATH="/path/to/FactoryGame.uproject"

# Package CustomWebSocket
$ENGINE_PATH/Engine/Build/BatchFiles/RunUAT.sh \
  -ScriptsForProject="$PROJECT_PATH" \
  PackagePlugin \
  -Project="$PROJECT_PATH" \
  -dlcname=CustomWebSocket \
  -merge -build -server \
  -clientconfig=Shipping \
  -serverconfig=Shipping \
  -platform=Linux \
  -serverplatform=Linux \
  -nocompileeditor \
  -installed

# Package DiscordBot
$ENGINE_PATH/Engine/Build/BatchFiles/RunUAT.sh \
  -ScriptsForProject="$PROJECT_PATH" \
  PackagePlugin \
  -Project="$PROJECT_PATH" \
  -dlcname=DiscordBot \
  -merge -build -server \
  -clientconfig=Shipping \
  -serverconfig=Shipping \
  -platform=Linux \
  -serverplatform=Linux \
  -nocompileeditor \
  -installed
```

### Method 3: CI/CD Workflow

The repository includes a GitHub Actions workflow that automatically builds and packages all plugins:

```bash
# See: .github/workflows/build.yml

# The CI workflow:
# 1. Sets up Unreal Engine 5.3.2-CSS
# 2. Installs Wwise plugin
# 3. Builds CustomWebSocket plugin
# 4. Builds SML mod
# 5. Builds DiscordBot mod
# 6. Packages all for Win64 and Linux
# 7. Uploads artifacts

# To trigger manually:
# 1. Go to GitHub repository
# 2. Click "Actions" tab
# 3. Select "Build" workflow
# 4. Click "Run workflow"
```

---

## Getting Help

### Check Existing Documentation

Before asking for help, review these resources:

1. **ALPAKIT_COMPILATION_FIX.md** - Original fix documentation
2. **BUILD_GUIDE.md** - Complete build instructions
3. **PLUGIN_COMPILATION_FIX.md** - Plugin registration details
4. **Mods/DiscordBot/README.md** - Discord bot setup
5. **Plugins/CustomWebSocket/README.md** - WebSocket details

### Run Verification Scripts

```bash
# Verify Discord bot integration
./verify_discordbot_integration.sh

# Verify WebSocket functionality
./verify_websocket.sh

# Verify configuration files
./verify_config_persistence.sh

# Verify server configuration
./verify_server_config.sh
```

### Collect Debug Information

When reporting issues, include:

```bash
# 1. System information
uname -a  # Linux
# or
systeminfo  # Windows

# 2. Unreal Engine version
cat ~/.config/Epic/UnrealEngine/Version.txt  # Linux
# or check installation directory on Windows

# 3. Plugin list
grep -A 2 '"Name":' FactoryGame.uproject | grep -E '"Name"|"Enabled"'

# 4. Build output
# Copy full output from compilation attempt

# 5. Alpakit configuration
cat Mods/DiscordBot/Config/Alpakit.ini
cat Plugins/CustomWebSocket/Config/Alpakit.ini

# 6. Editor log (if using Alpakit GUI)
# Windows: %LOCALAPPDATA%\FactoryGame\Saved\Logs\FactoryGame.log
# Linux: ~/.config/FactoryGame/Saved/Logs/FactoryGame.log
```

### Where to Get Support

1. **GitHub Issues**
   - Repository: https://github.com/Yamahasxviper/SatisfactoryDiscordbot/issues
   - Include debug information and steps to reproduce

2. **Satisfactory Modding Discord**
   - Server: https://discord.gg/QzcG9nX
   - Channel: #modding-help

3. **Satisfactory Modding Documentation**
   - Docs: https://docs.ficsit.app/
   - Tutorials and guides

---

## Summary

### Quick Reference

| Problem | Solution | Doc Reference |
|---------|----------|---------------|
| Alpakit can't see plugins | Verify `Alpakit.ini` files exist | [Step 2](#step-2-verify-alpakit-configuration-files) |
| Plugin not found error | Enable plugins in `.uproject` | [Issue 2](#issue-2-plugin-sml-not-found-or-plugin-customwebsocket-not-found) |
| Module not found | Check build order and dependencies | [Issue 3](#issue-3-could-not-find-module-customwebsocket) |
| Wrong engine version | Install UE 5.3.2-CSS | [Issue 4](#issue-4-wrong-unreal-engine-version) |
| Missing Wwise | Install Wwise plugin | [Issue 5](#issue-5-missing-wwise-plugin) |
| Out of memory | Reduce parallel builds | [Issue 6](#issue-6-out-of-memory-during-compilation) |
| Permission denied | Run as admin, check permissions | [Issue 7](#issue-7-access-denied-or-permission-denied-during-packaging) |
| Config files missing from package | Update `FilterPlugin.ini` | [Issue 8](#issue-8-packaged-mod-doesnt-include-config-files) |

### Files That Must Exist

- ✅ `Mods/DiscordBot/Config/Alpakit.ini`
- ✅ `Mods/DiscordBot/DiscordBot.uplugin`
- ✅ `Plugins/CustomWebSocket/Config/Alpakit.ini`
- ✅ `Plugins/CustomWebSocket/CustomWebSocket.uplugin`
- ✅ `Mods/SML/Config/Alpakit.ini`
- ✅ `Mods/SML/SML.uplugin`
- ✅ `FactoryGame.uproject` (with plugins enabled)

### Build Order

1. **CustomWebSocket** (no dependencies)
2. **SML** (depends on CustomWebSocket)
3. **DiscordBot** (depends on SML and CustomWebSocket)

### Verification Checklist

- [ ] All required files exist
- [ ] Plugins enabled in FactoryGame.uproject
- [ ] Correct Unreal Engine version (5.3.2-CSS)
- [ ] Wwise plugin installed
- [ ] Project compiles successfully
- [ ] Alpakit shows all plugins
- [ ] Packaging produces ZIP files
- [ ] Config files included in packages

---

**All fixes have been applied to this repository. If you follow this guide and still have issues, please open a GitHub issue with full debug information.**
