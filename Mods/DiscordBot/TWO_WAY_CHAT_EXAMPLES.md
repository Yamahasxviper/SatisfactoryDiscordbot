# Two-Way Chat Configuration Example

This file provides example configurations for the two-way chat feature.

## Basic Configuration (Single Channel)

```ini
[DiscordBot]
BotToken=YOUR_BOT_TOKEN_HERE
GatewayURL=wss://gateway.discord.gg
bEnabled=true
LogLevel=2

; Enable two-way chat
bEnableTwoWayChat=true

; Single Discord channel
ChatChannelId=1234567890123456789

; Simple sender formats
DiscordSenderFormat=[Discord] {username}
GameSenderFormat={playername}
```

## Multi-Channel Configuration (Comma-Separated - Recommended)

```ini
[DiscordBot]
BotToken=YOUR_BOT_TOKEN_HERE
GatewayURL=wss://gateway.discord.gg
bEnabled=true
LogLevel=2

; Enable two-way chat
bEnableTwoWayChat=true

; Multiple Discord channels (comma-separated)
ChatChannelId=1234567890123456789,9876543210987654321,5555555555555555555

; Simple sender formats
DiscordSenderFormat=[Discord] {username}
GameSenderFormat={playername}
```

## Multi-Channel Configuration (Array Format - Backward Compatible)

```ini
[DiscordBot]
BotToken=YOUR_BOT_TOKEN_HERE
GatewayURL=wss://gateway.discord.gg
bEnabled=true
LogLevel=2

; Enable two-way chat
bEnableTwoWayChat=true

; Multiple Discord channels (array format)
+ChatChannelId=1234567890123456789
+ChatChannelId=9876543210987654321
+ChatChannelId=5555555555555555555

; Simple sender formats
DiscordSenderFormat=[Discord] {username}
GameSenderFormat={playername}
```

## Multi-Channel Configuration (Mixed Format)

```ini
[DiscordBot]
BotToken=YOUR_BOT_TOKEN_HERE
GatewayURL=wss://gateway.discord.gg
bEnabled=true
LogLevel=2

; Enable two-way chat
bEnableTwoWayChat=true

; Multiple Discord channels (both formats work together)
ChatChannelId=1234567890123456789,9876543210987654321
+ChatChannelId=5555555555555555555
+ChatChannelId=7777777777777777777

; Simple sender formats
DiscordSenderFormat=[Discord] {username}
GameSenderFormat={playername}
```

## Custom Sender Name Formats

```ini
[DiscordBot]
BotToken=YOUR_BOT_TOKEN_HERE
GatewayURL=wss://gateway.discord.gg
bEnabled=true
LogLevel=2

; Enable two-way chat
bEnableTwoWayChat=true

; Discord channels
ChatChannelId=1234567890123456789

; Custom sender formats
DiscordSenderFormat=💬 {username}
GameSenderFormat=🎮 {playername}
```

## Professional Server Configuration

```ini
[DiscordBot]
BotToken=YOUR_BOT_TOKEN_HERE
GatewayURL=wss://gateway.discord.gg
bEnabled=true
LogLevel=2

; Enable two-way chat
bEnableTwoWayChat=true

; Main chat channel
ChatChannelId=1234567890123456789

; Professional formatting
DiscordSenderFormat=[D] {username}
GameSenderFormat=[Game] {playername}
```

## Disable Two-Way Chat (Only Send from Game to Discord)

If you want to disable Discord → Game messages but keep Game → Discord:

Note: This is not directly supported, but you can:
1. Set `bEnableTwoWayChat=true` to enable game → Discord
2. Mute the bot in-game channels to prevent it from sending messages
3. Or modify the code to add a one-way mode

## Getting Channel IDs

To get a Discord channel ID:

1. Enable Developer Mode in Discord:
   - User Settings → Advanced → Developer Mode (toggle on)

2. Right-click on the channel you want to use
   - Select "Copy Channel ID"

3. Paste the ID into your config file:
   ```ini
   ChatChannelId=PASTE_HERE
   # Or for multiple channels:
   ChatChannelId=PASTE_HERE,ANOTHER_ID,THIRD_ID
   ```
   ```

## Testing Your Configuration

1. Start your Satisfactory server with the mod
2. Check the logs for:
   ```
   LogDiscordBotSubsystem: Two-way chat enabled with X channel(s)
   LogDiscordBotSubsystem:   - Channel ID: YOUR_CHANNEL_ID
   ```
3. Send a test message in Discord
4. Check in-game if the message appears
5. Send an in-game chat message
6. Check Discord if the message appears

## Troubleshooting

### Channel ID not working

- Make sure you copied the full ID (should be 18-19 digits)
- Verify the bot has access to the channel
- Check the bot has "Read Messages" and "Send Messages" permissions

### Messages not appearing

- Verify `bEnableTwoWayChat=true`
- Check bot is connected (look for "Bot ready!" in logs)
- Verify MESSAGE CONTENT INTENT is enabled for your bot

### Format not applying

- Make sure there are no extra spaces in the format string
- Placeholders are case-sensitive: use `{username}` not `{Username}`
- Restart the server after config changes

## Advanced Configuration

### Different Formats for Different Scenarios

You can only have one format per direction, but you can make them descriptive:

```ini
; Make Discord users stand out
DiscordSenderFormat=🌐 [Discord] {username}

; Show in-game players with a game icon
GameSenderFormat=🎮 {playername}
```

### Minimal Formatting

For a clean look:

```ini
DiscordSenderFormat={username}
GameSenderFormat={playername}
```

This will show messages as if they're from the same system.

## Notes

- Channel IDs can be specified using comma-separated format: `ChatChannelId=123,456,789`
- Or using array format: `+ChatChannelId=123` (for backward compatibility)
- Both formats can be used together
- You can add as many channels as you want
- All configured channels will receive game messages
- All configured channels will relay to game
- The bot filters its own messages automatically
- Only player messages are sent from game to Discord (no system messages)
