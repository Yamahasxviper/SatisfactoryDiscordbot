# SatisfactoryModLoader [![CI](https://github.com/satisfactorymodding/SatisfactoryModLoader/actions/workflows/build.yml/badge.svg)](https://github.com/satisfactorymodding/SatisfactoryModLoader/actions/workflows/build.yml)

A tool used to load mods for the game Satisfactory. After Coffee Stain releases a proper Unreal modding API the project will continue as a utilities library.

This repository contains the SatisfactoryModLoader source code,
an ExampleMod with demos of some of the SML utilities,
a collection of editor utilities,
and more.
It also serves as the Unreal project used for developing mods.

## Documentation

Learn how to set up and use this repo on the [modding documentation](https://docs.ficsit.app/).

### Building C# Projects

This repository includes C# UnrealBuildTool plugins that need to be compiled:
- FactoryGameUbtPlugin
- AccessTransformers
- Alpakit.Automation

For detailed build instructions, see [CSHARP_BUILD_GUIDE.md](CSHARP_BUILD_GUIDE.md).

**Quick start:**
1. Install [.NET SDK 6.0+](https://dotnet.microsoft.com/download)
2. Install Unreal Engine 5.3.2-CSS from [satisfactorymodding/UnrealEngine](https://github.com/satisfactorymodding/UnrealEngine/releases)
3. Set `EngineDir` environment variable to your engine's `Engine` folder
4. Run `dotnet build` or open the project in Unreal Engine

## Discord Server

Join our [discord server](https://discord.gg/QzcG9nX) to talk about SML and Satisfactory Modding in general.

## DISCLAIMER

This software is provided by the author "as is". In no event shall the author be liable for any direct, indirect, incidental, special, exemplary, or consequential damages (including, but not limited to procurement of substitute goods or services; loss of use, data, or profits; or business interruption) however caused and on any
theory of liability, whether in contract, strict liability, or tort (including negligence or otherwise) arising in any way out of the use of this software, even if advised of the possibility of such damage.
