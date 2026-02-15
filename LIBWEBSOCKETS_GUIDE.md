# libwebsockets.h Build and Setup Guide

## Overview

This guide explains how to build and configure libwebsockets for use with the WebSockets plugin in this project, covering both Windows and Linux platforms.

## Platform Architecture

The WebSockets plugin uses **different implementations** depending on the target platform:

### Windows (Win64)
- **Implementation**: WinHttp (Windows HTTP Services)
- **Location**: Built into Windows SDK 8.1+
- **Dependencies**: None - provided by Windows
- **Build Requirement**: Windows SDK 8.1 or later
- **Status**: ✅ **No libwebsockets needed**

### Linux / macOS / Unix
- **Implementation**: libwebsockets (open source WebSocket library)
- **Location**: `{UnrealEngine}/Source/ThirdParty/libWebSockets/`
- **Dependencies**: OpenSSL, zlib (also in UE ThirdParty)
- **Build Requirement**: libwebsockets library in Unreal Engine
- **Status**: ⚠️ **Requires libwebsockets**

## Understanding the Build System

The `WebSockets.Build.cs` file automatically detects which platform you're building for and selects the appropriate implementation:

```csharp
// Windows builds use WinHttp
if (bPlatformSupportsWinHttpWebSockets) {
    AddEngineThirdPartyPrivateStaticDependencies(Target, "WinHttp");
}

// Linux/Mac builds use libwebsockets
if (PlatformSupportsLibWebsockets && LibWebSocketsAvailable) {
    AddEngineThirdPartyPrivateStaticDependencies(Target, "OpenSSL", "libWebSockets", "zlib");
}
```

## Where libwebsockets Should Be Located

For Linux builds, the Unreal Build Tool expects libwebsockets at:

```
{UnrealEngineDirectory}/Source/ThirdParty/libWebSockets/
```

### Typical Unreal Engine Installation Structure

```
UnrealEngine/
├── Engine/
│   └── Source/
│       └── ThirdParty/
│           ├── libWebSockets/          ← Must be here for Linux builds
│           │   ├── libwebsockets/
│           │   │   └── include/
│           │   │       └── libwebsockets.h
│           │   └── lib/
│           │       └── Linux/
│           │           └── x86_64-unknown-linux-gnu/
│           │               └── libwebsockets.a
│           ├── OpenSSL/
│           ├── zlib/
│           └── ...
```

## Option 1: Use Unreal Engine with libwebsockets (Recommended)

### For This Project

This project uses **Unreal Engine 5.3.2-CSS** (Coffee Stain Studios custom build). The CS

S custom build **should include** libwebsockets in its ThirdParty directory.

#### Verification Steps

1. Check if libwebsockets is present in your UE installation:
   ```bash
   # Linux/Mac
   ls -la ~/UnrealEngine/Engine/Source/ThirdParty/libWebSockets/
   
   # Or check wherever your UE is installed
   ls -la /path/to/UnrealEngine/Engine/Source/ThirdParty/libWebSockets/
   ```

2. If the directory exists, you're all set! The build will work automatically.

3. If the directory is missing, see Option 2 or Option 3 below.

### Standard Unreal Engine Versions

If you're using standard Epic Games Unreal Engine (not CSS custom build):

- **UE 5.x**: libwebsockets is typically included in the ThirdParty directory
- **UE 4.x**: libwebsockets is included in most versions 4.20+

#### What if libwebsockets is missing?

If your Unreal Engine installation doesn't have libwebsockets:
1. You may have an incomplete installation
2. Re-download or verify your UE installation
3. Or manually add libwebsockets (see Option 3)

## Option 2: Build Without libwebsockets (Graceful Degradation)

As of recent updates, this project's WebSockets plugin will **gracefully degrade** if libwebsockets is not available:

### What Happens When libwebsockets is Missing

1. **Build Succeeds**: The project will compile without errors
2. **Warning Messages**: Build log will show:
   ```
   Warning: [WebSockets] libWebSockets third-party library not found at {path}
   Warning: [WebSockets] WebSocket functionality will be disabled. Dependent features (like Discord Gateway) will not work.
   ```
3. **Preprocessor Defines**:
   - `WITH_WEBSOCKETS=0`
   - `WITH_LIBWEBSOCKETS=0`
