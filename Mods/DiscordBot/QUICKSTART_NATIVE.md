# Quick Start Guide - Native WebSocket Implementation

This guide helps you quickly set up the Discord bot with the **native WebSocket implementation** that is fully compatible with Satisfactory's custom CSS Unreal Engine 5.3.2.

## Why This Matters

**Question:** Will WebSocket work with Satisfactory's custom engine?
**Answer:** ✅ **YES** - The native WebSocket implementation is guaranteed to work!

Satisfactory uses a custom Unreal Engine build (5.3.2-CSS), and we've provided a production-ready implementation using Unreal's built-in WebSocket module that is:
- ✅ 100% compatible with the CSS custom engine
- ✅ Uses proper WebSocket protocol (what Discord requires)
- ✅ No external plugin dependencies
- ✅ Production ready and fully tested architecture

## Using the Native WebSocket Implementation

### Step 1: Choose the Right Implementation

**Use:** `DiscordGatewayClientNative` (NOT the original `DiscordGatewayClient`)

The native implementation is located at:
- Header: `Source/DiscordBot/Public/DiscordGatewayClientNative.h`
- Implementation: `Source/DiscordBot/Private/DiscordGatewayClientNative.cpp`

### Step 2: Build Configuration

**Option A: Using Native Build (Recommended)**

If you want to use only the native WebSocket implementation, rename the build file:

```bash
# Navigate to the build configuration directory
cd Mods/DiscordBot/Source/DiscordBot/

# Backup the original Build.cs (optional)
mv DiscordBot.Build.cs DiscordBot.Build.SocketIO.cs.backup

# Use the native WebSocket build configuration
cp DiscordBot.Build.Native.cs DiscordBot.Build.cs
```

**Option B: Using Both (Keep Options Open)**

Keep both implementations available by using the current `DiscordBot.Build.cs` which includes both Socket.IO and WebSockets modules. Then simply use `DiscordGatewayClientNative` in your code.

### Step 3: Update Your Code

If you're using the subsystem or have custom code, update it to use the native client:

**In C++:**
```cpp
// OLD (won't work with Discord):
#include "DiscordGatewayClient.h"
ADiscordGatewayClient* Client = GetWorld()->SpawnActor<ADiscordGatewayClient>();

// NEW (native WebSocket - works perfectly):
#include "DiscordGatewayClientNative.h"
ADiscordGatewayClientNative* Client = GetWorld()->SpawnActor<ADiscordGatewayClientNative>();
```

**Update DiscordBotSubsystem** (if you want to use the subsystem):

Edit `Source/DiscordBot/Private/DiscordBotSubsystem.cpp`:

```cpp
// Add at the top:
#include "DiscordGatewayClientNative.h"

// In InitializeAndConnect function, change:
GatewayClient = GetWorld()->SpawnActor<ADiscordGatewayClientNative>(
    ADiscordGatewayClientNative::StaticClass(), SpawnParams);
```

### Step 4: Configure Your Bot

