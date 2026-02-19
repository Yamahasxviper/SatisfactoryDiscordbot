# SatisfactoryModLoader + DiscordBot [![CI](https://github.com/satisfactorymodding/SatisfactoryModLoader/actions/workflows/build.yml/badge.svg)](https://github.com/satisfactorymodding/SatisfactoryModLoader/actions/workflows/build.yml)

A tool used to load mods for the game Satisfactory. After Coffee Stain releases a proper Unreal modding API the project will continue as a utilities library.

This repository contains:
- **SatisfactoryModLoader (SML)** - Mod loading and compatibility API
- **DiscordBot Mod** - Discord integration with two-way chat, presence updates, and server status
- **CustomWebSocket Plugin** - Platform-agnostic WebSocket implementation
- **ExampleMod** - Demos of SML utilities
- **Editor utilities** - Collection of tools for mod development

It also serves as the Unreal project used for developing mods.

## Quick Start

### Building the Project

See [BUILD_GUIDE.md](BUILD_GUIDE.md) for detailed build instructions.

**⚠️ Having Alpakit Issues?** 
- **Quick Fix:** [ALPAKIT_QUICK_REFERENCE.md](ALPAKIT_QUICK_REFERENCE.md) (5-second check + common solutions)
- **Complete Guide:** [ALPAKIT_TROUBLESHOOTING_GUIDE.md](ALPAKIT_TROUBLESHOOTING_GUIDE.md) (detailed troubleshooting)

**Note:** As of Feb 2026:
- SML and DiscordBot plugins are now properly registered in `FactoryGame.uproject` and will compile automatically. See [PLUGIN_COMPILATION_FIX.md](PLUGIN_COMPILATION_FIX.md) for details.
- CustomWebSocket and DiscordBot are now properly packaged by the CI workflow. See [PACKAGING_FIX.md](PACKAGING_FIX.md) for details.
- Alpakit configuration files have been added for DiscordBot and CustomWebSocket. See [ALPAKIT_COMPILATION_FIX.md](ALPAKIT_COMPILATION_FIX.md) for details.

### Discord Bot Setup

The DiscordBot mod provides Discord integration for Satisfactory servers:
- Two-way chat between Discord and in-game
- Server status updates
- Player join/leave notifications
- Presence updates

See [Mods/DiscordBot/README.md](Mods/DiscordBot/README.md) for setup instructions.

## Documentation

Learn how to set up and use this repo on the [modding documentation](https://docs.ficsit.app/).

## Discord Server

Join our [discord server](https://discord.gg/QzcG9nX) to talk about SML and Satisfactory Modding in general.

## DISCLAIMER

This software is provided by the author "as is". In no event shall the author be liable for any direct, indirect, incidental, special, exemplary, or consequential damages (including, but not limited to procurement of substitute goods or services; loss of use, data, or profits; or business interruption) however caused and on any
theory of liability, whether in contract, strict liability, or tort (including negligence or otherwise) arising in any way out of the use of this software, even if advised of the possibility of such damage.
