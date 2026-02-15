# libwebsockets Auto-Setup Feature - Complete Overview

## 🎉 Feature Complete!

libwebsockets now **automatically sets itself up** when you compile your Unreal Engine project!

## Quick Summary

**Before this feature:**
- ❌ Manual download and build required
- ❌ Complex CMake commands to remember
- ❌ Platform-specific build instructions
- ❌ Copy files to correct directories
- ❌ Time-consuming setup process

**After this feature:**
- ✅ Just build your project
- ✅ Everything happens automatically
- ✅ Works on Windows, Linux, and macOS
- ✅ Zero configuration needed
- ✅ Self-contained plugin

## Architecture

### Auto-Setup Flow

```
┌─────────────────────────────────────┐
│   User: Build UE Project           │
└─────────────┬───────────────────────┘
              │
              ▼
┌─────────────────────────────────────┐
│   Unreal Build Tool (UBT)           │
│   Compiles WebSockets Module        │
└─────────────┬───────────────────────┘
              │
              ▼
┌─────────────────────────────────────┐
│   WebSockets.Build.cs               │
│   Checks LibWebSocketsAvailable     │
└─────────────┬───────────────────────┘
              │
              ▼
        ┌─────┴─────┐
        │  Found?   │
        └───┬───┬───┘
            │   │
         Yes│   │No
            │   │
            ▼   ▼
            │   │
            │   └──────────────────────┐
            │                          │
            │                          ▼
            │        ┌─────────────────────────────────┐
            │        │  TryAutoSetupLibWebSockets()    │
            │        │  - Detect platform              │
            │        │  - Select script                │
            │        │  - Execute setup                │
            │        └─────────┬───────────────────────┘
            │                  │
            │                  ▼
            │        ┌─────────────────────────────────┐
            │        │  Platform Script Executes       │
            │        │  Windows: Setup-*.ps1           │
            │        │  Linux/Mac: setup-*.sh          │
            │        └─────────┬───────────────────────┘
            │                  │
            │                  ▼
            │        ┌─────────────────────────────────┐
            │        │  Script Actions:                │
            │        │  1. git clone libwebsockets     │
            │        │  2. cmake configure             │
            │        │  3. make/build                  │
            │        │  4. Copy headers                │
            │        │  5. Copy libraries              │
            │        └─────────┬───────────────────────┘
            │                  │
            │                  ▼
            │        ┌─────────────────────────────────┐
            │        │  Install to:                    │
            │        │  Plugins/WebSockets/            │
            │        │    ThirdParty/libwebsockets/    │
            │        └─────────┬───────────────────────┘
            │                  │
            │                  ▼
            └──────────────►┌─────────────────────────┐
                            │  Check again: Found!    │
                            └─────────┬───────────────┘
                                      │
                                      ▼
                            ┌─────────────────────────┐
                            │  Configure build with   │
                            │  local libwebsockets    │
                            │  - Add include paths    │
                            │  - Link libraries       │
                            └─────────┬───────────────┘
                                      │
                                      ▼
                            ┌─────────────────────────┐
                            │  Continue UBT Build     │
                            │  ✅ WebSockets enabled  │
                            └─────────────────────────┘
```

## File Structure

```
SatisfactoryDiscordbot/
├── Plugins/
│   └── WebSockets/
│       ├── Scripts/                          ← NEW!
│       │   ├── README.md                     ← Auto-setup documentation
│       │   ├── setup-libwebsockets.sh        ← Linux/Mac setup script
│       │   └── Setup-LibWebSockets.ps1       ← Windows setup script
│       ├── Source/
│       │   └── WebSockets/
│       │       └── WebSockets.Build.cs       ← MODIFIED: Added auto-setup
│       ├── ThirdParty/
│       │   ├── README.md
│       │   └── libwebsockets/
│       │       ├── include/                  ← Auto-populated by scripts
│       │       │   └── libwebsockets.h
│       │       └── lib/                      ← Auto-populated by scripts
│       │           ├── Linux/x86_64-unknown-linux-gnu/
│       │           │   └── libwebsockets.a
│       │           ├── Win64/
│       │           │   └── websockets_static.lib
│       │           └── Mac/
│       │               └── libwebsockets.a
│       ├── QUICK_START_LOCAL_LIBWEBSOCKETS.md  ← UPDATED
│       └── WebSockets.uplugin
├── LIBWEBSOCKETS_GUIDE.md                    ← UPDATED
└── README.md                                  ← UPDATED
```