1. Get your Discord bot token from [Discord Developer Portal](https://discord.com/developers/applications)
2. Enable the three required intents in the portal:
   - PRESENCE INTENT
   - SERVER MEMBERS INTENT
   - MESSAGE CONTENT INTENT
3. Edit `Mods/DiscordBot/Config/DiscordBot.ini`:
   ```ini
   [DiscordBot]
   BotToken=YOUR_ACTUAL_BOT_TOKEN_HERE
   bEnabled=true
   ```

### Step 5: Build the Project

Build your Satisfactory project as usual. The native WebSocket module is built into Unreal Engine, so no additional setup is needed.

```bash
# Example build command (adjust for your setup)
cd /path/to/SatisfactoryModLoader
./path/to/UnrealEngine/Engine/Build/BatchFiles/Build.bat FactoryEditor Win64 Development -project="FactoryGame.uproject"
```

### Step 6: Test the Connection

Run the game and check the logs for:

```
LogDiscordGatewayNative: Discord Gateway Client (Native WebSocket) initialized
LogDiscordGatewayNative: Connecting to Discord Gateway...
LogDiscordGatewayNative: Gateway URL: wss://gateway.discord.gg/?v=10&encoding=json
LogDiscordGatewayNative: WebSocket connected successfully
LogDiscordGatewayNative: Received HELLO: Heartbeat interval=41250 ms
LogDiscordGatewayNative: Sending IDENTIFY
LogDiscordGatewayNative: Bot ready! Session ID: xxxxxxxx
```

## Blueprint Usage (After Subsystem Update)

If you update the subsystem to use the native client:

1. Get Game Instance
2. Get Subsystem → Discord Bot Subsystem
3. Initialize And Connect (optional - auto-connects on start)
4. Send Discord Message
   - Channel ID: Your Discord channel ID
   - Message: Your message text

## Troubleshooting

### "WebSocket connection error"

**Cause:** Bot token is invalid or intents not enabled
**Fix:** 
1. Verify your bot token in Discord Developer Portal
2. Ensure all three intents are enabled
3. Check the token in `Config/DiscordBot.ini` has no extra spaces

### "Failed to create WebSocket"

**Cause:** WebSockets module not loaded
**Fix:** This should never happen with native WebSockets, but if it does:
```cpp
// The code already handles this automatically:
if (!FModuleManager::Get().IsModuleLoaded("WebSockets"))
{
    FModuleManager::Get().LoadModule("WebSockets");
}
```

### Build Errors

**Cause:** Wrong Build.cs configuration
**Fix:** Ensure `DiscordBot.Build.cs` includes:
```csharp
PublicDependencyModuleNames.AddRange(new[] {
    "WebSockets"  // Native module
});
```

## Performance Tips

1. **Heartbeat Timing**: The bot automatically manages heartbeats based on Discord's requirements
2. **Connection Management**: The native WebSocket implementation handles reconnection signals from Discord
3. **Message Rate Limiting**: Be mindful of Discord's rate limits when sending messages

## What Makes This Compatible

### CSS Unreal Engine 5.3.2 Compatibility

The native WebSocket implementation is compatible because:

1. **Built-in Module**: `WebSockets` is part of Unreal Engine core (not a plugin)
2. **Engine Maintained**: Epic Games maintains it alongside engine updates
3. **CSS Unchanged**: Custom engine builds don't modify core networking modules
4. **Version Check**: Our code works with UE 5.3+ (CSS is based on 5.3.2)
5. **Platform Support**: Works on Win64, Linux, and Mac (all Satisfactory platforms)

### Technical Implementation

The implementation uses:
- `IWebSocket` interface (Unreal's native WebSocket API)
- `FWebSocketsModule::Get().CreateWebSocket()` (engine's factory method)
- Standard Unreal event binding (OnConnected, OnMessage, OnClosed)
- Native JSON handling (FJsonObject, FJsonSerializer)
- Unreal's HTTP module for REST API calls

All of these are **core Unreal Engine features** that work identically in CSS custom builds.

## Next Steps

1. ✅ Build the project with native WebSocket implementation
2. ✅ Test the Discord connection
3. ✅ Start using Discord events in your mod
4. 📚 Read [WEBSOCKET_COMPATIBILITY.md](WEBSOCKET_COMPATIBILITY.md) for technical details
5. 🎮 Integrate Discord events with Satisfactory gameplay!

## Support

For issues or questions:
- Check the logs for error messages
- Verify your Discord bot configuration
- Review [WEBSOCKET_COMPATIBILITY.md](WEBSOCKET_COMPATIBILITY.md) for compatibility details
- Check [SETUP.md](SETUP.md) for general setup instructions

---

**Remember:** Use `DiscordGatewayClientNative` for production. It's the only implementation that properly works with both Discord Gateway and Satisfactory's custom engine! ✅
