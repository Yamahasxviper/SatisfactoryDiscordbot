# WebSocket Module Not Visible? Here's How to Check

> ⚠️ **NOTE**: This mod uses the **CustomWebSocket plugin** for Discord Gateway connection, NOT Unreal's native WebSocket module. This guide is for reference only if you want to check Unreal's native WebSocket availability. **The CustomWebSocket plugin does not require Unreal's WebSocket module** and works independently.

## Quick Answer

If you can't see the WebSocket module in your CSS Unreal Engine 5.3.2 build, **don't worry!** This mod uses the CustomWebSocket plugin which doesn't depend on Unreal's native WebSocket module.

However, if you're curious about native WebSocket availability, the module might still be there - it's just not immediately visible.

## Immediate Solution: Use the Verification Tool

We've created a tool that will tell you exactly what's going on:

### Method 1: In Unreal Editor (Easiest)

1. Open your Satisfactory project in Unreal Editor
2. Open any level
3. In the **Place Actors** panel, search for `WebSocketModuleVerifier`
4. Drag it into your level
5. Click **Play** (PIE - Play In Editor)
6. Check the **Output Log** - it will show you exactly what's available

### Method 2: Blueprint

1. Create a new Blueprint (or use existing)
2. Add this node chain:
   ```
   Event BeginPlay
   → Spawn Actor from Class (WebSocketModuleVerifier)
   → Call Function: Run Full Verification
   ```
3. Run and check Output Log

### Method 3: C++ Quick Check

Add this to any C++ file:

```cpp
#include "WebSocketModuleVerifier.h"

void YourFunction()
{
    AWebSocketModuleVerifier* Verifier = GetWorld()->SpawnActor<AWebSocketModuleVerifier>();
    Verifier->RunFullVerification();
    // Check Output Log for results
}
```

## What to Look For

### ✅ Success Output:
```
✅ RESULT: WebSocket module is AVAILABLE and WORKING
You can use DiscordGatewayClientNative for Discord integration!
```

**Great news!** The module is available. You can use the Discord bot.

### ❌ Failure Output:
```
❌ RESULT: WebSocket module is NOT available
See troubleshooting guide in WEBSOCKET_TROUBLESHOOTING.md
```

**Don't worry!** Follow the troubleshooting guide to fix it.

## Why You Might Not "See" It

The WebSocket module is a **Runtime Plugin**, not a source module. This means:

1. **It's in a different location**: `Engine/Plugins/Runtime/WebSockets/`
   - Not in `Engine/Source/Runtime/`
   - Might not show up in normal module listings

2. **It's loaded on-demand**: The module doesn't load until it's needed
   - Our code explicitly loads it when connecting
   - Won't appear in "loaded modules" until used

3. **It's a plugin, not core**: Plugins work differently
   - Core modules are always visible
   - Plugins are only visible when active

## Most Common Scenario

**Module IS available, just not "visible"** in the way you expect.

The verification tool will confirm this by:
- ✅ Finding the module registration
- ✅ Loading the module successfully
- ✅ Creating a test WebSocket

If all three succeed, **the module works fine!**

## Where Is It Actually Located?

Check these locations in your engine:

```
YourEngine/
├── Engine/
│   ├── Plugins/
│   │   └── Runtime/
│   │       └── WebSockets/              ← HERE!
│   │           ├── WebSockets.uplugin
│   │           └── Source/
│   │               └── WebSockets/
│   │                   ├── Public/
│   │                   │   ├── IWebSocket.h
│   │                   │   └── WebSocketsModule.h
│   │                   └── Private/
│   └── Source/
│       └── Runtime/                     ← NOT here
```

## Quick Verification Steps

1. **Navigate to engine directory**:
   ```bash
   cd /path/to/your/UnrealEngine
   ```

2. **Check if WebSockets plugin exists**:
   ```bash
   ls Engine/Plugins/Runtime/WebSockets/
   ```

3. **If folder exists**: Module is available! ✅
4. **If folder missing**: Module not included in build ❌

## What Our Code Does Automatically

The `DiscordGatewayClientNative` automatically:

```cpp
// 1. Check if module is loaded
if (!FModuleManager::Get().IsModuleLoaded("WebSockets"))
{
    // 2. Load it if needed
    FModuleManager::Get().LoadModule("WebSockets");
}

// 3. Create WebSocket
WebSocket = FWebSocketsModule::Get().CreateWebSocket(URL);

// 4. Show clear error if fails
if (!WebSocket.IsValid())
{
    UE_LOG(Error, TEXT("Failed to create WebSocket"));
    // Plus helpful troubleshooting info
}
```

So even if you don't "see" the module, our code will:
- Find it automatically
- Load it automatically
- Tell you if there's a problem

## Bottom Line

**Run the verification tool first!** It will tell you:
- ✅ If WebSocket is available (most likely yes)
- ❌ If it's truly missing (less likely)
- 📋 Exactly what to do next

## Still Concerned?

Read these in order:

1. **This file** (you are here) - Quick verification
2. **[WEBSOCKET_COMPATIBILITY.md](WEBSOCKET_COMPATIBILITY.md)** - Technical details
3. **[WEBSOCKET_TROUBLESHOOTING.md](WEBSOCKET_TROUBLESHOOTING.md)** - Detailed troubleshooting
4. **[QUICKSTART_NATIVE.md](QUICKSTART_NATIVE.md)** - Getting started guide

## TL;DR

1. ✅ **Run `WebSocketModuleVerifier`** - It will check everything
2. ✅ **Check the Output Log** - It will tell you the status
3. ✅ **Most likely**: Module is available, just not "visible" to you
4. ✅ **If available**: Use `DiscordGatewayClientNative` - it will work!
5. ❌ **If truly missing**: See `WEBSOCKET_TROUBLESHOOTING.md`

---

**The verification tool is your friend!** Run it and it will tell you exactly what's going on. No guessing needed! 🚀
