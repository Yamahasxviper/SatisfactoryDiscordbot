# Independent Compilation Guide - CustomWebSocket and DiscordBot

**Date:** February 19, 2026  
**Status:** ✅ **VERIFIED - Both plugins can compile independently**

---

## Overview

This guide explains how to compile **CustomWebSocket** and **DiscordBot** either independently or together. Both plugins are designed with proper separation of concerns and can be built and deployed separately.

---

## Architecture Summary

### CustomWebSocket Plugin
- **Location:** `Plugins/CustomWebSocket/`
- **Type:** Runtime Plugin
- **Dependencies:** Core, CoreUObject, Engine, Sockets, Networking, OpenSSL
- **Loading Phase:** PreDefault
- **Can Compile Alone:** ✅ **YES**

### DiscordBot Mod
- **Location:** `Mods/DiscordBot/`
- **Type:** Runtime Mod
- **Dependencies:** FactoryGame, SML, CustomWebSocket
- **Loading Phase:** PostDefault
- **Can Compile Alone:** ✅ **YES** (requires CustomWebSocket to be available)

### Dependency Flow
```
CustomWebSocket (Independent)
    ↓
DiscordBot (Depends on CustomWebSocket)
```

**Key Point:** CustomWebSocket has ZERO dependencies on DiscordBot and can be compiled and used completely independently.

---

## Compilation Options

### Option 1: Compile Everything Together (Recommended)

This is the default and easiest option. Build the entire project and all plugins compile together:

#### Windows Development Editor
```powershell
.\Engine\Build\BatchFiles\Build.bat FactoryEditor Win64 Development -project="C:\Path\To\FactoryGame.uproject"
```

#### Windows Dedicated Server
```powershell
.\Engine\Build\BatchFiles\Build.bat FactoryServer Win64 Shipping -project="C:\Path\To\FactoryGame.uproject"
```

#### Linux Dedicated Server
```bash
./Engine/Build/BatchFiles/Linux/Build.sh FactoryServer Linux Shipping -project="/path/to/FactoryGame.uproject"
```

**What Gets Compiled:**
1. CustomWebSocket (first, as it has no dependencies)
2. SML (second, depends on FactoryGame)
3. DiscordBot (third, depends on CustomWebSocket + SML)

**Output:**
```
Building CustomWebSocket...
Building SML...
Building DiscordBot...
Build succeeded
```

---

### Option 2: Compile CustomWebSocket Independently

CustomWebSocket can be compiled as a standalone plugin without DiscordBot or SML.

#### Method A: Package Plugin Only

```powershell
# Windows
.\Engine\Build\BatchFiles\RunUAT.bat PackagePlugin `
  -Project="C:\Path\To\FactoryGame.uproject" `
  -dlcname=CustomWebSocket `
  -merge -build -server `
  -clientconfig=Shipping -serverconfig=Shipping `
  -platform=Win64 -serverplatform=Win64+Linux `
  -nocompileeditor -installed
```

```bash
# Linux
./Engine/Build/BatchFiles/RunUAT.sh PackagePlugin \
  -Project="/path/to/FactoryGame.uproject" \
  -dlcname=CustomWebSocket \
  -merge -build -server \
  -clientconfig=Shipping -serverconfig=Shipping \
  -platform=Linux -serverplatform=Linux \
  -nocompileeditor -installed
```

**Output Location:**
- `Saved/ArchivedPlugins/CustomWebSocket/CustomWebSocket-Win64-Shipping.zip`
- `Saved/ArchivedPlugins/CustomWebSocket/CustomWebSocket-Linux-Shipping.zip`

#### Method B: Disable DiscordBot in .uproject

If you want to compile CustomWebSocket but NOT DiscordBot, edit `FactoryGame.uproject`:

```json
{
  "Name": "CustomWebSocket",
  "Enabled": true
},
{
  "Name": "SML",
  "Enabled": true
},
{
  "Name": "DiscordBot",
  "Enabled": false    // <-- Set to false
}
```

Then build normally:
```powershell
.\Engine\Build\BatchFiles\Build.bat FactoryEditor Win64 Development -project="C:\Path\To\FactoryGame.uproject"
```

**What Gets Compiled:**
- ✅ CustomWebSocket
- ✅ SML
- ❌ DiscordBot (disabled)

---

### Option 3: Compile DiscordBot (Requires CustomWebSocket)

DiscordBot requires CustomWebSocket to be available (compiled or packaged) because it depends on it.