## Implementation Details

### WebSockets.Build.cs Changes

**New method added:**
```csharp
private bool TryAutoSetupLibWebSockets()
{
    // Prevent multiple attempts
    if (_autoSetupAttempted) return false;
    _autoSetupAttempted = true;
    
    // Determine platform script
    string SetupScript = null;
    string ScriptRunner = null;
    
    if (Target.Platform == UnrealTargetPlatform.Win64) {
        SetupScript = Path.Combine(ScriptsDir, "Setup-LibWebSockets.ps1");
        ScriptRunner = "powershell.exe";
    } else if (Target.Platform == UnrealTargetPlatform.Linux || Target.Platform == UnrealTargetPlatform.Mac) {
        SetupScript = Path.Combine(ScriptsDir, "setup-libwebsockets.sh");
        ScriptRunner = "/bin/bash";
    }
    
    // Execute script
    ProcessStartInfo psi = new ProcessStartInfo {
        FileName = ScriptRunner,
        Arguments = SetupScript,
        UseShellExecute = false,
        RedirectStandardOutput = true,
        RedirectStandardError = true
    };
    
    using (Process process = Process.Start(psi)) {
        process.WaitForExit();
        return process.ExitCode == 0;
    }
}
```

**Integration point:**
```csharp
protected virtual bool LibWebSocketsAvailable
{
    get
    {
        // Check plugin ThirdParty
        // Check engine ThirdParty
        
        // If not found - auto-setup!
        if (PlatformSupportsLibWebsockets) {
            if (TryAutoSetupLibWebSockets()) {
                // Re-check after setup
                if (headers found) {
                    return true;
                }
            }
        }
        
        return false;
    }
}
```

### Setup Scripts

**setup-libwebsockets.sh (Linux/Mac):**
- Checks if already installed
- Clones libwebsockets v4.3.2
- Configures with CMake
- Builds with make
- Copies files to plugin ThirdParty
- Cleans up temp files

**Setup-LibWebSockets.ps1 (Windows):**
- Checks if already installed  
- Clones libwebsockets v4.3.2
- Configures with CMake for Visual Studio
- Builds in Release mode
- Copies files to plugin ThirdParty
- Cleans up temp files

## Prerequisites

### Linux (Ubuntu/Debian)
```bash
sudo apt-get install git cmake build-essential
```

### macOS
```bash
brew install git cmake
# Xcode Command Line Tools
```

### Windows
- Git for Windows
- CMake
- Visual Studio 2019 or 2022 with C++ tools

## Build Times

| Scenario | First Build | Subsequent Builds |
|----------|-------------|-------------------|
| **With auto-setup** | +5-10 minutes | Normal speed |
| **Manual setup** | Normal speed | Normal speed |
| **No setup** | Normal speed | Normal speed (no WebSockets) |

## Platform-Specific Details

### Windows Build
```powershell
# Auto-runs during build
powershell.exe -ExecutionPolicy Bypass -File Setup-LibWebSockets.ps1

# Downloads libwebsockets v4.3.2
# Builds with: cmake .. -A x64
# Builds: cmake --build . --config Release
# Outputs: websockets_static.lib
# Installs to: ThirdParty/libwebsockets/lib/Win64/
```

