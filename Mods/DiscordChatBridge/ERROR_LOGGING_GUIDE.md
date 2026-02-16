# Error Logging Quick Reference

The Discord Chat Bridge mod now includes file-based error logging to help you diagnose issues.

## Quick Start

1. **Default Location**: Error logs are automatically saved to:
   ```
   Mods/DiscordChatBridge/Logs/DiscordChatBridge_YYYYMMDD_HHMMSS.log
   ```

2. **Access Your Logs**:
   - Navigate to your server's mod directory
   - Open `Mods/DiscordChatBridge/Logs/`
   - Find the most recent `.log` file (files are timestamped)

3. **View Log Contents**:
   - Open with any text editor (Notepad, vim, nano, etc.)
   - Look for `[ERROR]` and `[WARNING]` entries
   - Each entry has a timestamp showing when it occurred

## Custom Log Location

To save logs to a different directory, edit your configuration:

**For config/ format** (Recommended):
```ini
# In: Mods/DiscordChatBridge/config/DiscordChatBridge.ini
[/Script/DiscordChatBridge.DiscordChatSubsystem]
LogFilePath=C:\MyServerLogs\Discord
```

**For Config/ format** (Legacy):
```ini
# In: %localappdata%/FactoryGame/Saved/Config/WindowsServer/DiscordChatBridge.ini
# Or: ~/.config/Epic/FactoryGame/Saved/Config/LinuxServer/DiscordChatBridge.ini
[/Script/DiscordChatBridge.DiscordChatSubsystem]
LogFilePath=/home/satisfactory/logs/discord
```

## What Gets Logged

- **Errors**: Critical failures, connection issues, API errors
- **Warnings**: Configuration problems, temporary failures
- **Info**: Startup messages, configuration loaded, shutdown

## Example Log Output

```
=============================================================================
Discord Chat Bridge - Log File
Started: 2024-02-16 14:30:00
Log File: /path/to/Mods/DiscordChatBridge/Logs/DiscordChatBridge_20240216_143000.log
=============================================================================
[2024-02-16 14:30:00] [INFO] ====================================================================
[2024-02-16 14:30:00] [INFO] Discord Chat Bridge - Initializing
[2024-02-16 14:30:00] [INFO] ====================================================================
[2024-02-16 14:30:05] [ERROR] DiscordAPI: ❌ Cannot start polling - API not initialized
[2024-02-16 14:30:05] [ERROR] DiscordAPI: Check that BotToken and ChannelId are configured
```

## Common Issues and Log Messages

### Missing Configuration
```
[ERROR] DiscordAPI: ❌ Cannot start polling - API not initialized
[ERROR] DiscordAPI: Check that BotToken and ChannelId are configured
```
**Solution**: Edit your configuration file and add BotToken and ChannelId

### Failed to Send Message
```
[WARNING] DiscordAPI: Failed to send message - Response code: 401
```
**Solution**: Check that your bot token is correct and the bot has proper permissions

### WebSocket Issues (Gateway Mode)
```
[ERROR] DiscordAPI: ❌ Gateway Feature NOT Available
[ERROR]    WebSockets plugin was not found during compilation
```
**Solution**: Gateway mode requires WebSockets plugin - see BUILD_REQUIREMENTS.md

## Troubleshooting the Logger

If log files are not being created:

1. **Check Console Output**: Look for messages like:
   ```
   DiscordChatSubsystem: Initializing file logger at: ...
   DiscordChatSubsystem: ✓ File logger initialized successfully
   ```

2. **Verify Directory Permissions**: Ensure the server can write to the log directory

3. **Check Default Location**: If custom path fails, check the default location:
   ```
   Mods/DiscordChatBridge/Logs/
   ```

4. **Console Still Works**: Even if file logging fails, errors still appear in the server console

## Need Help?

If you're experiencing issues:

1. Check your log file for ERROR entries
2. Check the server console for any messages about the logger
3. Share relevant log snippets when asking for help (remove sensitive info like tokens!)
4. Include the timestamp of when the issue occurred

## Log File Management

- Each server restart creates a new timestamped log file
- Old log files are kept automatically
- Manually delete old logs if they accumulate too much
- Consider setting up log rotation in your server management scripts

---

For more information, see the main [README.md](README.md) file.
