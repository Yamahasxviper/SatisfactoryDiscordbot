# SatisfactoryModLoader [![CI](https://github.com/satisfactorymodding/SatisfactoryModLoader/actions/workflows/build.yml/badge.svg)](https://github.com/satisfactorymodding/SatisfactoryModLoader/actions/workflows/build.yml)

A tool used to load mods for the game Satisfactory. After Coffee Stain releases a proper Unreal modding API the project will continue as a utilities library.

This repository contains the SatisfactoryModLoader source code,
an ExampleMod with demos of some of the SML utilities,
a collection of editor utilities,
and more.
It also serves as the Unreal project used for developing mods.

## Documentation

Learn how to set up and use this repo on the [modding documentation](https://docs.ficsit.app/).

### Build Requirements

For information about building this project, including engine requirements and plugin dependencies, see [BUILD_REQUIREMENTS.md](BUILD_REQUIREMENTS.md).

### Troubleshooting

Having issues? Check the [TROUBLESHOOTING.md](TROUBLESHOOTING.md) guide for quick solutions to common problems.

**Common build errors:**
- "Unable to find plugin 'WebSockets'" → See [BUILD_REQUIREMENTS.md](BUILD_REQUIREMENTS.md) or [TROUBLESHOOTING.md](TROUBLESHOOTING.md)

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

## Discord Chat Bridge Mod

This repository includes the **Discord Chat Bridge** mod, which provides two-way chat integration between Satisfactory in-game chat and Discord.

> **✅ NO CLIENT INSTALLATION REQUIRED**  
> Players connecting to your server **do not need to install anything**. This mod runs entirely on the server side. Only the server administrator needs to install and configure the mod.
> 
> **Technical Implementation:** The mod uses `"RequiredOnRemote": false` in its plugin descriptor and `SpawnOnServer` replication policy to ensure it runs only on the server and allows vanilla clients to connect.

### SML Compatibility

**✅ FULLY COMPATIBLE** - This Discord mod works perfectly with SML 3.11.3+. See [SML_COMPATIBILITY.md](SML_COMPATIBILITY.md) for detailed compatibility information.

### Features
- Two-way chat synchronization between Satisfactory and Discord
- Uses Discord Bot Token (no webhooks required)
- Configurable through INI file
- **Server-side only** - Players join and chat normally, no mod installation needed

### Quick Links
- [SML Compatibility Status](SML_COMPATIBILITY.md) - **Will my Discord project work with SML?**
- [Mod README](Mods/DiscordChatBridge/README.md)
- [Quick Start Guide](Mods/DiscordChatBridge/help/QUICKSTART.md) - Get started in 5 minutes
- [Setup Guide](Mods/DiscordChatBridge/help/SETUP_GUIDE.md) - Detailed instructions
- [Configuration Guide](Mods/DiscordChatBridge/config/README.md) - Recommended config method

## Discord Server

Join our [discord server](https://discord.gg/QzcG9nX) to talk about SML and Satisfactory Modding in general.

## DISCLAIMER

This software is provided by the author "as is". In no event shall the author be liable for any direct, indirect, incidental, special, exemplary, or consequential damages (including, but not limited to procurement of substitute goods or services; loss of use, data, or profits; or business interruption) however caused and on any
theory of liability, whether in contract, strict liability, or tort (including negligence or otherwise) arising in any way out of the use of this software, even if advised of the possibility of such damage.