### Linux Build
```bash
# Auto-runs during build
/bin/bash setup-libwebsockets.sh

# Downloads libwebsockets v4.3.2
# Builds with: cmake .. -DCMAKE_BUILD_TYPE=Release
# Builds: make -j$(nproc)
# Outputs: libwebsockets.a
# Installs to: ThirdParty/libwebsockets/lib/Linux/x86_64-unknown-linux-gnu/
```

### macOS Build
```bash
# Auto-runs during build
/bin/bash setup-libwebsockets.sh

# Downloads libwebsockets v4.3.2
# Builds with: cmake .. -DCMAKE_BUILD_TYPE=Release
# Builds: make -j$(sysctl -n hw.ncpu)
# Outputs: libwebsockets.a
# Installs to: ThirdParty/libwebsockets/lib/Mac/
```

## Build Configuration

All builds use these CMake options:
```cmake
-DCMAKE_BUILD_TYPE=Release
-DLWS_WITH_SHARED=OFF          # Static library only
-DLWS_WITH_STATIC=ON           # Build static lib
-DLWS_WITH_SSL=ON              # Enable SSL/TLS for wss://
-DLWS_WITH_ZLIB=ON             # Enable compression
-DLWS_IPV6=ON                  # IPv6 support
-DLWS_WITHOUT_TESTAPPS=ON      # Skip tests
-DLWS_WITHOUT_TEST_SERVER=ON   # Skip test server
-DLWS_WITHOUT_TEST_CLIENT=ON   # Skip test client
```

## User Experience

### First-Time Build Log

```
Building WebSockets module...
[WebSockets] libWebSockets not found in plugin ThirdParty: /path/to/Plugins/WebSockets/ThirdParty/libwebsockets
[WebSockets] libWebSockets not found in engine ThirdParty: /path/to/UE/Engine/Source/ThirdParty/libWebSockets
[WebSockets] Attempting to auto-setup libwebsockets...
[WebSockets] Running: /bin/bash "/path/to/Plugins/WebSockets/Scripts/setup-libwebsockets.sh"
[WebSockets] Auto-setup completed successfully
[WebSockets] Setup output:
=========================================
libwebsockets Auto-Setup Script
=========================================
Plugin Directory: /path/to/Plugins/WebSockets
ThirdParty Directory: /path/to/Plugins/WebSockets/ThirdParty/libwebsockets
Build Directory: /tmp/libwebsockets-build-12345

⚠ libwebsockets not found or incomplete, building from source...

→ Cloning libwebsockets v4.3.2...
Cloning into 'libwebsockets'...
→ Configuring build with CMake...
-- The C compiler identification is GNU 11.4.0
-- Configuring done
-- Generating done
→ Building libwebsockets...
[ 98%] Building C object lib/CMakeFiles/websockets.dir/libwebsockets.c.o
[100%] Linking C static library libwebsockets.a
→ Creating target directories...
→ Copying headers...
→ Copying library...
→ Cleaning up build directory...

=========================================
✓ libwebsockets setup complete!
=========================================
Headers: /path/to/Plugins/WebSockets/ThirdParty/libwebsockets/include/
Library: /path/to/Plugins/WebSockets/ThirdParty/libwebsockets/lib/Linux/x86_64-unknown-linux-gnu/

[WebSockets] Auto-setup successful! Using local libwebsockets
[WebSockets] Using local libwebsockets from plugin ThirdParty directory: /path/to/Plugins/WebSockets/ThirdParty/libwebsockets
[WebSockets] Configuring build with local libwebsockets
Building WebSockets module... DONE
```

### Subsequent Build Log

```
Building WebSockets module...
[WebSockets] Using local libwebsockets from plugin ThirdParty directory: /path/to/Plugins/WebSockets/ThirdParty/libwebsockets
[WebSockets] Configuring build with local libwebsockets
Building WebSockets module... DONE
```

Fast and clean!

## CI/CD Support

### GitHub Actions Example

