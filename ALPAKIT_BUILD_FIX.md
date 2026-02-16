# Alpakit Build Fix - ExitCode 6 Resolution

## Problem
The DiscordChatBridge mod was failing to package with Alpakit, producing ExitCode=6:
```
[Package DiscordChatBridge] BUILD FAILED
[Package DiscordChatBridge] Packaging failed with ExitCode=6
```

ExitCode 6 indicates a compilation error during the build process.

## Root Cause
The code in `DiscordGateway.cpp` was using an incorrect pattern to access the WebSockets module:
```cpp
WebSocket = FWebSocketsModule::Get().CreateWebSocket(GATEWAY_URL, TEXT(""));
```

This pattern (`FWebSocketsModule::Get()`) is not the correct way to access non-singleton modules in Unreal Engine 5 and causes a compilation error.

## Solution
Changed the module access pattern to use `FModuleManager::LoadModuleChecked`:
```cpp
FWebSocketsModule& WebSocketsModule = FModuleManager::LoadModuleChecked<FWebSocketsModule>("WebSockets");
WebSocket = WebSocketsModule.CreateWebSocket(GATEWAY_URL, TEXT(""));
```

This is the standard UE5 pattern for accessing modules that don't provide a `Get()` method.

## Technical Details
- **File Modified**: `Mods/DiscordChatBridge/Source/DiscordChatBridge/Private/DiscordGateway.cpp`
- **Line**: 89
- **Change Type**: Module access pattern correction
- **Impact**: Fixes compilation error, allows Alpakit packaging to succeed

## Why This Works
1. `FModuleManager::LoadModuleChecked<T>()` is the correct way to get a typed reference to a module in UE5
2. It ensures the module is loaded and returns a valid reference
3. It's consistent with Unreal Engine's module system architecture
4. The module is already checked and loaded earlier in the same function (lines 60-77)

## Additional Changes
- Updated error message on line 96 to reflect the corrected code pattern

## Verification
The fix:
- ✅ Uses standard UE5 module access patterns
- ✅ Maintains all existing error handling
- ✅ No functional changes to mod behavior
- ✅ All includes are already in place
- ✅ Follows existing code style

## References
- Unreal Engine Module System Documentation
- FModuleManager API Reference
- Previous fix for ModSubsystems issue (see `COMPILATION_FIX.md`)