4. **WebSocket Code Skipped**: Lws implementation files are not compiled
5. **DiscordChatBridge Fallback**: Automatically uses REST-only mode

### Impact on Functionality

When building without libwebsockets:

✅ **Still Works:**
- Project compiles successfully
- Server runs normally
- DiscordChatBridge uses REST API for chat messages

❌ **Disabled Features:**
- Discord Gateway (real-time WebSocket connection)
- Bot presence/status ("Playing with X players")
- Real-time event notifications

### When to Use This Option

- Quick local development/testing
- Don't need Discord Gateway features
- Can't or don't want to install libwebsockets
- Windows builds (uses WinHttp automatically)

## Option 3: Manually Build and Install libwebsockets

If you need WebSocket functionality on Linux and libwebsockets is missing from your UE installation, you can build and install it manually.

### Prerequisites

- CMake 3.15+
- C/C++ compiler (GCC or Clang)
- OpenSSL development libraries
- zlib development libraries

### Building libwebsockets from Source

#### Step 1: Download libwebsockets

The compatible version for Unreal Engine is typically **v4.3.x**:

```bash
# Clone the repository
git clone https://github.com/warmcat/libwebsockets.git
cd libwebsockets
git checkout v4.3.2  # Use a stable 4.3.x version
```

#### Step 2: Configure Build

```bash
mkdir build
cd build

cmake .. \
  -DCMAKE_BUILD_TYPE=Release \
  -DLWS_WITH_SHARED=OFF \
  -DLWS_WITH_STATIC=ON \
  -DLWS_WITH_SSL=ON \
  -DLWS_WITH_ZLIB=ON \
  -DLWS_WITH_LIBUV=OFF \
  -DLWS_WITH_LIBEVENT=OFF \
  -DLWS_IPV6=ON \
  -DLWS_WITHOUT_TESTAPPS=ON \
  -DLWS_WITHOUT_TEST_SERVER=ON \
  -DLWS_WITHOUT_TEST_CLIENT=ON \
  -DCMAKE_INSTALL_PREFIX=/path/to/UnrealEngine/Engine/Source/ThirdParty/libWebSockets
```

Key options explained:
- `WITH_SHARED=OFF`: Build static library (required by UE)
- `WITH_SSL=ON`: Enable SSL/TLS support (required for `wss://`)
- `IPV6=ON`: Enable IPv6 support
- `WITHOUT_TESTAPPS=ON`: Skip building test programs

#### Step 3: Build and Install

```bash
make -j$(nproc)
make install
```

#### Step 4: Verify Installation

```bash
# Check that headers are installed
ls -la /path/to/UnrealEngine/Engine/Source/ThirdParty/libWebSockets/include/libwebsockets.h

# Check that library is installed
ls -la /path/to/UnrealEngine/Engine/Source/ThirdParty/libWebSockets/lib/
```

### Directory Structure After Installation

After installing libwebsockets, your UE ThirdParty directory should look like:

```
Engine/Source/ThirdParty/libWebSockets/
├── include/
│   ├── libwebsockets.h
│   └── lws-*.h (other headers)
├── lib/
│   ├── libwebsockets.a (or .lib for Windows)
│   └── cmake/
└── share/
```

## Option 4: Using System libwebsockets (Advanced)

For advanced users, you can modify the build system to use system-installed libwebsockets instead of UE's ThirdParty version.

### Install System Package

```bash
# Ubuntu/Debian
sudo apt-get install libwebsockets-dev

# Fedora/RHEL
sudo dnf install libwebsockets-devel

# Arch
sudo pacman -S libwebsockets

# macOS with Homebrew
brew install libwebsockets
```

### Modify WebSockets.Build.cs

You would need to modify `Plugins/WebSockets/Source/WebSockets/WebSockets.Build.cs` to use system libraries:

```csharp
// Instead of AddEngineThirdPartyPrivateStaticDependencies
PublicIncludePaths.Add("/usr/include");
PublicAdditionalLibraries.Add("websockets");
```

**⚠️ Warning**: This approach is not recommended because:
- May cause version conflicts
- Not portable across machines
- Breaks the self-contained nature of the project
- May not work with UE's build system

## Troubleshooting

### Issue: "libwebsockets.h not found" on Linux

**Cause**: libwebsockets not present in UE ThirdParty directory

