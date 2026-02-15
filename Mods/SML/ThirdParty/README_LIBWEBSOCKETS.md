# libwebsockets Third-Party Library Setup

## Required Files

This project uses libwebsockets as a third-party dependency. To build successfully, you need to provide the following files:

### Header Files
Place in `ThirdParty/include/`:
- `libwebsockets.h` - Main libwebsockets header file
- Any other required header files from your libwebsockets distribution (if needed)

### Library Files

#### Windows (Win64)
Place in `ThirdParty/Win64/`:
- `websockets.lib` - libwebsockets static library for Windows 64-bit
- `websockets_static.lib` (if using static linking)

#### Linux
Place in `ThirdParty/Linux/`:
- `libwebsockets.a` - libwebsockets static library for Linux

## Where to Get libwebsockets

You can obtain libwebsockets from:
- Official repository: https://github.com/warmcat/libwebsockets
- Pre-built binaries for your platform
- Build from source following the official documentation

## Build Configuration

The build system (SML.Build.cs) is configured to:
1. Include headers from `ThirdParty/include/`
2. Link against the appropriate platform-specific library
3. Add necessary system dependencies (ws2_32.lib for Windows, etc.)

## Notes

- Ensure the libwebsockets version is compatible with your project requirements
- For Windows, you may also need to link against system libraries (ws2_32.lib, etc.)
- For Linux, additional system dependencies may be required
