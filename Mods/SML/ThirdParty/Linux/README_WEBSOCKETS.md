# Linux Libraries

Place the following libwebsockets library files in this directory:

- `libwebsockets.a` - libwebsockets static library for Linux

## How to Get the Library

1. Download or build libwebsockets from: https://github.com/warmcat/libwebsockets
2. For Linux, you can either:
   - Use pre-built binaries if available
   - Build from source using CMake and GCC/Clang
3. Place the resulting `libwebsockets.a` in this directory

## Notes

- The library should be built for Linux x86_64 architecture
- Use the Unreal Engine cross-compile toolchain on Windows if cross-compiling
- Additional system dependencies may need to be available at runtime
