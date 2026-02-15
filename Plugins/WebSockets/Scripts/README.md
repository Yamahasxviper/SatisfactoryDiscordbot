# WebSockets Auto-Setup Scripts

These scripts automatically download and build libwebsockets for the WebSockets plugin.

## How It Works

The WebSockets plugin will **automatically** run these scripts when:
1. You build your Unreal Engine project
2. libwebsockets is not found in the plugin's ThirdParty directory
3. libwebsockets is not found in the engine's ThirdParty directory

## Scripts

### setup-libwebsockets.sh (Linux/Mac)
- Downloads libwebsockets v4.3.2 from GitHub
- Builds it with CMake
- Installs headers and library to the plugin's ThirdParty directory
- Cleans up temporary build files

**Requirements:**
- git
- cmake
- make
- C/C++ compiler (gcc or clang)

### Setup-LibWebSockets.ps1 (Windows)
- Downloads libwebsockets v4.3.2 from GitHub
- Builds it with CMake and Visual Studio
- Installs headers and library to the plugin's ThirdParty directory
- Cleans up temporary build files

**Requirements:**
- git
- cmake
- Visual Studio 2019 or 2022 with C++ tools

## Manual Execution

You can also run these scripts manually:

### Linux/Mac:
```bash
cd Plugins/WebSockets/Scripts
./setup-libwebsockets.sh
```

### Windows:
```powershell
cd Plugins\WebSockets\Scripts
.\Setup-LibWebSockets.ps1
```

To force a rebuild:
```powershell
.\Setup-LibWebSockets.ps1 -Force
```

## What Gets Downloaded

- **libwebsockets v4.3.2** - The stable release compatible with Unreal Engine 5.3.x
- Built from source: https://github.com/warmcat/libwebsockets

## Build Configuration

The scripts build libwebsockets with these options:
- `LWS_WITH_STATIC=ON` - Static library (required by UE)
- `LWS_WITH_SSL=ON` - SSL/TLS support (for wss://)
- `LWS_WITH_ZLIB=ON` - Compression support
- `LWS_IPV6=ON` - IPv6 support
- `LWS_WITHOUT_TESTAPPS=ON` - Skip test applications

## Output

After successful execution:

**Headers:**
- `ThirdParty/libwebsockets/include/libwebsockets.h`
- `ThirdParty/libwebsockets/include/lws-*.h`

**Libraries:**
- Linux: `ThirdParty/libwebsockets/lib/Linux/x86_64-unknown-linux-gnu/libwebsockets.a`
- Windows: `ThirdParty/libwebsockets/lib/Win64/websockets_static.lib`
- macOS: `ThirdParty/libwebsockets/lib/Mac/libwebsockets.a`

## Troubleshooting

### "Command not found" errors
Install the required tools:

**Ubuntu/Debian:**
```bash
sudo apt-get install git cmake build-essential
```

**macOS:**
```bash
brew install git cmake
```

**Windows:**
- Install Git: https://git-scm.com/download/win
- Install CMake: https://cmake.org/download/
- Install Visual Studio with C++ tools

### Build fails
- Check that you have disk space (build requires ~500MB temporarily)
- Ensure you have internet connection to download from GitHub
- Check build logs for specific error messages

### Auto-setup doesn't run
The auto-setup only runs during UBT (Unreal Build Tool) execution when compiling the project. If you're just opening the editor, it won't trigger. To manually trigger:
1. Clean the project: Delete `Intermediate/` and `Saved/` folders
2. Regenerate project files
3. Build the project

### Script runs but libwebsockets still not found
- Check that the script completed successfully (exit code 0)
- Verify the files were created in `ThirdParty/libwebsockets/`
- Try running the script manually to see detailed output

## Disabling Auto-Setup

If you want to disable automatic setup and use manual installation:

1. Comment out the `TryAutoSetupLibWebSockets()` call in `WebSockets.Build.cs`
2. Follow the manual installation instructions in `LIBWEBSOCKETS_GUIDE.md`

## CI/CD Integration

For CI/CD pipelines, you can either:

**Option 1:** Let auto-setup run during the build (adds ~5-10 minutes)

**Option 2:** Pre-build and cache libwebsockets:
```yaml
- name: Setup libwebsockets
  run: ./Plugins/WebSockets/Scripts/setup-libwebsockets.sh
  
- name: Cache libwebsockets
  uses: actions/cache@v3
  with:
    path: Plugins/WebSockets/ThirdParty/libwebsockets
    key: libwebsockets-v4.3.2-${{ runner.os }}
```

**Option 3:** Commit pre-built libraries to the repository
- Run the setup script locally
- Commit the `ThirdParty/libwebsockets` directory
- Auto-setup will detect existing files and skip

## Platform Notes

### Windows
- Uses Visual Studio compiler
- Builds 64-bit library for Win64 platform
- Requires Visual Studio 2019+ with C++ tools

### Linux
- Uses system compiler (gcc or clang)
- Builds for x86_64 architecture
- Requires standard build tools

### macOS
- Uses clang compiler from Xcode
- Builds universal binary if possible
- Requires Xcode Command Line Tools

## Version Updates

To update to a newer version of libwebsockets:
1. Edit `LIBWEBSOCKETS_VERSION` in the scripts
2. Test the build
3. Update this README with the new version number

## Support

For issues with:
- **Auto-setup**: Check build logs for `[WebSockets]` messages
- **libwebsockets build**: See libwebsockets documentation
- **Integration**: See main repository documentation

## License

These setup scripts are provided to help build libwebsockets, which is licensed under LGPL 2.1 with static linking exception. The built libraries follow libwebsockets licensing.
