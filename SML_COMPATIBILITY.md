# SML Compatibility Status

## ✅ YES - Your Discord Project Works with SML

**Last Updated:** February 2026  
**Status:** FULLY COMPATIBLE ✅

---

## Summary

The **Discord Chat Bridge** mod in this repository is **fully compatible** with **Satisfactory Mod Loader (SML)** and will work correctly with your current setup.

---

## Version Compatibility

| Component | Required Version | Current Version | Status |
|-----------|-----------------|-----------------|--------|
| **SML** | ^3.11.3 (3.11.3+, <4.0.0) | 3.11.3 | ✅ Compatible |
| **Game Version** | >=416835 | >=416835 | ✅ Compatible |
| **WebSockets** | Optional (for Gateway features) | Available | ✅ Compatible |

### What the Version Numbers Mean

- **SML ^3.11.3**: The caret (`^`) means the mod requires SML version 3.11.3 or any higher version in the 3.x series (but not 4.0.0+)
- Your repository contains **exactly version 3.11.3** ✅
- This is a **perfect match** - no updates needed

---

## How It Works

### 1. DiscordChatBridge depends on SML

The `DiscordChatBridge.uplugin` file explicitly declares SML as a dependency:

```json
"Plugins": [
  {
    "Name": "SML",
    "Enabled": true,
    "SemVersion": "^3.11.3"
  }
]
```

### 2. SML is included in this repository

Both mods are in the same Unreal project:
- `/Mods/SML/` - Satisfactory Mod Loader (version 3.11.3)
- `/Mods/DiscordChatBridge/` - Your Discord mod (depends on SML)

### 3. Server-side only setup

- **Server requirement**: SML must be installed on the Satisfactory server
- **Client requirement**: Players do NOT need SML or the Discord mod installed
- The mod uses `"RequiredOnRemote": false` to allow vanilla clients to connect

---

## Dependencies

All required dependencies are met:

### Core Dependencies ✅
- ✅ **SML** (version 3.11.3) - Present in repository
- ✅ **OnlineSubsystem** - Standard Unreal Engine plugin
- ✅ **OnlineSubsystemUtils** - Standard Unreal Engine plugin  
- ✅ **ReplicationGraph** - Standard Unreal Engine plugin

### Optional Dependencies ✅
- ✅ **WebSockets** (marked "Optional") - Required only for Discord Gateway features
  - Enables "Playing with X players" bot presence
  - Works without it (REST API mode still functions)
  - See [BUILD_REQUIREMENTS.md](BUILD_REQUIREMENTS.md) for build-time setup

---

## Deployment

### For Server Administrators

1. **Install SML on your Satisfactory server**
   - Download from [ficsit.app/sml-versions](https://ficsit.app/sml-versions)
   - Version 3.11.3 or higher in the 3.x series

2. **Install DiscordChatBridge mod**
   - Place in server mods directory
   - Configure the Discord bot token and channel ID
   - See [Mods/DiscordChatBridge/README.md](Mods/DiscordChatBridge/README.md)

3. **Players don't need to install anything**
   - Vanilla Satisfactory clients can connect
   - No SML installation required for players
   - Chat bridge works automatically

---

## Building from Source

When building this project:

1. **SML builds first** (it's a core dependency)
2. **DiscordChatBridge builds second** (depends on SML)
3. Both mods package together into the final build

The Unreal Engine build system automatically handles the dependency chain.

---

## Common Questions

### Q: Will this work on my Satisfactory server?
**A:** Yes, as long as your server has SML 3.11.3+ installed. ✅

### Q: Do players need to install SML?
**A:** No, only the server needs SML. Players can use vanilla clients. ✅

### Q: What if I update SML to a newer 3.x version?
**A:** That's fine! The `^3.11.3` requirement means any 3.x version ≥3.11.3 will work. ✅

### Q: Will it work with SML 4.0 when it's released?
**A:** No, the `^3.11.3` requirement excludes 4.x. You would need to update the mod's SML dependency when SML 4.0 is released. ⚠️

### Q: What about WebSockets?
**A:** WebSockets is optional. The mod works without it (REST API mode), but you need it for Gateway features like bot presence status. ✅

---

## Testing Compatibility

To verify compatibility on your system:

```bash
# Check SML version in the repository
cat Mods/SML/SML.uplugin | grep -A 2 "VersionName"

# Check DiscordChatBridge SML dependency
cat Mods/DiscordChatBridge/DiscordChatBridge.uplugin | grep -A 3 '"Name": "SML"'

# Verify WebSockets availability (for Gateway features)
cat FactoryGame.uproject | grep -A 3 '"Name": "WebSockets"'
```

Expected output should show:
- SML version 3.11.3 ✅
- DiscordChatBridge requires SML ^3.11.3 ✅
- WebSockets enabled (optional) ✅

---

## Troubleshooting

### Problem: "Missing SML dependency" error

**Solution:** Ensure SML is:
1. Present in the `/Mods/SML/` directory
2. Listed in the project's plugin dependencies
3. Built before DiscordChatBridge

### Problem: "Version mismatch" error

**Solution:** 
1. Check SML version: Should be 3.11.3 or higher (3.x series)
2. Update the `SemVersion` in `DiscordChatBridge.uplugin` if needed
3. Rebuild the project

### Problem: Players can't connect

**Solution:** Verify `"RequiredOnRemote": false` is set in `DiscordChatBridge.uplugin` (line 19)

---

## Additional Resources

- **SML Documentation**: [docs.ficsit.app](https://docs.ficsit.app/)
- **Discord Mod Setup**: [Mods/DiscordChatBridge/README.md](Mods/DiscordChatBridge/README.md)
- **Build Requirements**: [BUILD_REQUIREMENTS.md](BUILD_REQUIREMENTS.md)
- **Troubleshooting**: [TROUBLESHOOTING.md](TROUBLESHOOTING.md)

---

## Conclusion

✅ **Your current setup WILL work with SML.**

No changes are required. The Discord Chat Bridge mod is properly configured to work with SML 3.11.3, and all dependencies are met.
