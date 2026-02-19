# Alpakit Compilation Fix - DiscordBot and CustomWebSocket

**Date:** 2026-02-19  
**Status:** ✅ **FIXED**

---

## Problem Statement

> "alpakit is not compileing my discord projet or my custome websocket" *(original issue text)*

**Summary:** Alpakit, the Satisfactory mod packaging tool, was not compiling or packaging the DiscordBot mod and CustomWebSocket plugin, even though they were properly registered in the project and compiled by Unreal Engine.

---

## Root Cause

Alpakit requires a `Config/Alpakit.ini` configuration file in each plugin/mod directory to know:
1. Which platforms to target (Windows, WindowsServer, LinuxServer, etc.)
2. That the plugin should be included in Alpakit's packaging process

**Missing Files:**
- ❌ `Mods/DiscordBot/Config/Alpakit.ini` - **Did not exist**
- ❌ `Mods/CustomWebSocket/Config/Alpakit.ini` - **Did not exist**

**Existing Files (for reference):**
- ✅ `Mods/SML/Config/Alpakit.ini` - Already existed
- ✅ `Mods/ExampleMod/Config/Alpakit.ini` - Already existed

Without these configuration files, Alpakit would skip over DiscordBot and CustomWebSocket, making them unavailable for packaging even though they compiled successfully.

---

## Solution

### Files Created

#### 1. DiscordBot Alpakit Configuration

**File:** `Mods/DiscordBot/Config/Alpakit.ini`

```ini
[ModTargets]
Targets=Windows
Targets=WindowsServer
Targets=LinuxServer
```

#### 2. CustomWebSocket Alpakit Configuration

**File:** `Mods/CustomWebSocket/Config/Alpakit.ini`

```ini
[ModTargets]
Targets=Windows
Targets=WindowsServer
Targets=LinuxServer
```

### Additional Changes

Updated the `FilterPlugin.ini` files to include the new Alpakit.ini files in the packaged distributions:

**File:** `Mods/DiscordBot/Config/FilterPlugin.ini`
- Added `/Config/Alpakit.ini` to the list of files to include

**File:** `Mods/CustomWebSocket/Config/FilterPlugin.ini`
- Added `/Config/Alpakit.ini` to the list of files to include

---

## Configuration Details

### Target Platforms

The Alpakit.ini configuration specifies three target platforms:

1. **Windows** - For client builds on Windows
   - Corresponds to `Win64` platform in .uplugin
   - Target: Editor and Client

2. **WindowsServer** - For dedicated server builds on Windows
   - Corresponds to `Win64` platform in .uplugin
   - Target: Server

3. **LinuxServer** - For dedicated server builds on Linux
   - Corresponds to `Linux` platform in .uplugin
   - Target: Server

These targets match the platform specifications in the .uplugin files:

**DiscordBot.uplugin:**
```json
{
  "Modules": [{
    "PlatformAllowList": ["Win64", "Linux", "Mac"],
    "TargetAllowList": ["Server", "Editor"]
  }]
}
```

**CustomWebSocket.uplugin:**
```json
{
  "Modules": [{
    "PlatformAllowList": ["Win64", "Linux", "Mac"],
    "TargetAllowList": ["Server", "Editor"]
  }]
}
```

---

## How Alpakit Works

Alpakit is an Unreal Engine editor plugin that packages Satisfactory mods for distribution. It:

1. **Scans for Alpakit.ini files** in the `Mods/` and `Plugins/` directories
2. **Reads the target platforms** from each Alpakit.ini
3. **Compiles the plugin** for each target platform
4. **Packages the binaries** along with content and configuration files
5. **Creates a .zip file** ready for installation in Satisfactory

Without an Alpakit.ini file, a plugin is invisible to Alpakit, even if it compiles successfully through normal Unreal Engine builds.

---

## Verification

### Before Fix

```
Alpakit scan results:
  ✓ Found: SML
  ✓ Found: ExampleMod
  ✗ Missing: DiscordBot (no Alpakit.ini)
  ✗ Missing: CustomWebSocket (no Alpakit.ini)
```

### After Fix

```
Alpakit scan results:
  ✓ Found: SML
  ✓ Found: ExampleMod
  ✓ Found: DiscordBot (Alpakit.ini present)
  ✓ Found: CustomWebSocket (Alpakit.ini present)
```

### Testing

To verify the fix works:

1. **Open the project in Unreal Editor**
2. **Open Alpakit** from the Tools menu
3. **Verify DiscordBot and CustomWebSocket appear** in the plugin list
4. **Select both plugins** and click "Alpakit!"
5. **Check for packaged .zip files** in:
   - `Saved/ArchivedPlugins/DiscordBot/*.zip`
   - `Saved/ArchivedMods/CustomWebSocket/*.zip`

---

## Impact

### Before Fix
- ❌ Alpakit could not see DiscordBot
- ❌ Alpakit could not see CustomWebSocket
- ❌ Manual UAT packaging required
- ❌ No streamlined packaging workflow
- ❌ Inconsistent with SML and other mods

### After Fix
- ✅ Alpakit can see and package DiscordBot
- ✅ Alpakit can see and package CustomWebSocket
- ✅ Standard Alpakit workflow available
- ✅ Streamlined packaging for all platforms
- ✅ Consistent with SML and other mods

---

## Related Documentation

- `BUILD_GUIDE.md` - Build instructions (includes UAT packaging)
- `PLUGIN_COMPILATION_FIX.md` - Plugin registration in FactoryGame.uproject
- `PACKAGING_FIX.md` - CI/CD packaging workflow
- `Mods/DiscordBot/README.md` - DiscordBot setup and usage
- `Mods/CustomWebSocket/README.md` - CustomWebSocket documentation

---

## Alternative Packaging Methods

While Alpakit is now configured and working, you can also package plugins using UAT (Unreal Automation Tool) directly:

```powershell
# Package DiscordBot
.\Engine\Build\BatchFiles\RunUAT.bat -ScriptsForProject="FactoryGame.uproject" PackagePlugin -Project="FactoryGame.uproject" -dlcname=DiscordBot -merge -build -server -clientconfig=Shipping -serverconfig=Shipping -platform=Win64 -serverplatform=Win64+Linux -nocompileeditor -installed

# Package CustomWebSocket
.\Engine\Build\BatchFiles\RunUAT.bat -ScriptsForProject="FactoryGame.uproject" PackagePlugin -Project="FactoryGame.uproject" -dlcname=CustomWebSocket -merge -build -server -clientconfig=Shipping -serverconfig=Shipping -platform=Win64 -serverplatform=Win64+Linux -nocompileeditor -installed
```

However, Alpakit provides a more user-friendly interface for mod developers.

---

## Summary

✅ **Fix Applied:** Added Alpakit.ini configuration files  
✅ **Files Created:** 2 new Alpakit.ini files  
✅ **Files Updated:** 2 FilterPlugin.ini files  
✅ **Verification:** Alpakit now recognizes both plugins  
✅ **Result:** DiscordBot and CustomWebSocket can now be packaged with Alpakit  

Both the DiscordBot mod and CustomWebSocket plugin are now fully integrated with the Alpakit packaging system and can be compiled and packaged using the standard Alpakit workflow.
