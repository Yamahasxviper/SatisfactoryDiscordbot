# WebSocket FAQ - Frequently Asked Questions

This document addresses common questions and misconceptions about WebSockets in the Satisfactory Mod Loader and Discord Chat Bridge.

---

## General Questions

### Q: Do I need to add WebSockets to this project?

**A: No! WebSockets are already included.**

The WebSocket plugin is located at `Plugins/WebSockets/` in this repository and builds automatically with the project. You don't need to add anything.

**Verify yourself:**
```bash
ls -la Plugins/WebSockets/
./scripts/verify_websocket_build_compatibility.sh
```

---

### Q: Does the SML custom Unreal Engine have WebSockets?

**A: Yes, but it doesn't matter!**

The WebSocket plugin in this project is a **project plugin**, not an engine plugin. This means:

1. **It's part of this repository**, not the engine
2. **It builds from source** during project compilation
3. **It works with any UE 5.3.2 installation** (standard or CSS custom)
4. **Engine doesn't need to have WebSockets** - we provide our own

**Think of it this way:** Even if the engine doesn't have WebSockets (which is rare), this project provides its own WebSocket implementation that works perfectly.

---

### Q: How do I know if WebSockets are working in my build?

**A: Check the build output.**

When you build the project, you should see:

```
Building WebSockets (Win64, Development)...
  Compiling WebSocketsModule.cpp
  ...
  UnrealEditor-WebSockets.dll

Building DiscordChatBridge (Win64, Development)...
  WITH_WEBSOCKETS_SUPPORT=1
  Compiling DiscordGateway.cpp
  ...
  UnrealEditor-DiscordChatBridge.dll
```

**Key indicator:** `WITH_WEBSOCKETS_SUPPORT=1` in DiscordChatBridge build

**Alternative check:**
```bash
./scripts/verify_websocket_build_compatibility.sh
# Should show: ✅ BUILD COMPATIBLE (26/26 checks passed)
```

---

### Q: What happens if WebSockets are somehow missing?

**A: The mod still works, just without Gateway features.**

The build system is smart:
- **If WebSockets found**: Full functionality (Gateway, presence, real-time updates)
- **If WebSockets not found**: REST API only (basic chat, no presence)

The mod is designed to gracefully degrade. Your server will work either way!

**In practice:** WebSockets are always present in this repository, so you'll always have full functionality.

---

## Configuration Questions

### Q: I configured the bot but don't see presence/activity. Why?

**A: Check these settings:**

1. **Discord Developer Portal** - Enable intents:
   - ✅ MESSAGE CONTENT INTENT
   - ✅ PRESENCE INTENT (required for Gateway)
   - ✅ SERVER MEMBERS INTENT (optional)

2. **Config file** - Enable Gateway:
   ```ini
   UseGatewayForPresence=true
   EnableBotActivity=true
   ```

3. **Check server logs** for Gateway connection:
   ```
   ✅ Good: "Gateway connected successfully"
   ❌ Bad: "Failed to connect to gateway"
   ```

