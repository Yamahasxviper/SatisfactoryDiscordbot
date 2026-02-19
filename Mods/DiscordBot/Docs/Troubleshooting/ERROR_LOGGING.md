# Error Logging for Dedicated Servers

The Discord Bot mod includes an error logging feature specifically designed for dedicated server administrators who don't have direct access to server console logs.

## Overview

All errors and warnings from the Discord bot are automatically written to log files that can be easily accessed and reviewed, even on dedicated servers.

## Log File Location

By default, error logs are stored in:
```
<YourSatisfactoryInstallation>/Saved/Logs/DiscordBot/
```

For example:
- Windows: `C:/Program Files/Satisfactory/Saved/Logs/DiscordBot/`
- Linux: `/home/satisfactory/Saved/Logs/DiscordBot/`

## Configuration

You can customize the log directory location by adding the following to your `Config/DiscordBot.ini`:

```ini
[DiscordBot]
; Custom error log directory (optional)
ErrorLogDirectory=C:/MyServer/Logs/DiscordBot
```

If you don't specify a custom directory, the default location will be used automatically.

## Log File Format

Log files are named with timestamps:
```
DiscordBot_20240315_143025.log
```

Each log entry includes:
- Timestamp
- Severity level (ERROR or WARNING)
- Log category (e.g., LogDiscordGateway, LogDiscordBotSubsystem)
- Detailed error message

Example log entries:
```
[2024-03-15 14:30:25] [ERROR] LogDiscordGateway: Cannot connect: Bot token is empty
[2024-03-15 14:30:30] [WARNING] LogDiscordBotSubsystem: Cannot send message: Gateway client is null
```

## Log Rotation

To prevent log files from growing too large:
- Maximum log file size: 10 MB
- When the limit is reached, the current log is renamed to `.old`
- A new log file is created
- Only the current log and one `.old` file are kept

## What Gets Logged

The error logger captures:
- **Errors**: Critical issues that prevent the bot from functioning
  - Connection failures
  - Invalid configuration
  - Failed message sends
  - WebSocket errors

- **Warnings**: Non-critical issues that may affect functionality
  - Missing configuration values
  - Connection timeouts
  - Rate limiting notices

## Accessing Logs on Dedicated Servers

### Via FTP/SFTP
1. Connect to your server via FTP or SFTP
2. Navigate to the Satisfactory installation directory
3. Open `Saved/Logs/DiscordBot/`
4. Download the latest log file

### Via File Sharing
Set up a file share (SMB/CIFS) that includes the Satisfactory installation directory, then access the logs folder from your local machine.

### Via Remote Desktop
If you have RDP access, simply navigate to the logs folder directly.

## Troubleshooting Common Issues

### Log files not being created
- Check that the Discord bot mod is properly installed
- Verify the bot is enabled in `Config/DiscordBot.ini`
- Ensure the server has write permissions to the logs directory

### Custom log directory not working
- Make sure the path is absolute (not relative)
- Verify the directory exists or the server has permissions to create it
- Use forward slashes (/) or escaped backslashes (\\\\) in the path

### Logs are empty
- The logger only captures Discord bot related errors and warnings
- If there are no issues, the log will only contain the header
- Try generating an error (e.g., invalid bot token) to verify logging works

## Example Configuration

Here's a complete example of error logging configuration in `Config/DiscordBot.ini`:

```ini
[DiscordBot]
; Bot configuration
BotToken=YOUR_BOT_TOKEN_HERE
bEnabled=true

; Custom error log directory (Windows example)
ErrorLogDirectory=D:/SatisfactoryServer/Logs/DiscordBot

; Or for Linux
; ErrorLogDirectory=/opt/satisfactory/logs/discordbot
```

## Benefits for Dedicated Servers

- **No console access needed**: View logs through any file access method
- **Historical tracking**: Keep logs for troubleshooting past issues
- **Easy sharing**: Send log files to support or other administrators
- **Automated**: No manual configuration required - works out of the box
- **Lightweight**: Minimal performance impact with automatic rotation
