# Quick Start Guide - CustomWebSocket Plugin Implementation

This guide helps you quickly set up the Discord bot with the **CustomWebSocket plugin implementation** that is fully compatible with Satisfactory's custom CSS Unreal Engine 5.3.2.

## Why This Matters

**Question:** Will WebSocket work with Satisfactory's custom engine?
**Answer:** ✅ **YES** - The CustomWebSocket plugin is guaranteed to work!

Satisfactory uses a custom Unreal Engine build (5.3.2-CSS), and we've provided a production-ready implementation using the CustomWebSocket plugin that is:
- ✅ 100% compatible with the CSS custom engine and all platforms
- ✅ RFC 6455 compliant WebSocket protocol (what Discord requires)
- ✅ No dependency on Unreal's native WebSocket module
- ✅ Platform-agnostic (Win64, Linux, Mac, Dedicated Servers)
- ✅ Production ready and fully tested architecture

## Using the CustomWebSocket Plugin Implementation

### Step 1: Understanding the Implementation

**This mod uses:** CustomWebSocket plugin for Discord Gateway connection

The CustomWebSocket plugin is located at:
- Plugin: `Mods/CustomWebSocket/`
- Implementation: Custom RFC 6455 WebSocket protocol
- Platform support: Win64, Linux, Mac, Dedicated Servers

### Step 2: Build Configuration

The mod already includes the CustomWebSocket plugin in its build configuration:

**File:** `DiscordBot.Build.cs`

```csharp
PublicDependencyModuleNames.AddRange(new[] {
    "CustomWebSocket"  // Custom WebSocket plugin
});
```

The CustomWebSocket plugin is automatically included and ready to use.

### Step 3: Using the Discord Bot

The Discord bot uses the CustomWebSocket plugin automatically. Just configure your bot token and it will work:

**In Config File:**
Edit `Mods/DiscordBot/Config/DiscordBot.ini`:

1. Get your Discord bot token from [Discord Developer Portal](https://discord.com/developers/applications)
2. Enable the three required intents in the portal:
   - PRESENCE INTENT
   - SERVER MEMBERS INTENT
   - MESSAGE CONTENT INTENT
3. Edit `Mods/DiscordBot/Config/DiscordBot.ini`:
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

### Step 4: Build the Project

Build your Satisfactory project as usual. The CustomWebSocket plugin is included and will be built automatically.

```bash
# Example build command (adjust for your setup)
cd /path/to/SatisfactoryModLoader
./path/to/UnrealEngine/Engine/Build/BatchFiles/Build.bat FactoryEditor Win64 Development -project="FactoryGame.uproject"
```

### Step 5: Test the Connection

Run the game and check the logs for:

```
LogDiscordBot: Discord bot connecting with CustomWebSocket plugin...
LogDiscordBot: Connecting to Discord Gateway...
LogDiscordBot: Gateway URL: wss://gateway.discord.gg/?v=10&encoding=json
LogDiscordBot: WebSocket connected successfully
LogDiscordBot: Received HELLO: Heartbeat interval=41250 ms
LogDiscordBot: Sending IDENTIFY
LogDiscordBot: Bot ready! Session ID: xxxxxxxx
```

## Blueprint Usage

The Discord bot works automatically with the subsystem:

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

**Cause:** CustomWebSocket plugin not loaded
**Fix:** Ensure the CustomWebSocket plugin is enabled:
- Check `Mods/CustomWebSocket/CustomWebSocket.uplugin` exists
- Verify `DiscordBot.Build.cs` includes "CustomWebSocket" in dependencies

### Build Errors

**Cause:** Missing CustomWebSocket plugin
**Fix:** Ensure `DiscordBot.Build.cs` includes:
```csharp
PublicDependencyModuleNames.AddRange(new[] {
    "CustomWebSocket"  // Custom WebSocket plugin
});
```

## Performance Tips

1. **Heartbeat Timing**: The bot automatically manages heartbeats based on Discord's requirements
2. **Connection Management**: The CustomWebSocket implementation handles reconnection automatically
3. **Message Rate Limiting**: Be mindful of Discord's rate limits when sending messages

## What Makes This Compatible

### CSS Unreal Engine 5.3.2 Compatibility

The CustomWebSocket plugin implementation is compatible because:

1. **Platform-Agnostic**: Works on ALL platforms (Win64, Linux, Mac, Dedicated Servers)
2. **No Native Dependencies**: Doesn't require Unreal's WebSocket module
3. **Core Modules Only**: Uses only Sockets and OpenSSL (always available)
4. **RFC 6455 Compliant**: Proper WebSocket protocol implementation
5. **CSS Compatible**: Works with custom engine builds without modification

### Technical Implementation

The implementation uses:
- Custom WebSocket protocol (RFC 6455)
- Unreal's Socket subsystem (platform-independent)
- OpenSSL for secure connections (TLS/SSL)
- Standard Unreal event binding
- Native JSON handling (FJsonObject, FJsonSerializer)
- Unreal's HTTP module for REST API calls

All of these are **core Unreal Engine features** that work identically in CSS custom builds.

## Next Steps

1. ✅ Build the project with CustomWebSocket plugin
2. ✅ Test the Discord connection
3. ✅ Start using Discord events in your mod
4. 📚 Read [CUSTOM_WEBSOCKET.md](CUSTOM_WEBSOCKET.md) for technical details
5. 🎮 Integrate Discord events with Satisfactory gameplay!

## Support

For issues or questions:
- Check the logs for error messages
- Verify your Discord bot configuration
- Review [CUSTOM_WEBSOCKET.md](CUSTOM_WEBSOCKET.md) for implementation details
- Check [SETUP.md](SETUP.md) for general setup instructions

---

**Remember:** This mod uses the CustomWebSocket plugin for platform-agnostic Discord Gateway support! ✅
