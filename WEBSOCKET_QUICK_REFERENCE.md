# WebSocket Quick Reference

> **TL;DR**: WebSockets are already included. Just build the project normally.

## Quick Check

```bash
# Are WebSockets available in my setup?
./scripts/check_websocket_status.sh
```

Expected: **✅ ALL CHECKS PASSED**

---

## Quick Facts

| Question | Answer |
|----------|--------|
| Are WebSockets included? | ✅ Yes, at `Plugins/WebSockets/` |
| Do I need to install anything? | ❌ No, everything is included |
| Will it build automatically? | ✅ Yes, builds with the project |
| What platforms are supported? | ✅ Windows, Linux, macOS, iOS, Android |
| Does it work on dedicated servers? | ✅ Yes, both Windows and Linux servers |
| Do I need special engine version? | ❌ No, works with UE 5.3.2-CSS (standard) |
| Is configuration required? | ✅ Yes, for Discord bot integration |

---

## Quick Commands

```bash
# Verify WebSocket status
./scripts/check_websocket_status.sh

# Detailed build compatibility check
./scripts/verify_websocket_build_compatibility.sh

# Test Linux compatibility
./scripts/test_websocket_linux_compatibility.sh

# Validate runtime integration
./scripts/validate_websocket_integration.sh
```

---

## Common Issues

### "I think WebSockets are missing"

**Check:**
```bash
ls -la Plugins/WebSockets/
# Should show plugin files
```

**If present:** WebSockets ARE available! Read [WEBSOCKET_HOW_TO.md](WEBSOCKET_HOW_TO.md)

### "Discord presence not working"

**Most common cause:** Configuration not set

**Fix:**
1. Set bot token in config
2. Enable PRESENCE INTENT in Discord Developer Portal
3. Set `UseGatewayForPresence=true` in config

**Details:** See [Quick Start Guide](Mods/DiscordChatBridge/help/QUICKSTART.md)

### "Build error about WebSockets"

**Try:**
```bash
# Clean build
rm -rf Intermediate/ Saved/

# Verify plugin exists
./scripts/check_websocket_status.sh
```

**Still failing?** See [TROUBLESHOOTING.md](TROUBLESHOOTING.md)

---

## Quick Start

### For Building

1. **Clone repository** (if not done)
   ```bash
   git clone https://github.com/Yamahasxviper/SatisfactoryDiscordbot.git
   cd SatisfactoryDiscordbot
   ```

2. **Verify WebSockets** (optional but recommended)
   ```bash
   ./scripts/check_websocket_status.sh
   ```

3. **Build normally**
   - Open `FactoryGame.uproject` in Unreal Editor
   - Build → Build FactoryEditor
   - WebSockets builds automatically ✅

### For Discord Integration

1. **Create Discord bot**
   - Go to https://discord.com/developers/applications
   - Create new application
   - Get bot token

2. **Enable intents**
   - Bot section → Enable:
     - ✅ MESSAGE CONTENT INTENT
     - ✅ PRESENCE INTENT

3. **Configure mod**
   Edit: `Mods/DiscordChatBridge/config/DiscordChatBridge.txt`
   ```ini
   BotToken=YOUR_TOKEN_HERE
   ChannelId=YOUR_CHANNEL_ID_HERE
   UseGatewayForPresence=true
   EnableBotActivity=true
   ```

4. **Run server**
   - Gateway connects automatically
   - Bot shows online with presence ✅

**Full instructions:** [Quick Start Guide](Mods/DiscordChatBridge/help/QUICKSTART.md)

---

## File Locations

### Plugin Files
```
Plugins/WebSockets/              ← WebSocket plugin
├── WebSockets.uplugin           ← Plugin descriptor
├── Source/                      ← Source code
└── ...

ThirdParty/libWebSockets/        ← Third-party libraries
├── libwebsockets/
│   ├── include/                 ← Headers
│   └── lib/                     ← Pre-compiled libs
└── ...
```

