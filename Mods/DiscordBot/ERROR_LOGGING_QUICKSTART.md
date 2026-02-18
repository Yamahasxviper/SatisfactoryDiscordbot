# Quick Start: Error Logging for Dedicated Servers

## For Server Administrators Without Console Access

If you're running a Satisfactory dedicated server and can't access the console logs, the Discord Bot mod now automatically creates error logs that you can view via FTP or file sharing.

### What You Get

✅ **Automatic error logging** - No configuration required  
✅ **Easy access** - View logs via FTP/SFTP/file sharing  
✅ **Smart rotation** - Logs automatically rotate at 10MB  
✅ **Clean output** - Only Discord bot errors and warnings  

### Default Log Location

```
YourSatisfactoryServer/Saved/Logs/DiscordBot/
```

### Quick Access Methods

#### Option 1: FTP/SFTP (Most Common)
1. Connect to your server with FileZilla or WinSCP
2. Navigate to: `Satisfactory/Saved/Logs/DiscordBot/`
3. Download the latest `.log` file
4. Open with any text editor

#### Option 2: File Share
1. If you have a shared folder set up (SMB/CIFS)
2. Browse to the shared Satisfactory folder
3. Open `Saved/Logs/DiscordBot/`
4. View the log file directly

#### Option 3: Web Panel
Many game server hosting panels provide file browsers:
1. Log into your hosting control panel
2. Use the file manager
3. Navigate to `Saved/Logs/DiscordBot/`

### What's in the Logs?

The logs show Discord bot issues like:
- Connection problems
- Configuration errors
- Message send failures
- WebSocket issues
- Token validation errors

Example log:
```
=== Discord Bot Error Log ===
Started: 2024-03-15 14:30:25
=================================

[2024-03-15 14:30:25] [ERROR] LogDiscordGateway: Cannot connect: Bot token is empty
[2024-03-15 14:30:30] [WARNING] LogDiscordBotSubsystem: Cannot send message: Gateway client is null
```

### Custom Log Location (Optional)

If you want logs in a different location, edit `Config/DiscordBot.ini`:

```ini
[DiscordBot]
; Put your logs wherever you want
ErrorLogDirectory=/my/custom/logs/path
```

### Troubleshooting

**No logs folder?**
- The folder is created automatically when the bot starts
- Make sure the bot is enabled: `bEnabled=true` in config
- Check server has write permissions

**Logs are empty?**
- Empty logs mean no errors! (Good thing!)
- If you suspect issues, check that the bot token is configured

**Can't find the logs?**
- Default location: `<SatisfactoryServer>/Saved/Logs/DiscordBot/`
- Check your FTP path carefully
- Try searching for files named `DiscordBot_*.log`

### Need More Help?

See the full documentation:
- [ERROR_LOGGING.md](ERROR_LOGGING.md) - Detailed documentation
- [README.md](README.md) - Complete bot setup guide
- [SETUP.md](SETUP.md) - Initial setup instructions

---

**That's it!** The error logging works automatically - just access your server files and check the logs folder whenever you need to troubleshoot Discord bot issues.