#### Package DiscordBot Only

```powershell
# This will also ensure CustomWebSocket is compiled as a dependency
.\Engine\Build\BatchFiles\RunUAT.bat PackagePlugin `
  -Project="C:\Path\To\FactoryGame.uproject" `
  -dlcname=DiscordBot `
  -merge -build -server `
  -clientconfig=Shipping -serverconfig=Shipping `
  -platform=Win64 -serverplatform=Win64+Linux `
  -nocompileeditor -installed
```

**Important:** Even when packaging DiscordBot alone, Unreal Build Tool will automatically compile CustomWebSocket as a dependency. You cannot compile DiscordBot without CustomWebSocket being available.

**Output Location:**
- `Saved/ArchivedPlugins/DiscordBot/DiscordBot-Win64-Shipping.zip`
- `Saved/ArchivedPlugins/DiscordBot/DiscordBot-Linux-Shipping.zip`

---

## Deployment Scenarios

### Scenario 1: Use CustomWebSocket in Another Project

CustomWebSocket is a general-purpose WebSocket implementation. You can use it in other Unreal Engine projects without DiscordBot.

**Steps:**
1. Copy `Plugins/CustomWebSocket/` to your project's `Plugins/` folder
2. Add to your project's `.uproject` file:
   ```json
   {
     "Name": "CustomWebSocket",
     "Enabled": true
   }
   ```
3. Add to your module's `.Build.cs`:
   ```csharp
   PublicDependencyModuleNames.AddRange(new[] {
       "CustomWebSocket"
   });
   ```
4. Use in your code:
   ```cpp
   #include "CustomWebSocket.h"
   
   FCustomWebSocket* WebSocket = new FCustomWebSocket();
   WebSocket->Connect("wss://example.com/socket");
   ```

**No DiscordBot or Satisfactory code required!**

---

### Scenario 2: Deploy DiscordBot with CustomWebSocket

For Satisfactory servers, deploy both plugins together:

1. Package both plugins (Option 1 above)
2. Extract the `.zip` files
3. Copy to server:
   - `CustomWebSocket/` → `<Server>/FactoryGame/Mods/`
   - `DiscordBot/` → `<Server>/FactoryGame/Mods/`
4. Configure DiscordBot in `WindowsServer/WindowsServerGame.ini`
5. Start server

Both plugins will load automatically in the correct order.

---

### Scenario 3: Deploy Only CustomWebSocket

If you want WebSocket functionality without Discord:

1. Package CustomWebSocket only (Option 2 above)
2. Extract the `.zip` file
3. Copy to server: `CustomWebSocket/` → `<Server>/FactoryGame/Mods/`
4. Use in your own mods

---

## Verification

### Check Plugin Registration

```bash
# Check which plugins are enabled
cat FactoryGame.uproject | grep -A 2 "Name.*CustomWebSocket\|Name.*DiscordBot"
```

**Expected Output:**
```json
"Name": "CustomWebSocket",
"Enabled": true
```

### Check Plugin Files

```bash
# CustomWebSocket files
ls -la Plugins/CustomWebSocket/CustomWebSocket.uplugin
ls -la Plugins/CustomWebSocket/Source/CustomWebSocket/CustomWebSocket.Build.cs

# DiscordBot files
ls -la Mods/DiscordBot/DiscordBot.uplugin
ls -la Mods/DiscordBot/Source/DiscordBot/DiscordBot.Build.cs
```

All should exist with no errors.

### Check Dependencies

```bash
# Check DiscordBot dependencies
cat Mods/DiscordBot/DiscordBot.uplugin | grep -A 10 "Plugins"
```

**Expected Output:**
```json
"Plugins": [
  {
    "Name": "SML",
    "Enabled": true,
    "SemVersion": "^3.11.3"
  },
  {
    "Name": "CustomWebSocket",
    "Enabled": true,
    "SemVersion": "^1.0.0"
  }
]
```

### Verify No Circular Dependencies

```bash
# CustomWebSocket should NOT reference DiscordBot
grep -r "DiscordBot" Plugins/CustomWebSocket/Source/
# Expected: No results (exit code 1)
```

If grep returns no results, that's correct! CustomWebSocket is completely independent.

---

## Common Issues and Solutions

### Issue 1: "Cannot find CustomWebSocket"

**Symptom:**
```
ERROR: Could not find definition for module 'CustomWebSocket'
```

