# Quick Start: Adding libwebsockets to WebSockets Plugin

## What Was Done

I've configured the WebSockets plugin to use local libwebsockets files. The build system now checks for libwebsockets in this order:

1. **Plugin ThirdParty** (new!) - `Plugins/WebSockets/ThirdParty/libwebsockets/`
2. **Engine ThirdParty** - `{UnrealEngine}/Source/ThirdParty/libWebSockets/`
3. **Graceful degradation** - Build succeeds with WebSockets disabled

## Directory Structure Created

```
Plugins/WebSockets/ThirdParty/libwebsockets/
├── include/           ← Place libwebsockets.h here
│   └── README.md
└── lib/
    ├── Linux/x86_64-unknown-linux-gnu/
    │   └── README.md  ← Place libwebsockets.a here (for Linux)
    └── Win64/
        └── README.md  ← Place websockets_static.lib here (for Windows)
```

## What You Need to Do

### Step 1: Add Header Files

Copy `libwebsockets.h` (and any other `lws-*.h` headers) to:
```
Plugins/WebSockets/ThirdParty/libwebsockets/include/
```

### Step 2: Add Library Files

#### For Linux Builds:
Copy `libwebsockets.a` to:
```
Plugins/WebSockets/ThirdParty/libwebsockets/lib/Linux/x86_64-unknown-linux-gnu/
```

#### For Windows Builds:
Copy `websockets_static.lib` (or `websockets.lib`) to:
```
Plugins/WebSockets/ThirdParty/libwebsockets/lib/Win64/
```

#### For macOS Builds:
Create directory and copy `libwebsockets.a` to:
```
Plugins/WebSockets/ThirdParty/libwebsockets/lib/Mac/
```

## Where to Get libwebsockets Files

### Option 1: Build from Source

See the repository's [LIBWEBSOCKETS_GUIDE.md](../../../LIBWEBSOCKETS_GUIDE.md) for complete instructions.

Quick build commands for Linux:
```bash
git clone https://github.com/warmcat/libwebsockets.git
cd libwebsockets
git checkout v4.3.2

mkdir build && cd build
cmake .. \
  -DCMAKE_BUILD_TYPE=Release \
  -DLWS_WITH_SHARED=OFF \
  -DLWS_WITH_STATIC=ON \
  -DLWS_WITH_SSL=ON \
  -DLWS_WITH_ZLIB=ON

make -j$(nproc)

# Copy files
cp -r ../include/*.h /path/to/Plugins/WebSockets/ThirdParty/libwebsockets/include/
cp lib/libwebsockets.a /path/to/Plugins/WebSockets/ThirdParty/libwebsockets/lib/Linux/x86_64-unknown-linux-gnu/
```

### Option 2: Extract from Unreal Engine

If you have Unreal Engine with libwebsockets, you can copy from:
```
{UnrealEngine}/Source/ThirdParty/libWebSockets/
```

## Verification

After adding the files, when you build the project, you should see:
```
[WebSockets] Using local libwebsockets from plugin ThirdParty directory: ...
[WebSockets] Configuring build with local libwebsockets
```

If files aren't found, you'll see:
```
[WebSockets] libWebSockets not found in plugin ThirdParty: ...
[WebSockets] libWebSockets not found in engine ThirdParty: ...
```

## Platform-Specific Notes

### Windows
- Windows can use WinHttp instead (no libwebsockets needed)
- Only add libwebsockets files if you specifically want to use libwebsockets on Windows
- If using WinHttp, the build will automatically use it when libwebsockets isn't available

### Linux
- Linux builds require libwebsockets for WebSocket functionality
- Without it, build succeeds but WebSockets are disabled
- Make sure to build libwebsockets with `-DLWS_WITH_STATIC=ON` for static linking

### Dependencies

libwebsockets requires:
- OpenSSL (for SSL/TLS)
- zlib (for compression)

These will be automatically linked from the Unreal Engine installation. No need to add them separately.

## Testing

After adding the files:

1. Clean your build:
   ```bash
   rm -rf Intermediate/ Saved/
   ```

2. Regenerate project files (right-click .uproject → Generate Visual Studio project files)

3. Build the project:
   ```bash
   # Example for Linux
   UnrealBuildTool FactoryEditor Linux Development -project=FactoryGame.uproject
   ```

4. Check the build log for the "[WebSockets]" messages to confirm it's using local files

## Troubleshooting

### "libwebsockets.h not found" error
- Verify the header file is in `ThirdParty/libwebsockets/include/`
- Check filename is exactly `libwebsockets.h` (case-sensitive on Linux)

### Linking errors
- Verify the library file is in the correct platform directory
- Ensure library was built with static linking enabled
- Check that library architecture matches (x64)

### Build succeeds but warns "not found"
- Check directory structure matches exactly
- Ensure at least `libwebsockets.h` is present in `include/`
- Try adding all `*.h` files from libwebsockets

## Need Help?

See the comprehensive guides in the repository:
- [LIBWEBSOCKETS_GUIDE.md](../../../LIBWEBSOCKETS_GUIDE.md) - Complete building guide
- [ThirdParty/README.md](../ThirdParty/README.md) - Detailed ThirdParty directory docs
- [BUILD_REQUIREMENTS.md](../../../BUILD_REQUIREMENTS.md) - Build requirements
- [TROUBLESHOOTING.md](../../../TROUBLESHOOTING.md) - Common issues

## Summary

You now have a plugin-local ThirdParty directory ready to receive libwebsockets files. Just:
1. Add `libwebsockets.h` to `ThirdParty/libwebsockets/include/`
2. Add library files to the appropriate `lib/` subdirectory for your platform
3. Build - it will automatically detect and use them!

The build system is now configured to prioritize your local files over engine files, giving you full control over the libwebsockets version used.
