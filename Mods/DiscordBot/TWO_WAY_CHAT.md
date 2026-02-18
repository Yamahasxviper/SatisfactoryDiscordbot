# Two-Way Chat Integration

This document describes the two-way chat integration feature between Satisfactory in-game chat and Discord.

## Overview

The Discord Bot mod now supports bidirectional chat relay:
- **Discord → Game**: Messages sent in configured Discord channels appear in-game chat
- **Game → Discord**: Player messages in-game are forwarded to configured Discord channels

## Features

✅ **Multiple Channel Support**: Configure multiple Discord channels for chat relay
✅ **Custom Sender Names**: Customize how Discord usernames appear in-game and vice versa
✅ **Bot Message Filtering**: Automatically filters out bot messages to prevent loops
✅ **Player Message Only**: Only relays player messages (not system/ADA messages) from game to Discord
✅ **Color-Coded Messages**: Discord messages appear in light blue color in-game for easy identification

## Configuration

All configuration is done in `Config/DiscordBot.ini`:

```ini
[DiscordBot]
; Discord Bot Token - Replace with your actual bot token
BotToken=YOUR_BOT_TOKEN_HERE

; Discord Channel ID for chat relay - Replace with your actual channel ID
ChatChannelId=YOUR_CHANNEL_ID_HERE

; Discord Gateway URL (default is fine for most cases)
GatewayURL=wss://gateway.discord.gg

; Enable/Disable the bot
bEnabled=true

; Log level (0=Error, 1=Warning, 2=Log, 3=Verbose)
LogLevel=2

; Two-Way Chat Integration Settings
; Enable two-way chat between Discord and in-game chat
bEnableTwoWayChat=true

; Custom sender name format for Discord messages in-game
; Available placeholders: {username}, {discriminator}
; Example: "[Discord] {username}" or "{username}#Discord"
DiscordSenderFormat=[Discord] {username}

; Custom sender name for in-game messages sent to Discord
; Leave empty to use player's actual name
GameSenderFormat={playername}
```

### Configuration Options

#### `bEnableTwoWayChat`
- **Type**: Boolean
- **Default**: `true`
- **Description**: Enable or disable the two-way chat integration

#### `ChatChannelId`
- **Type**: String (comma-separated) or Array
- **Default**: Empty
- **Description**: Discord channel IDs to relay messages to/from. Supports two formats:
  - **Comma-separated**: `ChatChannelId=123,456,789` (simpler for multiple channels)
  - **Array format**: `+ChatChannelId=` for each channel (backward compatible)
  - Both formats can be used together
- **Examples**:
  ```ini
  # Comma-separated (recommended for multiple channels)
  ChatChannelId=1234567890123456789,9876543210987654321
  
  # Array format (backward compatible)
  +ChatChannelId=1234567890123456789
  +ChatChannelId=9876543210987654321
  
  # Both formats together (will merge without duplicates)
  ChatChannelId=1234567890123456789,9876543210987654321
  +ChatChannelId=5555555555555555555
  ```

#### `DiscordSenderFormat`
- **Type**: String
- **Default**: `[Discord] {username}`
- **Description**: Format for Discord usernames displayed in-game
- **Placeholders**:
  - `{username}`: Discord username
  - `{discriminator}`: Discord discriminator (if applicable)

#### `GameSenderFormat`
- **Type**: String
- **Default**: `{playername}`
- **Description**: Format for player names displayed in Discord
- **Placeholders**:
  - `{playername}`: In-game player name

## Setup Instructions

### 1. Configure Discord Bot

Ensure your Discord bot has the following **Privileged Gateway Intents** enabled:
- ✅ **MESSAGE CONTENT INTENT** (Required for reading message content)
- ✅ **SERVER MEMBERS INTENT** (Required for member information)
- ✅ **PRESENCE INTENT** (Required for presence updates)

See [PRIVILEGED_INTENTS_GUIDE.md](PRIVILEGED_INTENTS_GUIDE.md) for detailed setup instructions.

### 2. Get Discord Channel IDs

1. Enable Developer Mode in Discord:
   - Settings → Advanced → Developer Mode
2. Right-click on a channel and select "Copy Channel ID"
3. Paste the channel ID in your config file

### 3. Configure the Mod

1. Open `Mods/DiscordBot/Config/DiscordBot.ini`
2. Set your bot token
3. Set `bEnableTwoWayChat=true`
4. Add your Discord channel IDs with `+ChatChannelId=`
5. Customize sender name formats if desired

### 4. Start the Server

The bot will automatically:
- Connect to Discord when the server starts
- Begin relaying messages between Discord and in-game chat
- Filter out its own messages to prevent loops

## How It Works

### Discord → Game Message Flow

1. Player sends message in Discord channel
2. Discord Gateway sends MESSAGE_CREATE event to bot
3. Bot filters out bot messages (including its own)
4. Bot checks if channel is in configured list
5. Bot formats username using `DiscordSenderFormat`
6. Bot broadcasts message to in-game chat via `AFGChatManager`
7. Message appears in-game with light blue color

