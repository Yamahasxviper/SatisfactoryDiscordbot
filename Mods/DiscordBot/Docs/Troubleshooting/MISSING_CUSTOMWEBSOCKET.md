# Troubleshooting: CustomWebSocket Module Not Found

## Problem

You see one of these errors in your game logs:

```
LogDiscordBot: Error: CRITICAL ERROR: CustomWebSocket plugin not found!
LogDiscordBot: Error: ERROR: CustomWebSocket plugin found but not enabled!
LogDiscordBot: Error: ERROR: Failed to load CustomWebSocket module!
```

## Quick Solution

**The DiscordBot mod requires the CustomWebSocket mod to be installed separately.**

### Step-by-Step Fix:

1. **Open Satisfactory Mod Manager (SMM)**

2. **Search for "CustomWebSocket"**
   - If not installed: Click "Install"
   - If installed but disabled: Click "Enable"

3. **Verify DiscordBot is also enabled**
   - Search for "DiscordBot"
   - Make sure it shows as "Enabled"

4. **Restart your game/server**

5. **Check the logs**
   - You should now see: `LogDiscordBot: CustomWebSocket module already loaded`

## Why This Happens

DiscordBot and CustomWebSocket are **two separate mods**:

- **DiscordBot** = Discord integration features (chat, notifications, etc.)
- **CustomWebSocket** = Low-level WebSocket communication (required by DiscordBot)

Think of it like:
- CustomWebSocket = The "engine" 
- DiscordBot = The "car" that uses the engine

Both must be installed for the Discord bot to work.

## Common Mistakes

### ❌ Mistake 1: Only installing DiscordBot
**Problem**: Installing DiscordBot alone won't work.  
**Solution**: Also install CustomWebSocket.

### ❌ Mistake 2: Different installation locations
**Problem**: Installing mods manually in different folders.  
**Solution**: Use SMM to install both mods. It will put them in the correct location automatically.

### ❌ Mistake 3: One mod is disabled
**Problem**: CustomWebSocket is installed but disabled in SMM.  
**Solution**: Enable both mods in SMM.

## Detailed Error Messages

### Error 1: Plugin Not Found

```
LogDiscordBot: Error: ========================================================================================
LogDiscordBot: Error: CRITICAL ERROR: CustomWebSocket plugin not found!
LogDiscordBot: Error: ========================================================================================
LogDiscordBot: Error: The DiscordBot mod requires the CustomWebSocket mod to be installed.
```

**Meaning**: CustomWebSocket mod is not installed at all.

**Fix**: Install CustomWebSocket mod via SMM.

---

### Error 2: Plugin Not Enabled

```
LogDiscordBot: Error: ========================================================================================
LogDiscordBot: Error: ERROR: CustomWebSocket plugin found but not enabled!
LogDiscordBot: Error: ========================================================================================
LogDiscordBot: Error: Plugin location: C:/Path/To/Mods/CustomWebSocket
```

**Meaning**: CustomWebSocket mod is installed but not enabled.

**Fix**: Enable CustomWebSocket mod in SMM.

---

### Error 3: Module Loading Failed

```
LogDiscordBot: Error: ========================================================================================
LogDiscordBot: Error: ERROR: Failed to load CustomWebSocket module!
LogDiscordBot: Error: ========================================================================================
LogDiscordBot: Error: Plugin location: C:/Path/To/Mods/CustomWebSocket
```

**Meaning**: Mod is installed and enabled, but failed to load. This is rare and usually indicates:
- Corrupted mod files
- Incompatible mod versions
- Conflicting mods

**Fix**: 
1. Reinstall both mods via SMM
2. Update both mods to latest version
3. Check for mod conflicts

## Verification

After fixing, you should see these messages in the logs:

### ✅ Success (Already Loaded)
```
LogDiscordBot: DiscordBot module starting up
LogDiscordBot: CustomWebSocket module already loaded from: C:/Path/To/Mods/CustomWebSocket
LogDiscordBot: Error logging initialized at: Saved/Logs/DiscordBot
```

### ✅ Success (Auto-Loaded)
```
LogDiscordBot: DiscordBot module starting up
LogDiscordBot: Warning: CustomWebSocket module not loaded, attempting to load...
LogDiscordBot: CustomWebSocket module loaded successfully from: C:/Path/To/Mods/CustomWebSocket
LogDiscordBot: Error logging initialized at: Saved/Logs/DiscordBot
```

## Where to Find Logs

**Windows**: 
```
%LOCALAPPDATA%/FactoryGame/Saved/Logs/
```

**Linux**:
```
~/.config/Epic/FactoryGame/Saved/Logs/
```

Look for the latest `.log` file and search for "DiscordBot" or "CustomWebSocket".

## Still Having Issues?

If you've followed all steps and still have problems:

1. **Check mod versions**: Make sure both mods are up-to-date
2. **Check compatibility**: Verify the mods are compatible with your game version
3. **Reinstall both mods**: 
   - Uninstall both DiscordBot and CustomWebSocket
   - Restart game/server
   - Reinstall both mods
   - Restart again
4. **Check for conflicts**: Disable other mods to see if there's a conflict
5. **Report the issue**: If nothing works, report the issue with your full logs

## See Also

- [Complete WebSocket Troubleshooting Guide](WEBSOCKET_TROUBLESHOOTING.md)
- [DiscordBot Setup Guide](../Setup/SETUP.md)
- [Error Logging Guide](ERROR_LOGGING.md)

---

**Last Updated**: 2024-02-19  
**Status**: Active Issue - Common Problem
