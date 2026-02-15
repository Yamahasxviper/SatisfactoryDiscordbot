# How to Add libwebsockets to Your Project

This document explains how to complete the libwebsockets integration for your Satisfactory Mod Loader project.

## What Has Been Done

The build system (`Mods/SML/Source/SML/SML.Build.cs`) has been configured to:

1. ✅ Include headers from `ThirdParty/include/` (libwebsockets.h is already in the include path)
2. ✅ Link against `websockets.lib` on Windows 64-bit
3. ✅ Link against `libwebsockets.a` on Linux
4. ✅ Link against required system libraries (ws2_32.lib on Windows)

## What You Need to Do

To complete the setup and fix the build error, you need to provide the actual libwebsockets files:

### Step 1: Get libwebsockets

Download or build libwebsockets from the official repository:
- Repository: https://github.com/warmcat/libwebsockets
- Latest release: Check the releases page

You can either:
- Use pre-built binaries (if available for your platform)
- Build from source using CMake

### Step 2: Replace the Placeholder Header

Replace the placeholder file at:
```
Mods/SML/ThirdParty/include/libwebsockets.h
```

With the actual `libwebsockets.h` header file from your libwebsockets distribution.

**Important:** The placeholder file currently contains an `#error` directive that will cause compilation to fail. You MUST replace it with the real header.

### Step 3: Add the Library Files

#### For Windows (Win64)

Place the libwebsockets static library in:
```
Mods/SML/ThirdParty/Win64/websockets.lib
```

If your library file has a different name (e.g., `websockets_static.lib`), rename it to `websockets.lib`.

#### For Linux

Place the libwebsockets static library in:
```
Mods/SML/ThirdParty/Linux/libwebsockets.a
```

### Step 4: Additional Header Files (if needed)

If libwebsockets requires additional header files, place them in:
```
Mods/SML/ThirdParty/include/
```

Common additional headers might include:
- `lws_config.h`
- Platform-specific headers

## Building libwebsockets from Source

If you need to build libwebsockets yourself:

### Windows
```bash
git clone https://github.com/warmcat/libwebsockets.git
cd libwebsockets
mkdir build
cd build
cmake .. -DLWS_WITH_STATIC=ON -DLWS_WITHOUT_TESTAPPS=ON
cmake --build . --config Release
```

The resulting library will be in `build/lib/Release/websockets_static.lib`

### Linux
```bash
git clone https://github.com/warmcat/libwebsockets.git
cd libwebsockets
mkdir build
cd build
cmake .. -DLWS_WITH_STATIC=ON -DLWS_WITHOUT_TESTAPPS=ON
make
```

The resulting library will be in `build/lib/libwebsockets.a`

## Verifying the Setup

After adding all the files:

1. Verify the directory structure:
```
Mods/SML/ThirdParty/
├── include/
│   └── libwebsockets.h (and any other headers)
├── Win64/
│   └── websockets.lib
└── Linux/
    └── libwebsockets.a
```

2. Try building your project:
```bash
# Using Unreal Engine Build Tool
.\ue\Engine\Build\BatchFiles\Build.bat FactoryEditor Win64 Development -project="path\to\FactoryGame.uproject"
```

3. If you get linker errors about undefined symbols, you may need to:
   - Add additional system libraries
   - Build libwebsockets with different options
   - Check that the library is compatible with your compiler

## Troubleshooting

### "websockets.lib not found" error
- Make sure the file is named exactly `websockets.lib` (or `libwebsockets.a` on Linux)
- Check that the file is in the correct directory

### Linker errors about undefined symbols
- You may need additional system libraries
- Check the libwebsockets documentation for dependencies
- On Windows, additional libraries like `ws2_32.lib` are automatically included

### Header file errors
- Make sure you've replaced the placeholder header with the actual one
- Check that all required header files are in the include directory

## Need Help?

- Check the libwebsockets documentation: https://libwebsockets.org/
- Review the build configuration in `Mods/SML/Source/SML/SML.Build.cs`
- Look at how other third-party libraries (funchook, AssemblyAnalyzer) are integrated

## Additional Notes

- The include path is already configured (line 49 of SML.Build.cs)
- Platform-specific configuration is handled automatically
- System dependencies are already included in the build configuration
