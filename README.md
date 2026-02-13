# SatisfactoryModLoader [![CI](https://github.com/satisfactorymodding/SatisfactoryModLoader/actions/workflows/build.yml/badge.svg)](https://github.com/satisfactorymodding/SatisfactoryModLoader/actions/workflows/build.yml)

A tool used to load mods for the game Satisfactory. After Coffee Stain releases a proper Unreal modding API the project will continue as a utilities library.

This repository contains the SatisfactoryModLoader source code,
an ExampleMod with demos of some of the SML utilities,
a collection of editor utilities,
and more.
It also serves as the Unreal project used for developing mods.

## Documentation

Learn how to set up and use this repo on the [modding documentation](https://docs.ficsit.app/).

## Building Locally

To build this project locally in Visual Studio or with MSBuild, you need:

1. **Unreal Engine 5.3.2-CSS** - The custom Satisfactory version of Unreal Engine
2. **Wwise Plugin** - Required audio middleware plugin

### Common Build Errors

**Error MSB3073: Build.bat exited with code 6**

This error typically occurs when the Wwise plugin is not properly installed. The project's PreBuildSteps require the Wwise plugin to be present in `Plugins\Wwise\` directory before building.

**Solutions:**
- The Wwise plugin is not publicly available and must be obtained through the project's CI process or from the Satisfactory modding community
- Refer to the [modding documentation](https://docs.ficsit.app/) for detailed setup instructions
- Ensure all repository files are properly cloned (the build script requires `Mods/WwisePatches/` files)

For CI builds, the Wwise plugin is automatically downloaded by the GitHub Actions workflow.

## Discord Chat Bridge Mod

This repository includes the **Discord Chat Bridge** mod, which provides two-way chat integration between Satisfactory in-game chat and Discord.

### Features
- Two-way chat synchronization between Satisfactory and Discord
- Uses Discord Bot Token (no webhooks required)
- Configurable through INI file
- Server-side only (no client installation required)

### Quick Links
- [Mod README](Mods/DiscordChatBridge/README.md)
- [Setup Guide](Mods/DiscordChatBridge/SETUP_GUIDE.md)
- [Configuration](Mods/DiscordChatBridge/Config/DefaultDiscordChatBridge.ini)

## Discord Server

Join our [discord server](https://discord.gg/QzcG9nX) to talk about SML and Satisfactory Modding in general.

## DISCLAIMER

This software is provided by the author "as is". In no event shall the author be liable for any direct, indirect, incidental, special, exemplary, or consequential damages (including, but not limited to procurement of substitute goods or services; loss of use, data, or profits; or business interruption) however caused and on any
theory of liability, whether in contract, strict liability, or tort (including negligence or otherwise) arising in any way out of the use of this software, even if advised of the possibility of such damage.
