# libwebsockets Third-Party Libraries

This directory contains the libwebsockets library files for the WebSockets plugin.

## Directory Structure

```
ThirdParty/libwebsockets/
├── include/                      # Header files
│   ├── libwebsockets.h          # Main header (you add this)
│   └── lws-*.h                  # Additional headers (if needed)
├── lib/
│   ├── Linux/
│   │   └── x86_64-unknown-linux-gnu/
│   │       └── libwebsockets.a  # Static library for Linux (you add this)
│   └── Win64/
│       └── websockets.lib       # Static library for Windows (you add this)
```

## How to Add libwebsockets

### Option 1: Build from Source (Recommended)

See the main repository's [LIBWEBSOCKETS_GUIDE.md](../../../LIBWEBSOCKETS_GUIDE.md) for complete build instructions.

Quick steps for Linux:
```bash
# Clone and build libwebsockets
git clone https://github.com/warmcat/libwebsockets.git
cd libwebsockets
git checkout v4.3.2

mkdir build && cd build
cmake .. \
  -DCMAKE_BUILD_TYPE=Release \
  -DLWS_WITH_SHARED=OFF \
  -DLWS_WITH_STATIC=ON \
  -DLWS_WITH_SSL=ON \
  -DLWS_WITH_ZLIB=ON \
  -DLWS_IPV6=ON \
  -DLWS_WITHOUT_TESTAPPS=ON

make -j$(nproc)

# Copy files to this directory
cp -r ../include/*.h /path/to/Plugins/WebSockets/ThirdParty/libwebsockets/include/
cp lib/libwebsockets.a /path/to/Plugins/WebSockets/ThirdParty/libwebsockets/lib/Linux/x86_64-unknown-linux-gnu/
```

### Option 2: Use Pre-built Libraries

If you have pre-built libwebsockets libraries:

1. Copy header files to `include/`
2. Copy library files to the appropriate `lib/` subdirectory:
   - Linux: `lib/Linux/x86_64-unknown-linux-gnu/libwebsockets.a`
   - Windows: `lib/Win64/websockets.lib` and `websockets_static.lib`

### Required Files

**Headers (include/):**
- `libwebsockets.h` - Main header (required)
- `lws-*.h` - Additional headers may be needed depending on version

**Libraries (lib/):**

**For Linux:**
- `lib/Linux/x86_64-unknown-linux-gnu/libwebsockets.a` - Static library

**For Windows:**
- `lib/Win64/websockets_static.lib` - Static library (if using static build)
- `lib/Win64/websockets.lib` - Import library (if using dynamic build)

## Build System Integration

The `WebSockets.Build.cs` automatically detects if libwebsockets is present in this directory:

1. **First**, it checks this local ThirdParty directory
2. **If not found**, it checks the Unreal Engine's ThirdParty directory
3. **If still not found**, it disables libwebsockets support (graceful degradation)

## Dependencies

libwebsockets requires:
- OpenSSL (for SSL/TLS support)
- zlib (for compression)

These are typically provided by the Unreal Engine installation at:
- `{UnrealEngine}/Engine/Source/ThirdParty/OpenSSL/`
- `{UnrealEngine}/Engine/Source/ThirdParty/zlib/`

## Platform Notes

### Windows
- Windows builds can use WinHttp instead of libwebsockets (no setup needed)
- If you want to use libwebsockets on Windows, place `websockets_static.lib` in `lib/Win64/`

### Linux
- Linux builds require libwebsockets for WebSocket functionality
- Place `libwebsockets.a` in `lib/Linux/x86_64-unknown-linux-gnu/`

### macOS
- macOS builds require libwebsockets
- Place library in `lib/Mac/` (directory structure similar to Linux)

## Verification

After adding the files, the build system will automatically detect them. You should see in the build output:
```
[WebSockets] Using local libwebsockets from plugin ThirdParty directory
```

If not found, you'll see:
```
[WebSockets] libwebsockets not found in plugin ThirdParty, checking engine...
```

## Troubleshooting

### Headers Found but Build Fails

Make sure you also added the library files (`.a` or `.lib`) to the appropriate `lib/` directory.

### Linking Errors

Ensure:
1. The library was built for the correct platform/architecture
2. The library was built with the same compiler version as Unreal Engine
3. OpenSSL and zlib are available in the UE ThirdParty directory

### Version Compatibility

Use libwebsockets v4.3.x for best compatibility with Unreal Engine 5.3.x.

## Additional Resources

- [Main libwebsockets Guide](../../../LIBWEBSOCKETS_GUIDE.md)
- [Build Requirements](../../../BUILD_REQUIREMENTS.md)
- [libwebsockets GitHub](https://github.com/warmcat/libwebsockets)
