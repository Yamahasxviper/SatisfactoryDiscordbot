# Wwise Setup and Troubleshooting Guide

This guide explains how to work with the Wwise audio middleware plugin in the Satisfactory Mod Loader project.

## Understanding Wwise in This Project

### What is Wwise?

Wwise (Wave Works Interactive Sound Engine) is an audio middleware solution used by the game Satisfactory for audio management and playback. This project requires the Wwise Unreal Engine plugin to build successfully.

### Why Isn't Wwise in the Repository?

The Wwise plugin is **not included in this repository** for several reasons:
- **Size**: The Wwise plugin is very large (hundreds of MB)
- **Licensing**: Wwise has specific distribution requirements
- **Versioning**: Coffee Stain Studios uses a specific version that must match exactly

Instead, the Wwise plugin is:
- Downloaded separately during CI builds from a B2 bucket (see `.github/workflows/build.yml`)
- Excluded from version control (see `.gitignore` lines 17-18)
- Applied with patches automatically during the build process

## Common Issue: "Wwise Launcher won't open my project"

### The Problem

If you're seeing this error, it typically means one of these issues:

1. **Wwise plugin is not downloaded**: The `Plugins/Wwise/` directory doesn't exist or is empty
2. **No Wwise project file exists**: There is no `.wproj` file in this repository because this is a mod development project, not the main Satisfactory game project
3. **Platform mismatch**: You're trying to build on Linux/Mac but patches weren't applied

### The Solution

#### For Building the Project (CI/Automated Builds)

The CI workflow automatically:
1. Downloads the Wwise plugin from the B2 bucket
2. Extracts it to `Plugins/Wwise/`
3. Runs pre-build steps to apply Wwise patches
4. Builds the project successfully

**You don't need the Wwise Launcher or a Wwise project file for building this project.**

#### For Local Development

If you're building locally and encounter Wwise-related errors:

##### Option 1: Download Wwise Plugin (Recommended for Full Builds)

1. **Get access to the Wwise plugin**:
   - Contact the repository maintainers for access to the Wwise B2 bucket
   - Or obtain the Wwise plugin matching the version used by Coffee Stain Studios (check `.github/workflows/build.yml` for details)

2. **Extract the plugin**:
   ```bash
   # Place the Wwise plugin in the correct location
   # The plugin should be extracted to: Plugins/Wwise/
   ```

3. **Build the project**:
   - The pre-build steps will automatically apply the necessary patches
   - **Windows**: The PowerShell script `Mods/WwisePatches/applyPatches.ps1` runs automatically
   - **Linux**: The Bash script `Mods/WwisePatches/applyPatches.sh` runs automatically

##### Option 2: Build Without Wwise (Development Only)

If you don't need audio features and just want to compile code:

1. **Disable the Wwise plugin** in `FactoryGame.uproject`:
   ```json
   {
       "Name": "Wwise",
       "Enabled": false
   }
   ```

2. **Note**: This will prevent audio features from working, but allows code compilation and testing of non-audio functionality.

## Wwise Patches

This project includes several patches that are automatically applied to the Wwise plugin during build:

### Available Patches

Located in `Mods/WwisePatches/Patches/`:

1. **01-ignore_missing_soundbanks.patch**
   - Changes error logs to warnings when soundbanks are missing
   - Allows the project to build even without complete audio assets

2. **02-fix_plugin_soundbanks_location.patch**
   - Fixes the path resolution for soundbank files
   - Ensures soundbanks are found in the correct Content directory

3. **03-CSS-CL404250.patch**
   - Coffee Stain Studios specific patch for compatibility

4. **04-CSS-CL404250_link_fixes.patch**
   - Additional fixes for the CSS patch

### How Patches Are Applied

#### Windows (Automatic)

Pre-build step in `FactoryGame.uproject` runs:
```powershell
powershell -Command "Set-ExecutionPolicy Bypass -Scope Process -Force;Unblock-File '$(ProjectDir)\Mods\WwisePatches\applyPatches.ps1';& '$(ProjectDir)\Mods\WwisePatches\applyPatches.ps1' '$(ProjectDir)'"
```

#### Linux (Automatic)

Pre-build step in `FactoryGame.uproject` runs:
```bash
bash "$(ProjectDir)/Mods/WwisePatches/applyPatches.sh" "$(ProjectDir)"
```

#### Manual Application (If Needed)

If automatic patching fails, you can apply patches manually:

**Windows:**
```powershell
cd Mods\WwisePatches
.\applyPatches.ps1 "C:\Path\To\SatisfactoryModLoader"
```

**Linux/Mac:**
```bash
cd Mods/WwisePatches
./applyPatches.sh "/path/to/SatisfactoryModLoader"
```

### Patch Tracking

Patches that have been applied are tracked in:
```
Plugins/Wwise/applied_patches.txt
```