### Configuration Files
```
Mods/DiscordChatBridge/config/DiscordChatBridge.txt    ← Primary config (recommended)

# Runtime locations (created after first run):
Windows: %localappdata%\FactoryGame\Saved\Config\WindowsServer\DiscordChatBridge.ini
Linux:   ~/.config/Epic/FactoryGame/Saved/Config/LinuxServer/DiscordChatBridge.ini
```

### Documentation
```
WEBSOCKET_HOW_TO.md              ← Complete guide (start here!)
WEBSOCKET_FAQ.md                 ← Common questions answered
BUILD_REQUIREMENTS.md            ← Build system details
TROUBLESHOOTING.md               ← Problem solutions
README.md                        ← Project overview
```

---

## Documentation Map

```
Start here:
   └─→ WEBSOCKET_HOW_TO.md ─────→ Comprehensive guide
          ├─→ Quick status check
          ├─→ How to build
          ├─→ Configuration
          └─→ Troubleshooting

Have questions?
   └─→ WEBSOCKET_FAQ.md ────────→ 25+ common questions answered

Build issues?
   └─→ TROUBLESHOOTING.md ──────→ Common problems + solutions
   └─→ BUILD_REQUIREMENTS.md ───→ Build system details

Need Discord setup?
   └─→ Mods/DiscordChatBridge/help/QUICKSTART.md ─→ Step-by-step setup
```

---

## Platform-Specific Notes

### Windows
- Uses **WinHttp** implementation (native Windows API)
- Requires Windows 8.1+ / Server 2012 R2+
- No third-party dependencies at runtime
- Status: ✅ **Fully supported**

### Linux
- Uses **libwebsockets** implementation
- Supports x86_64 and ARM64 architectures
- Libraries included in repository
- Status: ✅ **Fully supported**

### macOS
- Uses **libwebsockets** implementation
- Libraries included in repository
- Status: ✅ **Fully supported**

---

## Getting Help

### Self-Service
1. **Run verification:**
   ```bash
   ./scripts/check_websocket_status.sh
   ```

2. **Read documentation:**
   - [WEBSOCKET_HOW_TO.md](WEBSOCKET_HOW_TO.md) - How to use
   - [WEBSOCKET_FAQ.md](WEBSOCKET_FAQ.md) - Common questions
   - [TROUBLESHOOTING.md](TROUBLESHOOTING.md) - Problem solutions

3. **Check logs:**
   - Build logs for compilation issues
   - Server logs for runtime issues

### Community Help
- **Discord:** https://discord.gg/QzcG9nX
- **GitHub Issues:** https://github.com/Yamahasxviper/SatisfactoryDiscordbot/issues
- **Modding Docs:** https://docs.ficsit.app/

**When asking for help, include:**
- Output of `./scripts/check_websocket_status.sh`
- Relevant log excerpts
- Your platform (Windows/Linux/macOS)
- Steps you've already tried

---

## Key Takeaways

### ✅ What You Have
- Complete WebSocket plugin with source code
- Pre-compiled third-party libraries for all platforms
- Automatic build integration
- Full Discord Gateway support
- Comprehensive documentation

### ❌ What You Don't Need
- Additional engine plugins
- Manual WebSocket installation
- Special Unreal Engine version
- Complex build configuration
- Third-party downloads

### 🎯 Next Steps
1. Run `./scripts/check_websocket_status.sh` to verify
2. Build the project normally
3. Configure Discord bot (see QUICKSTART.md)
4. Run your server
5. Enjoy Discord integration! 🚀

---

## Additional Resources

- [WebSocket How-To Guide](WEBSOCKET_HOW_TO.md) - Complete walkthrough
- [WebSocket FAQ](WEBSOCKET_FAQ.md) - Detailed Q&A
- [Build Requirements](BUILD_REQUIREMENTS.md) - Technical specifications
- [Troubleshooting](TROUBLESHOOTING.md) - Common issues
- [Discord Chat Bridge README](Mods/DiscordChatBridge/README.md) - Mod overview
- [Quick Start Guide](Mods/DiscordChatBridge/help/QUICKSTART.md) - Setup instructions

---

**Still confused?** Read [WEBSOCKET_HOW_TO.md](WEBSOCKET_HOW_TO.md) for a step-by-step explanation!
