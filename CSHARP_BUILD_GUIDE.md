# C# Projects Build Guide

This document explains how to build the C# UnrealBuildTool plugins and automation tools in this repository.

## Overview

This repository contains several C# projects that extend Unreal Engine's build system:

1. **FactoryGameUbtPlugin** (`Plugins/FactoryGameUbtPlugin/`) - UnrealBuildTool plugin for FactoryGame-specific build logic
2. **AccessTransformers** (`Mods/AccessTransformers/Source/AccessTransformers.ubtplugin/`) - UnrealBuildTool plugin for access transformers
3. **Alpakit.Automation** (`Mods/Alpakit/Source/Alpakit.Automation/`) - Automation scripts for packaging mods

## Prerequisites

### Required Software

- **.NET SDK 6.0 or later** - Download from https://dotnet.microsoft.com/download
- **Unreal Engine 5.3.2-CSS** - Custom Satisfactory engine version
  - Official Satisfactory modding builds available at: https://github.com/satisfactorymodding/UnrealEngine/releases
  - Look for releases tagged with `-CSS` suffix

### Environment Variables

The C# projects require the `EngineDir` environment variable to be set to point to your Unreal Engine installation.

**Windows:**
```cmd
set EngineDir=C:\Path\To\UnrealEngine-5.3.2-CSS\Engine
```

**Linux/Mac:**
```bash
export EngineDir=/path/to/UnrealEngine-5.3.2-CSS/Engine
```

Alternatively, the projects will automatically detect the engine in these locations:
- `./ue/Engine/` (relative to repository root)
- `C:\SML\ue\Engine\` (CI/CD default location)

## Building the Projects

### Option 1: Build All Projects

From the repository root:

```bash
dotnet build
```

This will automatically build all C# projects in the correct order.

### Option 2: Build Individual Projects

Build each project separately:

```bash
# Build FactoryGameUbtPlugin
cd Plugins/FactoryGameUbtPlugin/Source/FactoryGameUbtPlugin
dotnet build

# Build AccessTransformers
cd Mods/AccessTransformers/Source/AccessTransformers.ubtplugin
dotnet build

# Build Alpakit.Automation
cd Mods/Alpakit/Source/Alpakit.Automation
dotnet build
```

### Option 3: Build with Unreal Engine

When you open `FactoryGame.uproject` in Unreal Engine, the engine will automatically:
1. Detect the C# projects
2. Set the `EngineDir` variable
3. Build the projects as needed
4. Generate the Visual Studio solution

This is the **recommended** way for normal development.

## Build Output

The compiled DLLs are placed in:
- **FactoryGameUbtPlugin**: `Binaries/DotNET/UnrealBuildTool/Plugins/FactoryGameUbtPlugin/`
- **AccessTransformers**: `Mods/AccessTransformers/Binaries/DotNET/`
- **Alpakit.Automation**: `Mods/Alpakit/Binaries/DotNET/`

## Troubleshooting

### Error: "EngineDir is not set"

**Solution:** Set the `EngineDir` environment variable as described above, or ensure the engine is in one of the default locations.

### Error: "Cannot find UnrealBuildTool.dll"

**Cause:** The `EngineDir` is set incorrectly or the Unreal Engine is not properly installed.

**Solution:** 
1. Verify your Unreal Engine installation
2. Ensure `EngineDir` points to the `Engine` directory (not the root)
3. Check that `$(EngineDir)/Binaries/DotNET/UnrealBuildTool/UnrealBuildTool.dll` exists

### Error: "The imported project ... was not found"

**Cause:** The Unreal Engine is not installed or `EngineDir` is incorrect.

**Solution:** Install Unreal Engine 5.3.2-CSS and set `EngineDir` correctly.

### Projects don't appear in Visual Studio

**Cause:** The solution file needs to be regenerated.

**Solution:** Right-click `FactoryGame.uproject` and select "Generate Visual Studio project files" or run the engine's `GenerateProjectFiles.bat`.

## CI/CD Builds

The GitHub Actions workflow automatically:
1. Downloads the Unreal Engine
2. Extracts it to `C:\SML\ue\`
3. Builds the C# projects using the detected engine
4. Builds the Unreal project

The `Directory.Build.props` and individual `.csproj.props` files ensure the engine is found automatically in the CI environment.

## Project Structure

```
SatisfactoryDiscordbot/
├── Directory.Build.props          # Common build properties for all C# projects
├── FactoryGame.uproject           # Main Unreal Engine project file
├── Plugins/
│   └── FactoryGameUbtPlugin/
│       └── Source/FactoryGameUbtPlugin/
│           ├── FactoryGameUbtPlugin.ubtplugin.csproj
│           └── FactoryGameUbtPlugin.ubtplugin.csproj.props
└── Mods/
    ├── AccessTransformers/
    │   └── Source/AccessTransformers.ubtplugin/
    │       ├── AccessTransformers.ubtplugin.csproj
    │       └── AccessTransformers.ubtplugin.csproj.props
    └── Alpakit/
        └── Source/Alpakit.Automation/
            ├── Alpakit.Automation.csproj
            └── Alpakit.Automation.csproj.props
```

## Configuration Files

### Directory.Build.props

This file at the repository root is automatically imported by all C# projects. It provides:
- Default `EngineDir` detection
- Common build properties
- Configuration-specific settings

### Individual .csproj.props files

Each C# project has its own `.csproj.props` file that:
- Sets project-specific `EngineDir` fallbacks
- Can be customized for local development
- Is git-tracked to ensure consistent builds

## Development Workflow

1. **Initial Setup:**
   - Install Unreal Engine 5.3.2-CSS
   - Set `EngineDir` environment variable (optional)
   - Open `FactoryGame.uproject` in Unreal Engine

2. **Making Changes:**
   - Edit C# files in your IDE
   - The projects will be rebuilt automatically by Unreal Engine

3. **Manual Building:**
   - Use `dotnet build` if you want to build without opening Unreal Engine
   - Useful for quick syntax checking

## Additional Resources

- [Satisfactory Modding Documentation](https://docs.ficsit.app/)
- [UnrealBuildTool Documentation](https://docs.unrealengine.com/en-US/ProductionPipelines/BuildTools/UnrealBuildTool/)
- [.NET SDK Documentation](https://docs.microsoft.com/en-us/dotnet/)

## Support

If you encounter issues:
1. Check the [Satisfactory Modding Discord](https://discord.gg/QzcG9nX)
2. Review the [GitHub Issues](https://github.com/Yamahasxviper/SatisfactoryDiscordbot/issues)
3. Ensure you have the correct Unreal Engine version installed