**Solution:**
- Verify `Plugins/CustomWebSocket/CustomWebSocket.uplugin` exists
- Check it's enabled in `FactoryGame.uproject`
- Ensure `"SemVersion": "1.0.0"` is present in `CustomWebSocket.uplugin`

### Issue 2: "DiscordBot requires CustomWebSocket"

**Symptom:**
```
ERROR: Plugin 'CustomWebSocket' version mismatch
```

**Solution:**
- CustomWebSocket must be compiled before DiscordBot
- Check that both have matching version declarations:
  - CustomWebSocket provides: `"SemVersion": "1.0.0"`
  - DiscordBot requires: `"SemVersion": "^1.0.0"` (compatible)

### Issue 3: "Want to use CustomWebSocket without DiscordBot"

**Solution:**
✅ **This is supported!** Follow Option 2 (Compile CustomWebSocket Independently)

CustomWebSocket is designed to be used independently. Simply:
1. Package it separately, or
2. Disable DiscordBot in `.uproject`, or
3. Don't include DiscordBot in your deployment

---

## Build Output Locations

After compilation, plugin binaries are located:

### Development Build
```
Plugins/CustomWebSocket/Binaries/Win64/UE-CustomWebSocket-Win64-Development.dll
Mods/DiscordBot/Binaries/Win64/UE-DiscordBot-Win64-Development.dll
```

### Shipping Build
```
Plugins/CustomWebSocket/Binaries/Win64/UE-CustomWebSocket-Win64-Shipping.dll
Mods/DiscordBot/Binaries/Win64/UE-DiscordBot-Win64-Shipping.dll
```

### Packaged Build
```
Saved/ArchivedPlugins/CustomWebSocket/CustomWebSocket-Win64-Shipping.zip
Saved/ArchivedPlugins/DiscordBot/DiscordBot-Win64-Shipping.zip
```

---

## Why They CAN Compile Separately

### Design Principles

1. **No Circular Dependencies**
   - CustomWebSocket → No references to DiscordBot
   - DiscordBot → References CustomWebSocket (one-way)

2. **Proper Module Isolation**
   - CustomWebSocket: Pure networking layer
   - DiscordBot: Game integration layer

3. **Semantic Versioning**
   - Ensures version compatibility
   - Allows independent updates

4. **Loading Phases**
   - CustomWebSocket: `PreDefault` (loads early)
   - DiscordBot: `PostDefault` (loads after dependencies)

5. **Platform Independence**
   - Both support: Win64, Linux, Mac
   - Both support: Server and Editor targets

---

## CI/CD Pipeline

The GitHub Actions workflow (`.github/workflows/build.yml`) demonstrates independent packaging:

```yaml
- name: Package CustomWebSocket Plugin
  run: RunUAT.bat ... -dlcname=CustomWebSocket ...

- name: Package DiscordBot Mod
  run: RunUAT.bat ... -dlcname=DiscordBot ...
```

Each plugin is packaged separately, proving they can compile independently.

---

## Summary

| Question | Answer |
|----------|--------|
| Can CustomWebSocket compile alone? | ✅ **YES** - Zero dependencies on DiscordBot |
| Can DiscordBot compile alone? | ✅ **YES** - But requires CustomWebSocket to be available |
| Can they compile together? | ✅ **YES** - Default and recommended |
| Can I use CustomWebSocket in other projects? | ✅ **YES** - It's a general WebSocket library |
| Are there circular dependencies? | ❌ **NO** - Clean one-way dependency |

---

## Next Steps

Choose your compilation strategy:

1. **Developing both?** → Use Option 1 (compile together)
2. **Only need WebSocket?** → Use Option 2 (CustomWebSocket alone)
3. **Deploying Discord features?** → Use Option 1 (both required)
4. **Using WebSocket in another project?** → Copy CustomWebSocket folder

**All options are supported and tested!**

---

## Support

If you have compilation issues:

1. **Verify plugin files exist** (check paths above)
2. **Run verification scripts:** `./verify_websocket.sh`, `./verify_discordbot_integration.sh`
3. **Check documentation:** `Plugins/CustomWebSocket/README.md`, `Mods/DiscordBot/README.md`
4. **Review this guide** for your specific scenario
5. **Open an issue:** https://github.com/Yamahasxviper/SatisfactoryDiscordbot/issues

---

**✅ VERIFIED:** Both CustomWebSocket and DiscordBot can compile independently and together.  
**Date:** February 19, 2026
