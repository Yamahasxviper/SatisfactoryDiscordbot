# Alpakit Quick Reference Card

**Having issues with Alpakit not compiling DiscordBot or CustomWebSocket?**  
👉 See [ALPAKIT_TROUBLESHOOTING_GUIDE.md](ALPAKIT_TROUBLESHOOTING_GUIDE.md) for complete solutions.

---

## 5-Second Check ✅

```bash
# Are these files present?
ls Mods/DiscordBot/Config/Alpakit.ini
ls Plugins/CustomWebSocket/Config/Alpakit.ini
ls Mods/SML/Config/Alpakit.ini
```

**If all three exist:** Your configuration is correct! See troubleshooting guide for compilation issues.  
**If any are missing:** Run the commands in the troubleshooting guide to create them.

---

## Most Common Issues

### 1. Plugins Don't Show in Alpakit
**Fix:** Verify Alpakit.ini files exist (see above)

### 2. "Plugin SML not found"
**Fix:** Enable plugins in FactoryGame.uproject:
```bash
grep '"Name": "SML"' FactoryGame.uproject
grep '"Name": "CustomWebSocket"' FactoryGame.uproject
grep '"Name": "DiscordBot"' FactoryGame.uproject
# All should show "Enabled": true
```

### 3. "Module CustomWebSocket not found"
**Fix:** Build order issue. Build CustomWebSocket before DiscordBot.

### 4. Wrong Unreal Engine Version
**Fix:** You need **Unreal Engine 5.3.2-CSS** (custom Satisfactory version), not standard UE5.

### 5. Missing Wwise
**Fix:** Install Wwise plugin (see .github/workflows/build.yml for URL)

---

## Quick Build Commands

### Using Alpakit (GUI)
1. Open Unreal Editor
2. Tools → Alpakit
3. Select DiscordBot and CustomWebSocket
4. Click "Alpakit!"

### Using UAT (Command Line)
```bash
# Windows
.\Engine\Build\BatchFiles\RunUAT.bat -ScriptsForProject="FactoryGame.uproject" PackagePlugin -Project="FactoryGame.uproject" -dlcname=CustomWebSocket -merge -build -server -clientconfig=Shipping -serverconfig=Shipping -platform=Win64 -serverplatform=Win64+Linux

.\Engine\Build\BatchFiles\RunUAT.bat -ScriptsForProject="FactoryGame.uproject" PackagePlugin -Project="FactoryGame.uproject" -dlcname=DiscordBot -merge -build -server -clientconfig=Shipping -serverconfig=Shipping -platform=Win64 -serverplatform=Win64+Linux
```

---

## Verification Scripts

```bash
# Check Discord bot integration
./verify_discordbot_integration.sh

# Check WebSocket functionality
./verify_websocket.sh

# Check configuration
./verify_config_persistence.sh
```

---

## Build Order

1. **CustomWebSocket** (no dependencies)
2. **SML** (depends on CustomWebSocket)
3. **DiscordBot** (depends on SML and CustomWebSocket)

---

## Getting Help

📖 **Full Guide:** [ALPAKIT_TROUBLESHOOTING_GUIDE.md](ALPAKIT_TROUBLESHOOTING_GUIDE.md)  
📖 **Build Guide:** [BUILD_GUIDE.md](BUILD_GUIDE.md)  
📖 **Plugin Fix Details:** [ALPAKIT_COMPILATION_FIX.md](ALPAKIT_COMPILATION_FIX.md)  
💬 **Discord:** https://discord.gg/QzcG9nX  
🐛 **Issues:** https://github.com/Yamahasxviper/SatisfactoryDiscordbot/issues  

---

## Status Check ✓

Your repository already has all fixes applied:
- ✅ Alpakit.ini files exist
- ✅ Plugins registered in FactoryGame.uproject
- ✅ Dependencies properly configured
- ✅ LoadingPhase settings correct

If Alpakit still isn't working, the issue is likely environment-related (wrong UE version, missing Wwise, etc.). See the full troubleshooting guide for solutions.
