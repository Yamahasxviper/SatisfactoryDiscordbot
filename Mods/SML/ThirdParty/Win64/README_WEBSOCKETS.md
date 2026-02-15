# Windows 64-bit Libraries

Place the following libwebsockets library files in this directory:

- `websockets.lib` - libwebsockets static library for Windows 64-bit

## How to Get the Library

1. Download or build libwebsockets from: https://github.com/warmcat/libwebsockets
2. For Windows, you can either:
   - Use pre-built binaries if available
   - Build from source using CMake and Visual Studio
3. Place the resulting `websockets.lib` (or `websockets_static.lib`) in this directory
4. Rename it to `websockets.lib` if necessary

## Notes

- Make sure to use the 64-bit version of the library
- The library should be compatible with the MSVC compiler used by Unreal Engine
- Additional system libraries (ws2_32.lib) are automatically linked by the build system
