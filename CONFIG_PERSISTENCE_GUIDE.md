# Discord Bot Configuration Persistence Guide

**Status:** ✅ All server configuration files are saved and persist across server restarts

**Last Updated:** 2026-02-18

---

## Overview

The Discord Bot mod uses Unreal Engine's configuration system to store all settings in INI files. **All configuration files persist across server restarts automatically** because they are static files stored on the filesystem.

## Configuration File Locations

The Discord Bot mod reads configuration from the following locations (in order of priority):

### 1. Development/Editor
```
Config/DefaultGame.ini
```
Used when running in the Unreal Editor for development and testing.

### 2. Windows Dedicated Server
```
Config/WindowsServer/WindowsServerGame.ini  (highest priority)
Config/DefaultGame.ini                       (fallback)
```
Platform-specific settings override the defaults from DefaultGame.ini.

### 3. Linux Dedicated Server
```
Config/LinuxServer/LinuxServerGame.ini      (highest priority)
Config/DefaultGame.ini                       (fallback)
```
Platform-specific settings override the defaults from DefaultGame.ini.

### 4. Generic Dedicated Server
```
Config/DefaultGame.ini
```
Used if no platform-specific config exists.

## How Configuration Persistence Works

### ✅ Configuration IS Persisted
- **Manual edits to INI files** - Any changes you make to config files are saved immediately
- **All settings in [DiscordBot] section** - Bot token, channel IDs, messages, etc.
- **Platform-specific overrides** - WindowsServerGame.ini and LinuxServerGame.ini persist
- **Cross-restart persistence** - Config files are never modified by the mod, only read

### ❌ Configuration is NOT Persisted
- **Runtime changes via Blueprint/C++** - Dynamic changes to bot behavior are not saved
- **Temporary state** - Player count, connection status, bot presence (these are ephemeral)
- **Dynamic messages** - Chat messages, notifications sent at runtime

## Configuration Workflow

### For Server Administrators

**Initial Setup:**
1. Edit `Config/DefaultGame.ini` or platform-specific config file
2. Set your Discord bot token: `BotToken=your_actual_token_here`
3. Set your channel IDs: `ChatChannelId=your_channel_id`
4. Configure other settings as desired
5. Save the file
6. Start or restart your server

**Making Changes:**
1. Stop your server (optional but recommended)
2. Edit the config file with your new settings
3. Save the file
4. Restart your server
5. Changes take effect immediately on startup

**Verifying Persistence:**
1. Make a change to any [DiscordBot] setting
2. Save the config file
3. Restart the server
4. Verify the setting is still applied
5. ✅ **Result:** Settings persist because config files are never modified by the mod

### For Mod Developers

**Adding New Settings:**
1. Add property to `UDiscordBotSubsystem` class
2. Load setting in `Initialize()`, `LoadTwoWayChatConfig()`, or `LoadServerNotificationConfig()`
3. Use `GConfig->GetString/GetBool/GetInt/GetFloat()`
4. Always use `GConfig->GetConfigFilename(TEXT("Game"))` for cross-platform compatibility
5. Document the new setting in config files and README

**Configuration Loading Pattern:**
```cpp
if (GConfig)
{
    // Use explicit config filename for cross-platform compatibility
    FString ConfigFilename = GConfig->GetConfigFilename(TEXT("Game"));
    GConfig->GetBool(TEXT("DiscordBot"), TEXT("bEnabled"), bEnabled, ConfigFilename);
}
```

## Platform-Specific Configuration

### Windows Server Example

Edit `Config/WindowsServer/WindowsServerGame.ini`:
```ini
[DiscordBot]
BotToken=YOUR_WINDOWS_SERVER_TOKEN
ChatChannelId=123456789012345678
bEnabled=true
```

### Linux Server Example

Edit `Config/LinuxServer/LinuxServerGame.ini`:
```ini
[DiscordBot]
BotToken=YOUR_LINUX_SERVER_TOKEN
ChatChannelId=987654321098765432
bEnabled=true
```

### Multi-Platform Setup

You can have different configurations for each platform:
- Different bot tokens per platform
- Different Discord channels per platform
- Different notification messages per platform

The platform-specific config automatically overrides DefaultGame.ini when running on that platform.

## Configuration Validation

