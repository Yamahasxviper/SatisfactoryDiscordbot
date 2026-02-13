# Discord Bot Configuration Guide

This guide explains how to configure the Discord bot integration for your Satisfactory dedicated server.

## Configuration Files

The Discord bot integration uses two configuration files:

1. **`Config/DefaultDiscordBot.ini`** - Main Discord bot settings
2. **`Mods/DiscordBotIntegration.ini`** - Mod-specific settings and features

## Quick Start

### 1. Set Up Discord Bot

1. Go to [Discord Developer Portal](https://discord.com/developers/applications)
2. Create a new application
3. Navigate to "Bot" section and create a bot
4. Copy the bot token
5. Enable the following intents:
   - Server Members Intent
   - Message Content Intent

### 2. Configure Bot Token

Edit `Config/DefaultDiscordBot.ini`:

```ini
DiscordBotToken=YOUR_BOT_TOKEN_HERE
DiscordServerID=YOUR_SERVER_ID_HERE
MainChatChannelID=YOUR_CHANNEL_ID_HERE
```

### 3. Invite Bot to Server

Use this URL (replace CLIENT_ID with your application ID):
```
https://discord.com/api/oauth2/authorize?client_id=CLIENT_ID&permissions=274877975552&scope=bot
```

## Configuration Options

### Discord Bot Connection Settings

| Setting | Description | Default |
|---------|-------------|---------|
| `DiscordBotToken` | Your Discord bot token from the Developer Portal | (empty) |
| `DiscordWebhookURL` | Alternative webhook URL for one-way messaging | (empty) |
| `DiscordServerID` | Your Discord server (guild) ID | (empty) |

**Note:** You need either a bot token (for two-way communication) or webhook URL (for game→Discord only).

### Channel Configuration

| Setting | Description |
|---------|-------------|
| `MainChatChannelID` | Channel for relaying in-game chat |
| `AdminChannelID` | Channel for admin notifications |
| `LogChannelID` | Channel for detailed server logs |

**How to get Channel IDs:**
1. Enable Developer Mode in Discord (User Settings → Advanced)
2. Right-click on a channel and select "Copy ID"

### Chat Relay Settings

#### Game to Discord

```ini
; Enable game chat relay to Discord
bEnableGameToDiscordRelay=true

; Customize message format
GameChatMessageFormat=[{time}] **{player}**: {message}

; Enable player notifications
bEnablePlayerNotifications=true
```

**Available Placeholders:**
- `{player}` - Player name
- `{message}` - Chat message content
- `{time}` - Timestamp
- `{server}` - Server name
- `{playercount}` - Current player count

#### Discord to Game

```ini
; Enable Discord chat relay to game
bEnableDiscordToGameRelay=true

; Customize message format
DiscordChatMessageFormat=[Discord] <{user}>: {message}

; Enable Discord commands
bEnableDiscordCommands=true
CommandPrefix=!
```

**Available Placeholders:**
- `{user}` - Discord username
- `{message}` - Discord message content
- `{role}` - User's highest role

### Message Filtering

Control what messages get relayed:

```ini
; Enable filtering
bEnableMessageFiltering=false

; Filter messages starting with these characters
FilteredPrefixes=/,!,.

; Maximum message length (characters)
MaxMessageLength=500
```

### Server Status Updates

Configure automatic server status messages:

```ini
bEnableServerStatusUpdates=true
ServerStatusUpdateInterval=300  ; Update every 5 minutes
ServerStatusMessageFormat=Server: **{servername}** | Players: **{playercount}/{maxplayers}**
```

### In-Game Chat Settings

Customize how Discord messages appear in-game:

```ini
bEnableCustomChatColors=true
DiscordMessageColorR=173  ; Red component (0-255)
DiscordMessageColorG=216  ; Green component (0-255)
DiscordMessageColorB=230  ; Blue component (0-255)

bEnableInGameTimestamps=true
TimestampFormat=24h  ; Options: 12h, 24h, unix
```

### Security Settings

Protect your server with security features:

```ini
; Whitelist specific users (comma-separated Discord user IDs)
WhitelistedUserIDs=123456789,987654321

; Blacklist specific users
BlacklistedUserIDs=111111111,222222222

; Require a specific role to send messages to game
RequiredRoleID=YOUR_ROLE_ID

; Enable rate limiting
bEnableRateLimiting=true
RateLimitMessagesPerMinute=10
```

## Mod-Specific Settings

### Chat Commands

Edit `Mods/DiscordBotIntegration.ini`:

```ini
[ChatCommands]
bEnableDiscordChatCommands=true
DiscordInviteLink=https://discord.gg/your-invite
CommandCooldown=5
```

**Available Commands:**
- `/discord` - Show Discord server invite link
- `/discord help` - Show help for Discord commands
- `/discord status` - Show Discord bot status
- `/discord toggle` - Toggle Discord relay for yourself

### Player Preferences

Allow players to control their Discord integration:

```ini
[PlayerPreferences]
bAllowPlayerOptOut=true
bDefaultOptIn=true
bSavePlayerPreferences=true
```

### Notification Settings

Choose which game events trigger Discord notifications:

```ini
[Notifications]
bNotifyServerStart=true
bNotifyServerStop=true
bNotifyServerRestart=true
bNotifyAchievements=false
bNotifyPlayerDeaths=false
bNotifyMilestones=true
bNotifyResearchComplete=true
```

### Performance Tuning

Optimize bot performance for your server:

```ini
[Performance]
MessageQueueSize=100
MessageProcessInterval=30
bUseAsyncProcessing=true
MaxConcurrentRequests=5
```

## Example Configurations

### Minimal Setup (Game to Discord Only)

```ini
[/Script/DiscordBot.DiscordBotSettings]
DiscordWebhookURL=https://discord.com/api/webhooks/YOUR_WEBHOOK
MainChatChannelID=123456789
bEnableGameToDiscordRelay=true
bEnableDiscordToGameRelay=false
```

### Full Two-Way Communication

```ini
[/Script/DiscordBot.DiscordBotSettings]
DiscordBotToken=YOUR_BOT_TOKEN
DiscordServerID=123456789
MainChatChannelID=987654321
bEnableGameToDiscordRelay=true
bEnableDiscordToGameRelay=true
bEnablePlayerNotifications=true
bEnableDiscordCommands=true
```

### Security-Focused Setup

```ini
[/Script/DiscordBot.DiscordBotSettings]
DiscordBotToken=YOUR_BOT_TOKEN
DiscordServerID=123456789
MainChatChannelID=987654321
bEnableGameToDiscordRelay=true
bEnableDiscordToGameRelay=true
RequiredRoleID=VERIFIED_ROLE_ID
bEnableRateLimiting=true
RateLimitMessagesPerMinute=5
bEnableMessageFiltering=true
FilteredPrefixes=/,!,.,@
```

## Troubleshooting

### Bot Not Connecting

1. Verify your bot token is correct
2. Ensure the bot is invited to your server with proper permissions
3. Check `bEnableDebugLogging=true` in config
4. Review logs in `Logs/DiscordBot.log`

### Messages Not Relaying

1. Verify channel IDs are correct
2. Check bot has permission to read/send messages in channels
3. Ensure relay settings are enabled (`bEnableGameToDiscordRelay` / `bEnableDiscordToGameRelay`)
4. Check message filtering settings

### Rate Limiting Issues

If you see rate limit errors:
1. Increase `MessageProcessInterval`
2. Reduce `MaxConcurrentRequests`
3. Increase `ServerStatusUpdateInterval`

### Permission Errors

Required Discord bot permissions:
- Read Messages/View Channels
- Send Messages
- Embed Links
- Attach Files (for screenshots/logs)
- Read Message History
- Use External Emojis

## Support

For issues or questions:
1. Check the logs: `Logs/DiscordBot.log`
2. Enable debug logging: `bEnableDebugLogging=true`
3. Join the [Satisfactory Modding Discord](https://discord.gg/QzcG9nX)

## Advanced Topics

### Custom Message Formats

You can use multiple placeholders in message formats:

```ini
GameChatMessageFormat=🎮 [{time}] {server} | **{player}** ({playercount} online): {message}
PlayerJoinMessageFormat=➕ **{player}** joined! Welcome to {server}! ({playercount}/{maxplayers} players)
```

### Rich Embeds

Enable rich embed messages for better formatting:

```ini
bUseRichEmbeds=true
EmbedColor=#00FF00  ; Green color (hex format)
```

### Multiple Channel Support

Configure different channels for different purposes:

```ini
MainChatChannelID=123456789      ; Player chat
AdminChannelID=987654321         ; Admin notifications
LogChannelID=111222333           ; Detailed logs
```

## Configuration Validation

To validate your configuration:
1. Start the server
2. Check `Logs/DiscordBot.log` for any errors
3. Test by sending a message in-game
4. Test by sending a message in Discord
5. Verify bot status with `/discord status` in-game

## Best Practices

1. **Start Simple**: Begin with basic game→Discord relay, then add features
2. **Test Incrementally**: Enable one feature at a time to identify issues
3. **Security First**: Use whitelists/role requirements for public servers
4. **Monitor Performance**: Watch message queue and processing times
5. **Regular Backups**: Keep backups of your configuration files
6. **Rate Limiting**: Always enable rate limiting for public servers

## Updates and Maintenance

When updating the mod:
1. Backup your configuration files
2. Review changelog for new settings
3. Update configuration files with new options
4. Test in a development environment first
5. Monitor logs after deployment

---

**Version:** 1.0.0  
**Last Updated:** February 2026  
**Compatibility:** Satisfactory Mod Loader v3.11.3+