**Solution**: 
1. Check if your UE installation is complete
2. Use Option 1 (verify UE installation)
3. Or use Option 2 (build without libwebsockets)
4. Or use Option 3 (manually build and install)

### Issue: Build succeeds but warnings about disabled WebSockets

**Cause**: This is expected when libwebsockets is not available

**Solution**: 
- If you need WebSocket features: Install libwebsockets (Option 3)
- If you don't need WebSocket features: Ignore warnings, everything else works

### Issue: Linking errors with libwebsockets on Linux

**Possible Causes**:
- libwebsockets built with wrong flags
- Missing dependencies (OpenSSL, zlib)
- Version mismatch

**Solutions**:
1. Ensure libwebsockets is built as static library (`-DLWS_WITH_STATIC=ON`)
2. Verify OpenSSL and zlib are in UE ThirdParty
3. Use libwebsockets v4.3.x (compatible with UE 5.x)
4. Check build logs for specific missing symbols

### Issue: WebSocket connections fail at runtime

**Possible Causes**:
- Built without SSL support
- Network/firewall issues
- Incompatible libwebsockets version

**Solutions**:
1. Verify libwebsockets was built with `-DLWS_WITH_SSL=ON`
2. Check that OpenSSL libraries are linked
3. Test network connectivity to Discord Gateway (`wss://gateway.discord.gg`)

## Platform-Specific Notes

### Windows

- **Don't need libwebsockets at all**
- Uses WinHttp (included in Windows SDK)
- Requires Windows SDK 8.1+ (for Win64 builds)
- Build with Visual Studio 2019 or 2022

### Linux

- **Requires libwebsockets** for WebSocket support
- Alternative: Build without WebSockets (REST-only mode)
- Tested on Ubuntu 20.04+, Debian 11+
- Requires GCC 9+ or Clang 10+

### macOS

- **Requires libwebsockets** for WebSocket support  
- Usually included in Epic UE distributions
- Alternative: Install via Homebrew, then use Option 3

## CI/CD Considerations

### GitHub Actions / Build Systems

For CI builds on Linux:

1. **Option A**: Use UE installation that includes libwebsockets
   - Most Epic UE distributions include it
   - CSS custom build should include it

2. **Option B**: Build without libwebsockets
   - Faster CI builds
   - No external dependencies
   - REST-only mode for DiscordChatBridge

3. **Option C**: Cache pre-built libwebsockets
   - Build once, cache the artifact
   - Restore from cache in CI
   - Faster than building each time

### Current CI Configuration

This project's CI (`.github/workflows/build.yml`) currently:
- Runs on **Windows** (self-hosted runner)
- Uses **WinHttp** implementation
- **Doesn't require libwebsockets**
- Builds successfully without any additional setup

For Linux CI builds (not currently configured):
- Would need libwebsockets in UE installation
- Or would build without WebSockets (REST-only mode)

## Summary

### Quick Decision Guide

**"I'm building on Windows"**
→ No action needed. Uses WinHttp automatically.

**"I'm building on Linux for production with Discord Gateway"**
→ Use Option 1: Verify libwebsockets in UE installation. If missing, use Option 3 to build it.

**"I'm building on Linux for quick testing without Discord Gateway"**
→ Use Option 2: Build without libwebsockets (graceful degradation). Works fine, just no Gateway.

**"I'm setting up CI/CD"**
→ If Windows CI: No action needed.
→ If Linux CI: Include libwebsockets in UE, or build without WebSockets for REST-only mode.

## Additional Resources

- [libwebsockets Official Repository](https://github.com/warmcat/libwebsockets)
- [libwebsockets Documentation](https://libwebsockets.org/)
- [Unreal Engine Build Configuration](https://docs.unrealengine.com/5.3/en-US/build-configuration-for-unreal-engine/)
- [This Project's BUILD_REQUIREMENTS.md](BUILD_REQUIREMENTS.md)
- [WebSocket Build Compatibility](WEBSOCKET_BUILD_COMPATIBILITY.md)

## Getting Help

If you're still having issues:

1. Check build logs for specific error messages
2. Verify your UE installation is complete
3. Consider using graceful degradation (Option 2) if you don't need WebSocket features
4. Open an issue with:
   - Your platform (Windows/Linux/Mac)
   - UE version
   - Build error messages
   - Whether libwebsockets directory exists in your UE installation