```yaml
name: Build Project

on: [push, pull_request]

jobs:
  build:
    runs-on: ubuntu-latest
    steps:
      - uses: actions/checkout@v3
      
      # Install prerequisites
      - name: Install build tools
        run: |
          sudo apt-get update
          sudo apt-get install -y git cmake build-essential
      
      # Build project - auto-setup happens here
      - name: Build Project
        run: |
          ./Engine/Build/BatchFiles/Linux/Build.sh \
            FactoryEditor Linux Development \
            -project="$(pwd)/FactoryGame.uproject"
      
      # libwebsockets was automatically set up during build!
```

### With Caching (Faster)

```yaml
- name: Cache libwebsockets
  id: cache-libwebsockets
  uses: actions/cache@v3
  with:
    path: Plugins/WebSockets/ThirdParty/libwebsockets
    key: libwebsockets-v4.3.2-${{ runner.os }}
    
- name: Setup libwebsockets (if not cached)
  if: steps.cache-libwebsockets.outputs.cache-hit != 'true'
  run: ./Plugins/WebSockets/Scripts/setup-libwebsockets.sh
  
- name: Build Project
  run: Build.sh FactoryEditor Linux Development
```

## Troubleshooting

### Auto-setup doesn't trigger
**Problem:** Building but setup script doesn't run
**Solution:** 
- Clean build: `rm -rf Intermediate/ Saved/`
- Regenerate project files
- Build again

### Auto-setup fails
**Problem:** Script runs but exits with error
**Solution:**
- Check build logs for specific error
- Ensure prerequisites installed
- Check internet connection
- Check disk space (~500MB needed)

### Prerequisites not installed
**Problem:** "cmake: command not found"
**Solution:** Install required tools for your platform

### Permission denied
**Problem:** Script can't execute
**Solution:** 
```bash
chmod +x Plugins/WebSockets/Scripts/setup-libwebsockets.sh
```

### Want to skip auto-setup
**Problem:** Want manual control
**Solution:** Comment out auto-setup call in Build.cs or provide files manually

## Performance Optimization

### For Development Machines
- First build: Auto-setup runs (~10 min)
- Subsequent: Fast (libraries cached)

### For CI/CD
**Option 1:** Auto-setup each time (simple, slower)
**Option 2:** Cache libwebsockets directory (faster)
**Option 3:** Commit pre-built libraries (fastest)

## Security Considerations

- Downloads from official libwebsockets GitHub
- Uses verified release tags (v4.3.2)
- Build from source (no pre-built binaries)
- Scripts in version control (reviewable)

## Future Enhancements

Possible improvements:
- [ ] Download pre-built binaries (faster than building)
- [ ] Multiple version support
- [ ] Offline mode with bundled sources
- [ ] Progress indicator during build
- [ ] Retry logic for download failures

## Support

**Documentation:**
- [Plugins/WebSockets/Scripts/README.md](Plugins/WebSockets/Scripts/README.md)
- [LIBWEBSOCKETS_GUIDE.md](LIBWEBSOCKETS_GUIDE.md)
- [TROUBLESHOOTING.md](TROUBLESHOOTING.md)

**Manual Execution:**
```bash
# Linux/Mac
cd Plugins/WebSockets/Scripts
./setup-libwebsockets.sh

# Windows
cd Plugins\WebSockets\Scripts
.\Setup-LibWebSockets.ps1
```

## Summary

✅ **Mission Accomplished!**

libwebsockets now automatically sets itself up during the Unreal Engine build process. Users just need to have build tools installed, and everything else happens automatically.

**Key Achievements:**
- Zero-configuration setup
- Cross-platform support (Windows, Linux, macOS)
- Integrated into UBT build process
- Comprehensive documentation
- CI/CD friendly
- Self-contained plugin

**User Experience:**
1. Install git, cmake, build tools (one-time)
2. Build UE project normally
3. First build: libwebsockets auto-installs (5-10 min)
4. Subsequent builds: Fast (already installed)
5. Enjoy Discord Gateway features!

🚀 **It just works!**
