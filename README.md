# SatisfactoryModLoader [![CI](https://github.com/satisfactorymodding/SatisfactoryModLoader/actions/workflows/build.yml/badge.svg)](https://github.com/satisfactorymodding/SatisfactoryModLoader/actions/workflows/build.yml)

A tool used to load mods for the game Satisfactory. After Coffee Stain releases a proper Unreal modding API the project will continue as a utilities library.

This repository contains the SatisfactoryModLoader source code,
an ExampleMod with demos of some of the SML utilities,
a collection of editor utilities,
and more.
It also serves as the Unreal project used for developing mods.

> **🔌 Looking for WebSocket information?**  
> WebSockets are **already included** in this repository! See [Quick Reference](WEBSOCKET_QUICK_REFERENCE.md) or [Complete Guide](WEBSOCKET_HOW_TO.md).

## Documentation

Learn how to set up and use this repo on the [modding documentation](https://docs.ficsit.app/).

### Build Requirements

For information about building this project, including engine requirements and plugin dependencies, see [BUILD_REQUIREMENTS.md](BUILD_REQUIREMENTS.md).

### Troubleshooting

Having issues? Check the [TROUBLESHOOTING.md](TROUBLESHOOTING.md) guide for quick solutions to common problems.

**Common questions:**
- "Does this have WebSockets?" → ✅ **YES!** Already included. See [WEBSOCKET_HOW_TO.md](WEBSOCKET_HOW_TO.md)
- "Unable to find plugin 'WebSockets'" → See [TROUBLESHOOTING.md](TROUBLESHOOTING.md) or [WEBSOCKET_FAQ.md](WEBSOCKET_FAQ.md)

### WebSocket Plugin Status

**✅ WebSockets are INCLUDED and READY TO USE!**

The WebSocket plugin is located at `Plugins/WebSockets/` and builds automatically with the project. You don't need to add or install anything.

**Quick verification:**
```bash
./scripts/verify_websocket_build_compatibility.sh
```

**Need help?**
- 📖 [How to Use WebSockets](WEBSOCKET_HOW_TO.md) - Complete guide
- ❓ [WebSocket FAQ](WEBSOCKET_FAQ.md) - Common questions answered
- 🔧 [Build Requirements](BUILD_REQUIREMENTS.md) - Technical details

### WebSocket Server Compatibility

#### Windows Servers
WebSockets work perfectly on Windows dedicated servers! See [WEBSOCKET_WINDOWS_SERVER_COMPATIBILITY.md](WEBSOCKET_WINDOWS_SERVER_COMPATIBILITY.md) for comprehensive Windows server documentation.

**Status:** ✅ **FULLY COMPATIBLE** with Windows servers (Win64, requires Windows 8.1+/Server 2012 R2+)

#### Linux Servers
Testing WebSocket functionality for Linux servers? See [WEBSOCKET_LINUX_TESTING.md](WEBSOCKET_LINUX_TESTING.md) for comprehensive testing instructions.

**Quick test:**
```bash
./scripts/test_websocket_linux_compatibility.sh
```

**Status:** ✅ **FULLY COMPATIBLE** with Linux servers (x86_64, ARM64)

### Proxy Support

Need to run the server behind a proxy? See [PROXY_SUPPORT.md](PROXY_SUPPORT.md) for information about automatic proxy detection on Linux.

**Quick setup (Linux):**
```bash
export HTTPS_PROXY="http://proxy.example.com:8080"
./YourServerExecutable
```

## Discord Chat Bridge Mod

This repository includes the **Discord Chat Bridge** mod, which provides two-way chat integration between Satisfactory in-game chat and Discord.

> **⚠️ CONFIGURATION REQUIRED**  
> After installation, you **must configure the mod** with your Discord bot token and channel ID. The server will start normally without configuration, but Discord integration will be **inactive** until configured. See the [Quick Start Guide](Mods/DiscordChatBridge/help/QUICKSTART.md) for setup instructions.

> **✅ NO CLIENT INSTALLATION REQUIRED**  
> Players connecting to your server **do not need to install anything**. This mod runs entirely on the server side. Only the server administrator needs to install and configure the mod.
> 
> **Technical Implementation:** The mod uses `"RequiredOnRemote": false` in its plugin descriptor and `SpawnOnServer` replication policy to ensure it runs only on the server and allows vanilla clients to connect.

### Features
- Two-way chat synchronization between Satisfactory and Discord
- Uses Discord Bot Token (no webhooks required)
- Configurable through INI or TXT file formats
- **Server-side only** - Players join and chat normally, no mod installation needed

### Quick Links
- [Quick Start Guide](Mods/DiscordChatBridge/help/QUICKSTART.md) - **Start here!**
- [Mod README](Mods/DiscordChatBridge/README.md)
- [Setup Guide](Mods/DiscordChatBridge/help/SETUP_GUIDE.md)
- [Configuration Examples](Mods/DiscordChatBridge/help/EXAMPLES.md)
- [Configuration File](Mods/DiscordChatBridge/config/DiscordChatBridge.txt)
- [All Help Files](Mods/DiscordChatBridge/help/)

## Discord Server

Join our [discord server](https://discord.gg/QzcG9nX) to talk about SML and Satisfactory Modding in general.

## DISCLAIMER

This software is provided by the author "as is". In no event shall the author be liable for any direct, indirect, incidental, special, exemplary, or consequential damages (including, but not limited to procurement of substitute goods or services; loss of use, data, or profits; or business interruption) however caused and on any
theory of liability, whether in contract, strict liability, or tort (including negligence or otherwise) arising in any way out of the use of this software, even if advised of the possibility of such damage.
