# Quick Build Guide - After Plugin Fix

## What Was Fixed

The SML and DiscordBot plugins are now properly registered in `FactoryGame.uproject` and will be compiled with the project.

## How to Build

### Prerequisites

1. **Unreal Engine 5.3.2-CSS** - Custom Satisfactory version
2. **Wwise Plugin** - Audio middleware (see CI workflow for download)
3. **Windows or Linux** build environment

### Build Commands

#### Windows - Development Editor

```powershell
# Build the editor
.\Engine\Build\BatchFiles\Build.bat FactoryEditor Win64 Development -project="C:\Path\To\FactoryGame.uproject"
```

#### Windows - Dedicated Server

```powershell
# Build the server
.\Engine\Build\BatchFiles\Build.bat FactoryServer Win64 Shipping -project="C:\Path\To\FactoryGame.uproject"
```

#### Linux - Dedicated Server

```bash
# Build the server
./Engine/Build/BatchFiles/Linux/Build.sh FactoryServer Linux Shipping -project="/path/to/FactoryGame.uproject"
```

### Package Mods

To package the plugins as mods:

```powershell
# Package SML
.\Engine\Build\BatchFiles\RunUAT.bat -ScriptsForProject="C:\Path\To\FactoryGame.uproject" PackagePlugin -Project="C:\Path\To\FactoryGame.uproject" -dlcname=SML -merge -build -server -clientconfig=Shipping -serverconfig=Shipping -platform=Win64 -serverplatform=Win64+Linux -nocompileeditor -installed

# Package DiscordBot
.\Engine\Build\BatchFiles\RunUAT.bat -ScriptsForProject="C:\Path\To\FactoryGame.uproject" PackagePlugin -Project="C:\Path\To\FactoryGame.uproject" -dlcname=DiscordBot -merge -build -server -clientconfig=Shipping -serverconfig=Shipping -platform=Win64 -serverplatform=Win64+Linux -nocompileeditor -installed
```

## What Gets Compiled

With the fix applied, the following will be compiled:

### 1. Core Game Modules
- FactoryGame
- FactoryEditor
- FactoryDedicatedServer
- FactoryDedicatedClient
- FactoryPreEarlyLoadingScreen
- DummyHeaders

### 2. Plugins (Base Game)
- CustomWebSocket - Platform-agnostic WebSocket
- AbstractInstance
- SignificanceManager
- ReplicationGraph
- ... (and many others)

### 3. Mods (NEW - Now Compiling)
- **SML** - Satisfactory Mod Loader
  - Provides mod loading and compatibility API
  - Required for DiscordBot to work
  
- **DiscordBot** - Discord Integration
  - Two-way chat (Discord ↔ Game)
  - Server status updates
  - Presence notifications
  - Player join/leave alerts

## Expected Build Output

You should see output like:

```
Building FactoryGame...
Building CustomWebSocket...
Building SML...
Building DiscordBot...
Compiling with UAT...
Build succeeded
```

## Troubleshooting

### "Plugin 'SML' not found"
- Check that `Mods/SML/SML.uplugin` exists
- Verify the plugin is enabled in FactoryGame.uproject

### "Plugin 'DiscordBot' not found"
- Check that `Mods/DiscordBot/DiscordBot.uplugin` exists
- Verify the plugin is enabled in FactoryGame.uproject

### "Could not find dependency 'CustomWebSocket'"
- Check that `Plugins/CustomWebSocket/CustomWebSocket.uplugin` exists
- Verify the plugin is enabled in FactoryGame.uproject

### "Missing module SML"
- Ensure SML is built before DiscordBot
- Check that SML is enabled in the project file
- Verify build order (UBT should handle this automatically)

## Verification

After building, verify the plugins are loaded:

1. **Check build logs** for "Building SML" and "Building DiscordBot"
2. **Check Binaries folder** for plugin DLLs:
   - `Mods/SML/Binaries/Win64/UE-SML-Win64-Shipping.dll`
   - `Mods/DiscordBot/Binaries/Win64/UE-DiscordBot-Win64-Shipping.dll`
   - `Plugins/CustomWebSocket/Binaries/Win64/UE-CustomWebSocket-Win64-Shipping.dll`

3. **In-game verification** (after running the server):
   - Check logs for "SML initialized"
   - Check logs for "DiscordBot initialized"
   - Check Discord bot connection status

## Next Steps

After building successfully:

1. **Configure Discord Bot** - Edit `Config/DefaultGame.ini` or `Config/WindowsServer/WindowsServerGame.ini`
   - Add your Discord bot token
   - Set channel IDs for notifications and chat
   - Configure two-way chat settings

2. **Run the Server** - The Discord bot will automatically connect if configured

3. **Test Integration**
   - Send a message in Discord → should appear in-game
   - Send a message in-game → should appear in Discord
   - Check server status updates in Discord

## Additional Resources

- `PLUGIN_COMPILATION_FIX.md` - Complete fix documentation
- `Mods/DiscordBot/README.md` - Discord bot setup guide
- `Mods/DiscordBot/QUICKSTART_NATIVE.md` - Quick start guide
- `Plugins/CustomWebSocket/README.md` - WebSocket plugin details
- `.github/workflows/build.yml` - CI build configuration

## Support

If you encounter issues:
1. Check the verification scripts: `./verify_*.sh`
2. Review the documentation in `Mods/DiscordBot/`
3. Open an issue at: https://github.com/Yamahasxviper/SatisfactoryDiscordbot/issues