### Game → Discord Message Flow

1. Player sends message in-game
2. `AFGChatManager` broadcasts message to clients
3. `UDiscordChatRelay` intercepts the message via `OnChatMessageAdded` event
4. Relay filters out non-player messages (system, ADA, custom)
5. Relay formats player name using `GameSenderFormat`
6. Relay sends message to Discord Bot Subsystem
7. Bot sends message to all configured Discord channels

## Technical Implementation

### Key Components

#### `UDiscordBotSubsystem`
- **Location**: `Source/DiscordBot/Private/DiscordBotSubsystem.cpp`
- **Purpose**: Main subsystem managing Discord connection and chat relay
- **Key Methods**:
  - `OnDiscordMessageReceived()`: Handles Discord messages and forwards to game
  - `OnGameChatMessage()`: Handles game messages and forwards to Discord
  - `LoadTwoWayChatConfig()`: Loads configuration from INI file

#### `ADiscordGatewayClientNative`
- **Location**: `Source/DiscordBot/Private/DiscordGatewayClientNative.cpp`
- **Purpose**: WebSocket connection to Discord Gateway
- **Key Methods**:
  - `HandleGatewayEvent()`: Routes Discord Gateway events
  - `HandleMessageCreate()`: Processes MESSAGE_CREATE events
  - Extracts message data and filters bot messages

#### `UDiscordChatRelay`
- **Location**: `Source/DiscordBot/Private/DiscordChatRelay.cpp`
- **Purpose**: Hooks into Satisfactory chat system
- **Key Methods**:
  - `Initialize()`: Binds to FGChatManager events
  - `OnChatMessageAdded()`: Intercepts new chat messages
  - Filters and forwards player messages only

### Message Filtering

The system implements several filtering mechanisms:

1. **Bot Message Filter**: Discord messages from bots are ignored (checks `bot` field and bot user ID)
2. **Empty Message Filter**: Empty messages are ignored
3. **Player Message Filter**: Only `CMT_PlayerMessage` type messages are relayed from game to Discord
4. **Channel Filter**: Only messages from configured channels are relayed

### Error Handling

- Graceful handling of missing chat manager
- Null checks for all subsystem references
- Logging at appropriate levels (Log, Warning, Error, Verbose)
- Automatic cleanup on subsystem deinitialization

## Troubleshooting

### Messages not appearing in-game

1. Check that `bEnableTwoWayChat=true` in config
2. Verify Discord channel ID is correct and in config
3. Check bot has MESSAGE CONTENT INTENT enabled
4. Check logs for "Discord message relayed to game" messages
5. Ensure FGChatManager is available (server-side only)

### Messages not appearing in Discord

1. Check bot is connected (look for "Bot ready!" in logs)
2. Verify Discord channel IDs are correct
3. Check bot has SEND MESSAGES permission in channel
4. Check logs for "Game message relayed to Discord" messages
5. Ensure ChatRelay is initialized

### Duplicate messages

- The bot filters its own messages using bot user ID
- Check that `BotUserId` is being set correctly in READY event

### Messages from wrong channels

- Only channels in `ChatChannelId` config are monitored
- Verify channel IDs are correct
- Check logs to see which channels are configured on startup

## Log Messages

Key log messages to look for:

### Startup
```
LogDiscordBotSubsystem: Discord Bot Subsystem initialized
LogDiscordBotSubsystem: Two-way chat enabled with X channel(s)
LogDiscordBotSubsystem:   - Channel ID: XXXXX
LogDiscordChatRelay: Discord Chat Relay initialized
```

### Discord → Game
```
LogDiscordGatewayNative: MESSAGE_CREATE: [CHANNEL_ID] Username: Message
LogDiscordBotSubsystem: Discord message relayed to game: [Discord] Username: Message
```

### Game → Discord
```
LogDiscordChatRelay: Relayed game message to Discord: [PlayerName] Message
LogDiscordBotSubsystem: Game message relayed to Discord: **PlayerName**: Message
```

## Performance Considerations

- Message relay adds minimal overhead (<1ms per message)
- WebSocket connection is persistent (no new connections per message)
- HTTP requests for Discord messages are asynchronous
- Chat relay uses event-driven architecture (no polling)

## Security Notes

- Bot token should never be committed to version control
- Use environment-specific config files
- Bot automatically filters its own messages to prevent loops
- Only player messages are relayed (system messages stay local)

## Future Enhancements

Potential future improvements:

- [ ] Rich message formatting (bold, italic, etc.)
- [ ] Discord embed support for in-game messages
- [ ] Message rate limiting
- [ ] Channel-specific formatting options
- [ ] User mention support
- [ ] Emoji support
- [ ] Message editing/deletion sync
- [ ] Thread support
- [ ] Per-channel enable/disable

## License

Copyright (c) 2024 Yamahasxviper
