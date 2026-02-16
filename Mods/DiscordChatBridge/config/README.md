# Configuration Folder

This folder contains configuration files in INI format that work with SML (Satisfactory Mod Loader) and persist settings across server restarts.

## About This Folder

The `config` folder provides an alternative configuration system that:
- ✅ Uses standard **INI format** for easy editing
- ✅ **Loads automatically** when the server starts
- ✅ **Persists settings** across server restarts via SML's configuration system
- ✅ Works alongside the existing INI-based configuration in Config/
- ✅ Provides a **server-friendly** configuration option

## Configuration Files

### `DiscordChatBridge.ini`
The main configuration file for the Discord Chat Bridge mod. Contains all settings needed to configure:
- Discord bot token and channel ID
- Message polling intervals
- Message formatting options
- Server status notifications
- Bot activity/presence settings
- Gateway/WebSocket configuration

## How It Works

### Format
The INI files use the standard INI format with sections and KEY=VALUE pairs:
```ini
[/Script/DiscordChatBridge.DiscordChatSubsystem]
; This is a comment
BotToken=YOUR_BOT_TOKEN_HERE
ChannelId=123456789012345678
PollIntervalSeconds=2.0
```

- Each setting is on its own line
- Format: `KEY=VALUE` (no quotes needed)
- Lines starting with `;` are comments and will be ignored
- Empty lines are ignored
- Section header `[/Script/DiscordChatBridge.DiscordChatSubsystem]` is required

### Loading Process
1. Server starts and loads SML (Satisfactory Mod Loader)
2. DiscordChatBridge mod initializes
3. Configuration loader checks for `config/DiscordChatBridge.ini`
4. Settings are parsed and loaded into memory
5. SML's configuration system persists these settings

### Persistence
- Settings are automatically saved by SML's configuration system
- Changes made in-game (if exposed via UI) are persisted
- Settings survive server restarts
- No manual file editing needed after initial setup

## Setup Instructions

### First-Time Setup
1. Open `DiscordChatBridge.ini` in a text editor
2. Replace `YOUR_BOT_TOKEN_HERE` with your Discord bot token
3. Replace `YOUR_CHANNEL_ID_HERE` with your Discord channel ID
4. Customize other settings as needed (optional)
5. Save the file
6. Start your Satisfactory dedicated server
7. Settings will load automatically

### Configuration Priority
If both config/DiscordChatBridge.ini and Config/ INI configurations exist:
1. config/DiscordChatBridge.ini is checked first
2. Config/ INI format (via Unreal Engine config system) is used as fallback
3. SML persisted configuration has highest priority for runtime changes

## Benefits of INI Format

### Easy to Read and Edit
- Standard INI section syntax
- Simple KEY=VALUE pairs
- Extensive inline comments
- Clear section organization

### Server-Friendly
- Works directly with SML's configuration system
- Automatic persistence across restarts
- No need to manually copy files to runtime directories
- Settings can be version controlled easily

### Compatible
- Works alongside existing INI-based configuration in Config/
- Can migrate from Config/ to config/ format gradually
- No breaking changes to existing setups

## Troubleshooting

### Settings Not Loading
1. Check file name is exactly `DiscordChatBridge.ini`
2. Verify file is in `Mods/DiscordChatBridge/config/` folder
3. Check server logs for configuration loading messages
4. Ensure no syntax errors in the INI file (KEY=VALUE format)
5. Verify the section header `[/Script/DiscordChatBridge.DiscordChatSubsystem]` is present

### Settings Not Persisting
1. Verify SML is installed and working correctly
2. Check file permissions (server needs read/write access)
3. Look for error messages in server logs
4. Ensure SML configuration directory is writable

### Conflicts with Config/ Configuration
If you have both config/ and Config/ configurations:
- config/ INI format takes precedence
- To use Config/ only, remove or rename the config/DiscordChatBridge.ini file
- To use config/ only, remove or rename the Config/ INI files

## Example Configuration

Here's a minimal working example:

```ini
[/Script/DiscordChatBridge.DiscordChatSubsystem]
; Minimal Discord Chat Bridge Configuration
BotToken=MTIzNDU2Nzg5MDEyMzQ1Njc4OTAuAbCdEf.gHiJkLmNoPqRsTuVwXyZ0123456789
ChannelId=123456789012345678
PollIntervalSeconds=2.0
DiscordNameFormat=[Discord] {username}
GameNameFormat=**[{username}]** {message}
```

## Advanced Configuration

For advanced features like:
- Server status notifications
- Bot activity/presence updates
- Gateway/WebSocket integration
- Custom message formatting

See the comments in `DiscordChatBridge.ini` for detailed configuration options.

## Support

For issues or questions:
- Check the main README.md in the mod folder
- Review SETUP_GUIDE.md for detailed setup instructions
- Visit the Satisfactory Modding Discord: https://discord.gg/QzcG9nX