### On Startup
The mod performs the following validations:
1. ✅ Checks if `bEnabled=true`
2. ✅ Checks if `BotToken` is set and not placeholder
3. ✅ Checks if channel IDs are set and not placeholders
4. ⚠️ Warns if required settings are missing
5. ⚠️ Warns if bot token appears invalid

### Validation Messages
```
✅ "Discord Bot Subsystem initialized"
✅ "Two-way chat enabled with X channel(s)"
⚠️ "Discord bot enabled but no valid token configured"
⚠️ "Cannot initialize chat relay: ChatManager not found"
```

## Troubleshooting

### Problem: Settings not being applied after restart
**Cause:** Editing the wrong config file
**Solution:** 
- For Windows Server: Edit `Config/WindowsServer/WindowsServerGame.ini`
- For Linux Server: Edit `Config/LinuxServer/LinuxServerGame.ini`
- For Editor/Dev: Edit `Config/DefaultGame.ini`

### Problem: Config file not found
**Cause:** Config file doesn't exist for your platform
**Solution:** 
- Platform-specific configs are optional
- If missing, the mod uses `Config/DefaultGame.ini`
- Create platform-specific config by copying from DefaultGame.ini

### Problem: Changes not taking effect
**Cause:** Server not restarted or config syntax error
**Solution:**
- Restart the server after making config changes
- Check INI syntax (no quotes around strings unless part of the value)
- Check for typos in setting names
- Look for validation warnings in server logs

### Problem: Config reset after update
**Cause:** Never happens - config files are never modified by the mod
**Solution:** 
- Config files persist across all updates
- If you think config was lost, check if you're editing the correct file
- Check if file permissions are correct (file should be readable)

## Best Practices

### ✅ DO
- Keep backups of your config files before making changes
- Use version control (git) to track config changes
- Document your config settings with comments
- Use platform-specific configs for different server environments
- Restart server after making config changes for immediate effect
- Use descriptive bot tokens and channel IDs (add comments)

### ❌ DON'T
- Don't commit bot tokens to version control (use placeholders)
- Don't modify config files while server is running (changes won't apply until restart)
- Don't delete the `[DiscordBot]` section from config files
- Don't expect runtime changes (via Blueprint) to persist
- Don't use special characters in config values without understanding INI format

## Migration from Mods/DiscordBot/Config/DiscordBot.ini

**Historical Note:** Earlier versions of the mod used a separate config file at `Mods/DiscordBot/Config/DiscordBot.ini`. 

**Current Behavior:** 
- Config is now read from `Config/DefaultGame.ini` and platform-specific overrides
- This ensures proper packaging and deployment to dedicated servers
- Settings are merged into the Game config hierarchy automatically

**No Migration Needed:**
- If you have settings in both locations, `Config/DefaultGame.ini` takes precedence
- The separate `Mods/DiscordBot/Config/DiscordBot.ini` file is kept for reference
- You can safely use either location, but `Config/DefaultGame.ini` is recommended

## Summary

| Question | Answer |
|----------|--------|
| Are config files saved after server restart? | ✅ YES - Config files are static and never modified |
| Do I need to do anything special to persist config? | ❌ NO - Just edit the INI file and restart |
| Can I have different configs per platform? | ✅ YES - Use platform-specific config files |
| Do runtime changes persist? | ❌ NO - Only manual INI edits persist |
| Is config backed up automatically? | ❌ NO - You should backup configs manually |
| Can config be lost on update? | ❌ NO - Config files are never overwritten |

---

## Related Documentation

- [Server Configuration Fix](SERVER_CONFIG_FIX.md) - Details on the cross-platform config loading fix
- [Setup Guide](Mods/DiscordBot/SETUP.md) - Initial setup instructions
- [Two-Way Chat](Mods/DiscordBot/TWO_WAY_CHAT.md) - Chat configuration details
- [Error Logging](Mods/DiscordBot/Docs/Troubleshooting/ERROR_LOGGING.md) - Logging configuration

## Technical References

- [Unreal Engine Config Files](https://dev.epicgames.com/documentation/en-us/unreal-engine/configuration-files-in-unreal-engine)
- [Config File Hierarchy](https://ikrima.dev/ue4guide/application-os-generic-platform/config-files/)
- [Dedicated Server Configuration](https://forums.unrealengine.com/t/server-specific-config-files/2561821)