**See:** [WEBSOCKET_HOW_TO.md](WEBSOCKET_HOW_TO.md#configuration)

---

### Q: Do I need different settings for Linux vs Windows servers?

**A: No, configuration is identical.**

WebSockets work the same on both platforms:
- ✅ Windows: Uses WinHttp implementation
- ✅ Linux: Uses libwebsockets implementation
- ✅ Configuration: Identical for both

**Platform differences are handled automatically by the build system.**

---

## Build Questions

### Q: I get "Unable to find plugin 'WebSockets'" during build. Help!

**A: This should not happen with the current code, but if it does:**

**First, verify the plugin actually exists:**
```bash
ls -la Plugins/WebSockets/WebSockets.uplugin
# Should show the file, not "No such file"
```

**If the file exists:**
1. Clean your build:
   ```bash
   rm -rf Intermediate/ Saved/ *.sln *.vcxproj*
   ```

2. Regenerate project files:
   - Right-click `FactoryGame.uproject`
   - Select "Generate Visual Studio project files"

3. Build again

**If the file doesn't exist:**
- You may have an incomplete clone of the repository
- Re-clone the repository: `git clone https://github.com/Yamahasxviper/SatisfactoryDiscordbot.git`

**Still failing?** Check [TROUBLESHOOTING.md](TROUBLESHOOTING.md#unable-to-find-plugin-websockets)

---

### Q: The build says "WebSockets plugin NOT FOUND in any location"

**A: Check the build output carefully.**

The DiscordChatBridge.Build.cs checks **7 different locations** for WebSockets:

```
[DiscordChatBridge] Build.cs: Checking for WebSockets plugin...
  Path 1 (Engine/Runtime): <path> - NOT FOUND
  Path 2 (Engine/Experimental): <path> - NOT FOUND
  Path 3 (Project/Plugins): <path> - FOUND ✓
```

**If Path 3 shows "FOUND"**: You're good! The warning is just for info.

**If all paths show "NOT FOUND"**: 
- Verify: `ls -la Plugins/WebSockets/`
- This indicates a file system or repository issue
- Re-clone the repository

---

### Q: Can I use the WebSockets plugin from the Unreal Engine installation instead?

**A: You can, but it's not recommended.**

This project provides its own WebSocket plugin for several reasons:
1. **Ensures compatibility** with all UE 5.3.2 installations
2. **Guarantees availability** - no dependency on engine installation
3. **Includes custom build configuration** for Satisfactory
4. **Pre-compiled third-party libraries** included

**Bottom line:** Use the provided plugin. It's already configured and tested.

---

## Technical Questions

### Q: What's the difference between Gateway and REST API?

**A: Gateway uses WebSockets for real-time communication.**

| Feature | Gateway (WebSocket) | REST API Only |
|---------|-------------------|---------------|
| Chat messages | ✅ Supported | ✅ Supported |
| Bot presence | ✅ "Online" status | ❌ Not available |
| Activity status | ✅ "Playing with X players" | ❌ Not available |
| Real-time updates | ✅ Instant | ⚠️ Polling (slower) |
| Reconnection | ✅ Automatic | ⚠️ Manual restart |
| Resource usage | ⚠️ Persistent connection | ✅ Lower (no persistent connection) |

**Recommendation:** Use Gateway (default) for best experience.

---

### Q: Does WebSocket support secure connections (WSS)?

**A: Yes, all connections are secure.**

- ✅ Discord Gateway uses `wss://` (WebSocket Secure)
- ✅ SSL/TLS encryption enabled by default
- ✅ Uses OpenSSL for encryption
- ✅ Certificate validation enabled

**You don't need to configure anything - it's secure by default.**

---

### Q: What ports does WebSocket use?

**A: Discord Gateway uses standard HTTPS port 443.**

The WebSocket connection to Discord uses:
- **Protocol:** WSS (WebSocket Secure)
- **Port:** 443 (standard HTTPS)
- **Endpoint:** `wss://gateway.discord.gg`

**Firewall configuration:**
- Allow outbound connections to `*.discord.gg` on port 443
- No inbound connections required (client initiates)

---

### Q: How much bandwidth does the Gateway connection use?

**A: Very little - less than 1 KB/s average.**

Typical usage:
- **Initial connection:** ~5 KB (handshake)
- **Heartbeat:** ~100 bytes every 41 seconds
- **Chat messages:** ~500 bytes per message
- **Activity updates:** ~200 bytes per update

**Total:** Usually under 500 KB per day for a moderately active server.

---

## Platform-Specific Questions

### Q: Do WebSockets work on Linux dedicated servers?

**A: Yes, fully supported!**

Linux servers use the libwebsockets implementation:
- ✅ Tested on x86_64 (standard Linux servers)
- ✅ Tested on ARM64 (e.g., Raspberry Pi servers)
- ✅ No additional configuration required
- ✅ Same functionality as Windows

**See:** [WEBSOCKET_LINUX_TESTING.md](WEBSOCKET_LINUX_TESTING.md)

---

### Q: Do I need to install any system libraries on Linux?

**A: No, everything is included.**

The pre-compiled libwebsockets library is provided in:
```
ThirdParty/libWebSockets/libwebsockets/lib/Unix/
├── x86_64-unknown-linux-gnu/    ← Standard Linux
└── aarch64-unknown-linux-gnueabi/  ← ARM Linux
```

**These are statically linked** during build - no runtime dependencies.

---

### Q: Can I use this on Windows Server?

**A: Yes, fully supported!**

Windows servers use the WinHttp implementation:
- ✅ Windows Server 2012 R2 and later
- ✅ Windows 8.1 and later (desktop)
- ✅ Native Windows API (no third-party dependencies)
- ✅ Enterprise-grade reliability

**See:** [WEBSOCKET_WINDOWS_SERVER_COMPATIBILITY.md](WEBSOCKET_WINDOWS_SERVER_COMPATIBILITY.md)

---

## Advanced Questions

### Q: Can I build without WebSockets if I don't want Gateway features?

**A: Yes, but not recommended.**

If you somehow remove the WebSocket plugin:
1. Build will succeed with `WITH_WEBSOCKETS_SUPPORT=0`
2. Discord Chat Bridge will work with REST API only
3. No presence/activity features

**However:** There's no reason to do this. WebSockets are:
- ✅ Lightweight (minimal impact)
- ✅ Optional at runtime (config controlled)
- ✅ Provide better features
- ✅ More efficient than polling

**Recommendation:** Keep WebSockets, disable Gateway in config if needed.

---

### Q: How do I disable Gateway but keep WebSockets available?

**A: Set `UseGatewayForPresence=false` in your config.**

```ini
[/Script/DiscordChatBridge.DiscordConfig]
UseGatewayForPresence=false
```

This will:
- ✅ Keep WebSocket support compiled in
- ✅ Use REST API only at runtime
- ✅ Save the persistent connection overhead
- ✅ Allow re-enabling Gateway without rebuilding

**This is the recommended way to disable Gateway if you don't need it.**

---

### Q: Can I use the WebSocket plugin in my own mod?

**A: Yes! It's available to all mods.**

To use WebSockets in your mod:

1. **Add dependency in YourMod.uplugin:**
   ```json
   "Plugins": [
       {
           "Name": "WebSockets",
           "Enabled": true,
           "Optional": true
       }
   ]
   ```

2. **Add module in YourMod.Build.cs:**
   ```csharp
   PublicDependencyModuleNames.Add("WebSockets");
   ```

3. **Use in your code:**
   ```cpp
   #include "IWebSocket.h"
   #include "WebSocketsModule.h"
   
   TSharedPtr<IWebSocket> Socket = FWebSocketsModule::Get().CreateWebSocket(URL);
   ```

**Example:** See `Mods/DiscordChatBridge/Source/` for complete implementation.

---

### Q: Where can I find the WebSocket implementation source code?

**A: In multiple locations:**

**WebSocket Plugin:**
- `Plugins/WebSockets/Source/WebSockets/` - Plugin implementation
- `Plugins/WebSockets/WebSockets.Build.cs` - Build configuration

**DiscordChatBridge Usage:**
- `Mods/DiscordChatBridge/Source/DiscordChatBridge/Public/DiscordGateway.h` - Interface
- `Mods/DiscordChatBridge/Source/DiscordChatBridge/Private/DiscordGateway.cpp` - Implementation

**Third-Party Library:**
- `ThirdParty/libWebSockets/` - Pre-compiled libwebsockets

---

## Troubleshooting Questions

### Q: The server starts but Discord presence doesn't show. What's wrong?

**A: Most common causes:**

1. **Configuration not set** - Most common issue!
   - ❌ Bot token missing or incorrect
   - ❌ PRESENCE INTENT not enabled in Discord
   - ✅ See [Quick Start Guide](Mods/DiscordChatBridge/help/QUICKSTART.md)

2. **Gateway disabled in config:**
   ```ini
   UseGatewayForPresence=false  ← Change to true
   ```

3. **Firewall blocking connection:**
   - Allow outbound to `*.discord.gg` port 443

4. **Check server logs:**
   ```
   Look for: "Gateway connected successfully"
   ```

---

### Q: I see "WebSocket connection error" in logs. Help!

**A: Check these in order:**

1. **Internet connectivity:**
   ```bash
   # Can the server reach Discord?
   curl -I https://discord.com
   # Should return "HTTP/2 200"
   ```

2. **Bot token valid:**
   - Test token with Discord API
   - Regenerate if needed

3. **Discord API status:**
   - Check https://discordstatus.com
   - Wait if Discord is having issues

4. **Firewall/proxy:**
   - Allow WSS connections
   - Configure proxy if needed ([PROXY_SUPPORT.md](PROXY_SUPPORT.md))

---

### Q: Gateway was working but stopped. What happened?

**A: Common causes:**

1. **Bot token regenerated** - Update config with new token
2. **Intents disabled** - Re-enable in Discord Developer Portal
3. **Discord API changes** - Check for mod updates
4. **Network issues** - Check server connectivity
5. **Rate limiting** - Reduce message frequency

**First step:** Check server logs for error messages.

---

## Getting Help

### Q: Where can I get more help?

**A: Multiple resources available:**

**Documentation:**
- [WebSocket How-To Guide](WEBSOCKET_HOW_TO.md) - Comprehensive guide
- [Troubleshooting](TROUBLESHOOTING.md) - Common issues
- [Quick Start](Mods/DiscordChatBridge/help/QUICKSTART.md) - Getting started
- [Build Requirements](BUILD_REQUIREMENTS.md) - Build setup

**Scripts:**
```bash
# Verify build compatibility
./scripts/verify_websocket_build_compatibility.sh

# Test runtime integration
./scripts/validate_websocket_integration.sh

# Test Linux compatibility
./scripts/test_websocket_linux_compatibility.sh
```

**Community:**
- **Discord:** https://discord.gg/QzcG9nX
- **GitHub Issues:** https://github.com/Yamahasxviper/SatisfactoryDiscordbot/issues
- **Modding Docs:** https://docs.ficsit.app/

---

### Q: I have a question not answered here. Where do I ask?

**A: Post in the right place:**

1. **Build/compilation issues** → GitHub Issues
2. **Configuration/setup help** → Discord community
3. **Bug reports** → GitHub Issues (with logs)
4. **Feature requests** → GitHub Issues (with use case)
5. **General questions** → Discord community

**When asking for help, include:**
- ✅ Verification script output
- ✅ Relevant log excerpts
- ✅ Your platform (Windows/Linux)
- ✅ Your UE version
- ✅ Steps you've already tried

---

## Quick Reference

### Essential Commands

```bash
# Verify WebSockets are available
./scripts/verify_websocket_build_compatibility.sh

# Check plugin exists
ls -la Plugins/WebSockets/

# Check project configuration
grep -A 2 "WebSockets" FactoryGame.uproject

# Clean build
rm -rf Intermediate/ Saved/ *.sln *.vcxproj*
```

### Essential Files

```
Plugins/WebSockets/               ← WebSocket plugin
ThirdParty/libWebSockets/         ← Third-party libraries
FactoryGame.uproject              ← Project config (line 81-84)
Mods/DiscordChatBridge/           ← Mod using WebSockets
BUILD_REQUIREMENTS.md             ← Build documentation
WEBSOCKET_HOW_TO.md              ← Complete guide (start here!)
```

### Key Takeaways

✅ **WebSockets are included** - No need to add them  
✅ **Build automatically** - No special setup required  
✅ **Work on all platforms** - Windows, Linux, macOS  
✅ **Configuration required** - Bot token + intents  
✅ **Check documentation** - Comprehensive guides available  

---

**Still confused?** → Read [WEBSOCKET_HOW_TO.md](WEBSOCKET_HOW_TO.md) for a complete walkthrough!
