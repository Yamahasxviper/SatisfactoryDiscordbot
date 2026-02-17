# WebSocket Compatibility with Satisfactory Custom Engine

## Overview

This document addresses the compatibility of WebSocket implementations with Satisfactory's custom Unreal Engine build (5.3.2-CSS).

## Engine Version

- **Satisfactory Engine**: Unreal Engine 5.3.2-CSS (Coffee Stain Studios custom build)
- **Base Engine**: Unreal Engine 5.3.2
- **Custom Modifications**: CSS (Coffee Stain Studios) customizations

## WebSocket Implementation Options

### Option 1: SocketIOClient Plugin (Current)

**Status**: ⚠️ **Partially Compatible** - Not Recommended for Discord Gateway

**Details:**
- **Plugin Version**: 2.11.0
- **Supports**: UE 5.7+ (documented), but includes backwards compatibility checks
- **Compatibility Check**: `SocketIOLib.Build.cs` includes version check for UE 5.3+
  ```csharp
  if ((Target.Version.MajorVersion == 5 && Target.Version.MinorVersion >= 3) ||
      Target.Version.MajorVersion > 5)
  {
      PublicDefinitions.Add("ASIO_HAS_STD_INVOKE_RESULT=1");
  }
  ```

**Issues:**
1. **Protocol Mismatch**: Discord Gateway uses **native WebSocket** (RFC 6455), NOT Socket.IO protocol
2. **Socket.IO vs WebSocket**: 
   - Socket.IO is a library built on top of WebSocket with additional features (rooms, namespaces, etc.)
   - Discord Gateway requires pure WebSocket connection
   - Using Socket.IO for Discord will NOT work properly
3. **Dependencies**: Adds unnecessary dependencies (asio, rapidjson, websocketpp) for a simple WebSocket connection

**Verdict**: ❌ **Not suitable for Discord Gateway** despite being compatible with the engine

### Option 2: Native Unreal WebSocket Module (Recommended)

**Status**: ✅ **Fully Compatible** - Recommended

**Details:**
- **Module**: `WebSockets` (built into Unreal Engine)
- **Availability**: Standard module in all UE 5.x versions including custom builds
- **API**: `IWebSocket` interface
- **CSS Compatibility**: ✅ Works with custom CSS engine builds

**Advantages:**
1. ✅ **Native to Engine**: Guaranteed compatibility with any UE 5.3.x build
2. ✅ **Protocol Match**: Implements pure WebSocket (RFC 6455) - exactly what Discord needs
3. ✅ **No External Dependencies**: Uses engine's built-in WebSocket implementation
4. ✅ **Lightweight**: Minimal overhead
5. ✅ **Well Tested**: Used extensively in Unreal Engine networking

**Implementation:**
```cpp
// Include
#include "IWebSocket.h"
#include "WebSocketsModule.h"

// Create WebSocket
TSharedPtr<IWebSocket> WebSocket = FWebSocketsModule::Get().CreateWebSocket(URL);

// Bind events
WebSocket->OnConnected().AddLambda([]() { /* Connected */ });
WebSocket->OnMessage().AddLambda([](const FString& Message) { /* Handle message */ });
WebSocket->OnClosed().AddLambda([](int32 StatusCode, const FString& Reason, bool bWasClean) { /* Closed */ });

// Connect
WebSocket->Connect();

// Send
WebSocket->Send(Message);
```

## Compatibility Matrix

| Component | SocketIOClient | Native WebSockets |
|-----------|---------------|-------------------|
| **UE 5.3.2 Compatibility** | ⚠️ Yes (with workarounds) | ✅ Yes (native) |
| **CSS Custom Build** | ⚠️ Unknown | ✅ Yes |
| **Discord Gateway Protocol** | ❌ No (wrong protocol) | ✅ Yes (correct protocol) |
| **Build Complexity** | ⚠️ Complex (third-party libs) | ✅ Simple (built-in) |
| **Maintenance** | ⚠️ External plugin updates | ✅ Engine maintained |
| **Dependencies** | ❌ Many (asio, websocketpp, etc.) | ✅ None (built-in) |

## Recommendation

### For Discord Gateway Integration: Use Native WebSocket Module

**Why:**
1. **Protocol Match**: Discord requires WebSocket, not Socket.IO
2. **Guaranteed Compatibility**: Native module works with ALL UE 5.3.x builds including CSS custom builds
3. **Simpler Dependencies**: No third-party plugins required
4. **Better Performance**: Lighter weight, fewer abstraction layers
5. **Future-Proof**: Maintained by Epic Games alongside engine updates

### Implementation Status

**Current Implementation**: 
- Uses SocketIOClient (incorrect for Discord Gateway)
- Includes placeholder code noting the issue
- Will not work for actual Discord connections

**Required Changes**:
- Replace SocketIOClient dependency with native WebSockets module
- Reimplement `DiscordGatewayClient` using `IWebSocket` interface
- Remove SocketIOClient/SocketIOLib/SIOJson dependencies from Build.cs

## Testing with Custom Engine

### Prerequisites
- Satisfactory Unreal Engine 5.3.2-CSS installed
- Mod building environment set up
- Discord bot token

### Verification Steps

1. **Check WebSocket Module Availability**:
   ```cpp
   FWebSocketsModule& WebSocketsModule = FModuleManager::LoadModuleChecked<FWebSocketsModule>("WebSockets");
   UE_LOG(LogTemp, Log, TEXT("WebSockets module loaded successfully"));
   ```

2. **Test Connection**:
   - Create WebSocket with Discord Gateway URL
   - Verify connection establishment
   - Check HELLO message reception
   - Validate IDENTIFY and HEARTBEAT

3. **Build Test**:
   - Build mod with CSS engine
   - Verify no compilation errors
   - Check runtime behavior

## Migration Path

### Phase 1: Update Dependencies (Low Risk)
1. Update `DiscordBot.Build.cs` to use native WebSockets
2. Remove SocketIO dependencies
3. Verify build

### Phase 2: Reimplement Gateway Client (Medium Risk)
1. Replace USocketIOClientComponent with IWebSocket
2. Implement proper WebSocket message handling
3. Update event binding

### Phase 3: Test & Validate (High Priority)
1. Test with Discord Gateway
2. Verify all opcodes work correctly
3. Validate heartbeat mechanism

## Conclusion

**Answer to "Will WebSocket work with this custom engine?"**

✅ **YES** - Native WebSocket module will work perfectly with CSS Unreal Engine 5.3.2

❌ **NO** - SocketIOClient is not appropriate for Discord Gateway (protocol mismatch)

**Action Required**: Migrate from SocketIOClient to native Unreal WebSockets module for proper Discord Gateway integration.

## Additional Resources

- [Unreal Engine WebSockets Documentation](https://docs.unrealengine.com/5.3/en-US/API/Runtime/WebSockets/)
- [Discord Gateway Documentation](https://discord.com/developers/docs/topics/gateway)
- [WebSocket RFC 6455](https://datatracker.ietf.org/doc/html/rfc6455)