This prevents patches from being applied multiple times. If you need to re-apply patches:
1. Delete `Plugins/Wwise/applied_patches.txt`
2. Delete the `Plugins/Wwise/` directory
3. Re-extract the Wwise plugin
4. Rebuild the project (patches will be applied automatically)

## Troubleshooting

### "Cannot find Wwise plugin"

**Cause**: The `Plugins/Wwise/` directory is missing or empty.

**Solution**:
- For CI builds: Ensure B2 credentials are configured in GitHub secrets
- For local builds: Download and extract the Wwise plugin to `Plugins/Wwise/`

### "Wwise Launcher cannot open project"

**Cause**: You're trying to open a Wwise project file that doesn't exist.

**Solution**: This repository is for mod development, not for creating Wwise audio projects. You don't need the Wwise Launcher to build mods. If you want to work with Wwise audio:
- The main Satisfactory game has the Wwise project
- Mods use existing Wwise assets from the base game
- Audio modding requires the Wwise project from Coffee Stain Studios

### "Patch application failed"

**Cause**: Patches have already been applied, or the Wwise plugin version doesn't match.

**Solution**:
1. Check `Plugins/Wwise/applied_patches.txt` to see what's already applied
2. Delete the Wwise plugin directory and re-extract a clean version
3. Ensure you're using the correct Wwise plugin version

### "patch: command not found" (Linux)

**Cause**: The `patch` utility is not installed.

**Solution**:
```bash
# Ubuntu/Debian
sudo apt-get install patch

# macOS
# patch is pre-installed, but if missing:
brew install gpatch
```

## Platform-Specific Notes

### Windows
- Uses PowerShell script with `ptch.exe` (included in `Mods/WwisePatches/ThirdParty/`)
- Pre-build steps run automatically when building through Visual Studio or UE Editor
- Requires PowerShell execution policy bypass for script execution

### Linux
- Uses Bash script with standard `patch` command
- Pre-build steps run automatically when building through UE Editor
- Requires `patch` utility to be installed
- Make sure the script is executable: `chmod +x Mods/WwisePatches/applyPatches.sh`

### macOS
- Same as Linux, uses Bash script
- `patch` command should be pre-installed
- Pre-build steps run automatically

## Build Process Overview

When you build the project:

1. **Pre-Build Phase**:
   - Platform-specific script runs (PowerShell on Windows, Bash on Linux/Mac)
   - Script checks `Plugins/Wwise/applied_patches.txt` for already-applied patches
   - Script applies any missing patches in alphabetical order
   - Each successfully applied patch is logged to prevent re-application

2. **Build Phase**:
   - Unreal Build Tool compiles the project
   - Wwise plugin modules are compiled
   - Mods that use Wwise features are linked against the plugin

3. **Post-Build Phase**:
   - Mods are packaged
   - Wwise soundbanks are included in the packaged output (if present)

## FAQ

### Do I need a Wwise account to build this project?

No. The Wwise plugin is provided separately, and you don't need to install Wwise Launcher or create a Wwise account.

### Do I need Wwise Launcher?

No. Wwise Launcher is only needed if you're creating or modifying Wwise audio projects. For mod development, you only need the Wwise Unreal plugin.

### Can I create custom audio for my mod?

Yes, but it's complex:
- You need the original Satisfactory Wwise project from Coffee Stain Studios
- You need Wwise Launcher to edit the project
- You need to generate new soundbanks
- Custom audio modding is an advanced topic beyond the scope of this guide

### What version of Wwise is used?

Check the CI workflow (`.github/workflows/build.yml`) or ask the repository maintainers. The version must exactly match the one used by Coffee Stain Studios for Satisfactory.

### Why can't I just disable Wwise?

The base game uses Wwise extensively. While you can disable it for development, any functionality that relies on audio will not work. For production builds, Wwise must be enabled.

## Additional Resources

- [Wwise Official Documentation](https://www.audiokinetic.com/library/)
- [Unreal Engine Wwise Integration](https://www.audiokinetic.com/library/edge/?source=UE4&id=index.html)
- [BUILD_REQUIREMENTS.md](../../BUILD_REQUIREMENTS.md) - General build requirements
- [TROUBLESHOOTING.md](../../TROUBLESHOOTING.md) - General troubleshooting guide
- [Satisfactory Modding Documentation](https://docs.ficsit.app/) - Modding community docs

## Getting Help

If you're still having issues:

1. Check the [TROUBLESHOOTING.md](../../TROUBLESHOOTING.md) guide
2. Review the CI workflow in `.github/workflows/build.yml` to see how automated builds work
3. Join the [Satisfactory Modding Discord](https://discord.gg/QzcG9nX)
4. Open an issue on GitHub with:
   - Your operating system
   - Build error messages
   - Steps you've tried
   - Whether you're building locally or in CI
