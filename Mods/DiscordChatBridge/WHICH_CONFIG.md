# Which Configuration File Should I Use?

## ⚠️ IMPORTANT: Choose ONE Configuration Method

This mod has **TWO** configuration systems. You only need to use **ONE** of them.

## Quick Answer

**For new installations:** Use the **TXT format** (recommended) ⭐

**Already using INI?** Keep using it - both work fine!

---

## The Two Configuration Systems

### 1. TXT Format (Recommended) ⭐ NEW!

**Location:** `Mods/DiscordChatBridge/config/DiscordChatBridge.txt`

**Why use this:**
- ✅ Simple KEY=VALUE format - easy to edit
- ✅ Located in the mod directory - easy to find
- ✅ Automatically persists via SML configuration system
- ✅ Settings survive server restarts
- ✅ Extensive inline documentation
- ✅ No need to copy files anywhere

**Perfect for:**
- New installations
- Server administrators who want simplicity
- Anyone who prefers straightforward configuration

**Documentation:**
- [config/README.md](config/README.md) - Complete TXT format guide
- [help/QUICKSTART.md](help/QUICKSTART.md) - 5-minute setup guide

---

### 2. INI Format (Legacy)

**Location (runtime):**
- Windows: `%localappdata%/FactoryGame/Saved/Config/WindowsServer/DiscordChatBridge.ini`
- Linux: `~/.config/Epic/FactoryGame/Saved/Config/LinuxServer/DiscordChatBridge.ini`

**Template:** `Mods/DiscordChatBridge/Config/DefaultDiscordChatBridge.ini`

**Why use this:**
- Traditional Unreal Engine configuration format
- Compatible with existing server management tools
- Same format as other Satisfactory/Unreal mods

**Perfect for:**
- Existing installations already using INI
- Server administrators familiar with Unreal Engine config system
- Automated deployment scripts that manage INI files

**Documentation:**
- [Config/ExampleConfigs/README.md](Config/ExampleConfigs/README.md) - INI examples
- Main [README.md](README.md) - See "Configuration" section

---

## Configuration Priority

The mod loads configuration in this order:

1. **TXT format** (`config/DiscordChatBridge.txt`) - Checked first
2. **INI format** (Config system) - Used as fallback if TXT not found

**If both exist:** TXT format takes priority, INI is ignored.

---

## Migration Guide

### From INI to TXT

1. Copy your settings from your INI file
2. Edit `Mods/DiscordChatBridge/config/DiscordChatBridge.txt`
3. Paste your settings using KEY=VALUE format (no sections needed)
4. Save and restart server
5. (Optional) Delete or rename your old INI file

### From TXT to INI

1. Copy your settings from `config/DiscordChatBridge.txt`
2. Create the INI file in the runtime location (see above)
3. Add the `[/Script/DiscordChatBridge.DiscordChatSubsystem]` section
4. Add your settings in INI format
5. Delete or rename `config/DiscordChatBridge.txt` (so INI is used)

---

## Still Confused?

**For the simplest setup:**

1. Open `Mods/DiscordChatBridge/config/DiscordChatBridge.txt`
2. Replace `YOUR_BOT_TOKEN_HERE` with your Discord bot token
3. Replace `YOUR_CHANNEL_ID_HERE` with your Discord channel ID
4. Save and restart your server
5. Done! ✅

**Need step-by-step help?**
- See [help/QUICKSTART.md](help/QUICKSTART.md) for a 5-minute guided setup
- See [help/SETUP_GUIDE.md](help/SETUP_GUIDE.md) for detailed instructions

---

## Summary

| Question | Answer |
|----------|--------|
| **I'm setting up for the first time** | Use TXT format (`config/DiscordChatBridge.txt`) |
| **I already have an INI file** | Keep using INI - it works great! |
| **Can I use both?** | No, TXT takes priority if both exist |
| **Which is better?** | TXT is simpler for most users |
| **Can I switch later?** | Yes, see Migration Guide above |
| **Where do I get help?** | See [help/QUICKSTART.md](help/QUICKSTART.md) |

---

**Ready to configure?**
- TXT format → [config/README.md](config/README.md)
- INI format → [README.md](README.md#configuration)
- Quick start → [help/QUICKSTART.md](help/QUICKSTART.md)
