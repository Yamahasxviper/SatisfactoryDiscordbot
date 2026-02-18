# Discord Bot for Satisfactory

A **server-side-only** Discord bot integration mod for Satisfactory that uses WebSocket communication with the following Discord Gateway intents:

> ⚠️ **Server-Side Mod**: This mod is marked as `"Remote": true` and is designed to run on the server only. **Clients do not need to install this mod** to join the server. The Discord bot functionality runs entirely server-side.

- **Guilds Intent** (1 << 0 = 1) - Basic guild events (required baseline)
- **Server Members Intent** (1 << 1 = 2) - PRIVILEGED - Allows the bot to receive member join/leave events
- **Presence Intent** (1 << 8 = 256) - PRIVILEGED - Allows the bot to receive presence updates
- **Message Content Intent** (1 << 15 = 32768) - PRIVILEGED - Allows the bot to read message content

Combined Intent Value: **33027** (includes baseline GUILDS + three privileged intents)

> 📖 **For detailed information about Privileged Gateway Intents, see [PRIVILEGED_INTENTS_GUIDE.md](PRIVILEGED_INTENTS_GUIDE.md)**

## Setup

1. **Create a Discord Bot**
   - Go to [Discord Developer Portal](https://discord.com/developers/applications)
   - Create a new application
   - Go to the "Bot" section and create a bot
   - Enable the following Privileged Gateway Intents:
     - PRESENCE INTENT
     - SERVER MEMBERS INTENT
     - MESSAGE CONTENT INTENT
   - Copy your bot token

2. **Configure the Mod**
   - Navigate to `Mods/DiscordBot/Config/DiscordBot.ini`
   - Replace `YOUR_BOT_TOKEN_HERE` with your actual bot token
   - Set `bEnabled=true` to enable the bot
   - **For Two-Way Chat**:
     - Set `bEnableTwoWayChat=true`
     - Add Discord channel IDs with `+ChatChannelId=YOUR_CHANNEL_ID`
     - Customize sender name formats (optional)
     - See [TWO_WAY_CHAT.md](TWO_WAY_CHAT.md) for full documentation
   - **For Server Status Notifications**:
     - Set `bEnableServerNotifications=true`
     - Set `NotificationChannelId=YOUR_CHANNEL_ID` (can be different from chat channels)
     - Customize `ServerStartMessage` and `ServerStopMessage` (optional)
     - Customize `BotPresenceMessage` to set bot status (optional)

3. **Build the Mod**
   - Build the Satisfactory project with the DiscordBot mod included
   - The mod will be compiled as part of the build process

## Usage

The Discord bot can be used in two ways:

### Automatic Initialization (Recommended)

The bot will automatically initialize and connect when the game starts if configured properly:

1. Set `bEnabled=true` in `Config/DiscordBot.ini`
2. Set your bot token in the config file
3. The bot will connect automatically on game start

### Manual Usage via Subsystem (Blueprint or C++)

#### Blueprint Usage

Access the Discord Bot Subsystem from any Blueprint:

1. Get the `DiscordBotSubsystem` from the Game Instance
2. Call `InitializeAndConnect` with your bot token (or it will auto-load from config)
3. Use `SendDiscordMessage` to send messages to Discord channels
4. Check connection status with `IsBotConnected`

#### C++ Usage

```cpp
// Get the subsystem
UDiscordBotSubsystem* BotSubsystem = GetWorld()->GetGameInstance()->GetSubsystem<UDiscordBotSubsystem>();

// Initialize and connect
BotSubsystem->InitializeAndConnect(TEXT("YOUR_BOT_TOKEN"));

// Send a message
BotSubsystem->SendDiscordMessage(TEXT("CHANNEL_ID"), TEXT("Hello from Satisfactory!"));

// Check connection
bool bConnected = BotSubsystem->IsBotConnected();
```

### Direct Actor Usage (Advanced)

You can also directly spawn and use the `ADiscordGatewayClient` actor:

```cpp
ADiscordGatewayClient* Client = GetWorld()->SpawnActor<ADiscordGatewayClient>();
Client->InitializeBot(TEXT("YOUR_BOT_TOKEN"));
Client->Connect();
Client->SendMessage(TEXT("CHANNEL_ID"), TEXT("Hello from Satisfactory!"));
```

## Features

- WebSocket connection to Discord Gateway
- Automatic heartbeat management
- Support for required intents (Presence, Server Members, Message Content)
- HTTP API integration for sending messages
- Blueprint and C++ support
- Game Instance Subsystem for easy access
- Automatic initialization on game start (configurable)
- Configuration file support for bot token and settings
- **🆕 Two-Way Chat Integration**: Relay messages between Discord and in-game chat
  - Multiple Discord channel support
  - Customizable sender name formats
  - Automatic bot message filtering
  - Color-coded messages in-game
  - See [TWO_WAY_CHAT.md](TWO_WAY_CHAT.md) for detailed documentation
- **🆕 Server Status Notifications**: Get notified when the server starts or stops
  - Separate notification channel for server events
  - Custom notification messages
  - Bot presence updates (online/offline status)
  - **Live player count display in bot status**
  - Configurable update interval for player count
  - Configurable notification settings

## Dependencies

- SML (Satisfactory Mod Loader) ^3.11.3
- CustomWebSocket plugin (for platform-agnostic WebSocket support)

**Important**: This mod is configured as a server-side-only mod (`"Remote": true` in the uplugin file). This means:
- Only the **server** needs this mod installed
- **Clients can join without having this mod** installed
- The Discord bot integration works entirely from the server side
- Compatible with both modded and non-modded clients joining the server

## Technical Details

The bot uses the Discord Gateway API v10 and establishes a WebSocket connection with the following intents:

```
Intents = (1 << 0) | (1 << 1) | (1 << 8) | (1 << 15)  // 1 + 2 + 256 + 32768 = 33027
```

### Discord Gateway Operations

- OPCODE 0 (DISPATCH): Receive events
- OPCODE 1 (HEARTBEAT): Keep connection alive
- OPCODE 2 (IDENTIFY): Authenticate with Discord
- OPCODE 10 (HELLO): Receive heartbeat interval
- OPCODE 11 (HEARTBEAT_ACK): Heartbeat acknowledged

## Notes

### WebSocket Implementation ⚠️

This mod uses the **CustomWebSocket plugin** which provides:
   - Custom WebSocket implementation (RFC 6455 compliant)
   - **Fully compatible** with Satisfactory's custom CSS Unreal Engine 5.3.2
   - Platform-agnostic WebSocket support (Win64, Linux, Mac, Dedicated Servers)
   - **Production ready** with complete Discord Gateway implementation
   - No dependency on Unreal's native WebSocket module
   - Only requires Sockets and OpenSSL (always available)

> **Note:** The `DiscordGatewayClient` class in the codebase is a reference implementation showing Discord Gateway protocol structure. It does NOT implement actual WebSocket connection and is kept for educational purposes only.

See [WEBSOCKET_COMPATIBILITY.md](WEBSOCKET_COMPATIBILITY.md) for detailed compatibility analysis.

### Security & Best Practices

- The bot token should be kept secure and not committed to version control
- Make sure your Discord bot has the necessary permissions in your server
- To extend functionality, modify the `HandleGatewayEvent` method in `DiscordGatewayClientNative.cpp`

## Server Status Notifications

The Discord bot can automatically send notifications to a designated channel when the Satisfactory server starts or stops, and update its online/offline presence status.

### Configuration

Add the following settings to `Config/DiscordBot.ini`:

```ini
; Server Status Notifications
bEnableServerNotifications=true

; Channel ID for server status notifications (can be separate from chat channels)
NotificationChannelId=YOUR_NOTIFICATION_CHANNEL_ID_HERE

; Custom message when server starts (optional)
ServerStartMessage=🟢 Satisfactory Server is now ONLINE!

; Custom message when server stops (optional)
ServerStopMessage=🔴 Satisfactory Server is now OFFLINE!

; Custom bot presence/status message (appears as "Playing <message>")
BotPresenceMessage=Satisfactory Server

; Show player count in bot presence (appears as "Playing <message> (X players)")
bShowPlayerCount=true

; Player count update interval in seconds (default: 30.0)
PlayerCountUpdateInterval=30.0
```

### Features

- **Separate Notification Channel**: Server status notifications can go to a different channel than chat messages
- **Custom Messages**: Personalize your server start/stop messages with emojis and custom text
- **Bot Presence Updates**: The bot's Discord status automatically changes to "online" when the server starts
- **Custom Status Message**: Configure what appears in the bot's "Playing" status
- **🆕 Player Count Display**: The bot status automatically shows the current number of players on the server
- **🆕 Configurable Update Interval**: Control how often the player count is updated

### How It Works

1. **On Server Start**:
   - The bot connects to Discord
   - Sends the configured `ServerStartMessage` to the notification channel
   - Updates its presence to show as "Playing <BotPresenceMessage> (X players)"
   - Status indicator shows as 🟢 online
   - Starts periodic player count updates based on `PlayerCountUpdateInterval`

2. **On Server Stop**:
   - Sends the configured `ServerStopMessage` to the notification channel
   - Stops player count updates
   - Bot disconnects and status shows as offline

3. **Player Count Updates**:
   - The bot automatically updates its presence every `PlayerCountUpdateInterval` seconds
   - Shows current player count in format: "Playing <BotPresenceMessage> (X players)"
   - Singular "player" when count is 1, plural "players" otherwise
   - Set `bShowPlayerCount=false` to disable player count display

### Getting Channel IDs

To find a Discord channel ID:
1. Enable Developer Mode in Discord (User Settings → Advanced → Developer Mode)
2. Right-click on the desired channel
3. Select "Copy ID"
4. Paste the ID into the configuration file

### Example Setup

```ini
[DiscordBot]
BotToken=YOUR_BOT_TOKEN_HERE
bEnabled=true
bEnableServerNotifications=true

; Notifications go to #server-status channel
NotificationChannelId=123456789012345678

; Chat goes to #satisfactory-chat channel
ChatChannelId=987654321098765432

ServerStartMessage=🎮 The factory is now open for business!
ServerStopMessage=🛑 The factory has shut down for maintenance
BotPresenceMessage=Factory Simulator 2024
bShowPlayerCount=true
PlayerCountUpdateInterval=30.0
```


## Production Considerations

The **CustomWebSocket plugin implementation is production-ready** with:

✅ **Implemented:**
- Custom WebSocket implementation using RFC 6455 protocol
- Complete Discord Gateway protocol (HELLO, IDENTIFY, HEARTBEAT, DISPATCH)
- Automatic heartbeat management
- Event handling system
- HTTP API integration for messages
- Proper error handling and connection management
- Platform-agnostic support (Win64, Linux, Mac, Dedicated Servers)

📋 **Optional Enhancements:**

1. **Reconnection Logic**: Add automatic reconnection with exponential backoff
2. **Extended Event Handlers**: Expand handlers for more Discord Gateway events
3. **Rate Limiting**: Implement rate limit handling for API requests
4. **Session Resume**: Implement session resumption for connection interruptions
5. **Presence Updates**: Add bot presence/status updates

## Engine Compatibility

✅ **Confirmed Compatible with:**
- Satisfactory Custom Unreal Engine 5.3.2-CSS
- Standard Unreal Engine 5.3.x
- Unreal Engine 5.4+ (forward compatible)

The CustomWebSocket plugin implementation:
- Works on ALL platforms (Win64, Linux, Mac, Dedicated Servers)
- No dependency on Unreal's WebSocket module
- Only requires core Unreal modules (Sockets, OpenSSL)
- Platform-independent implementation
- Compatible with all engine customizations

## Quick Start

For detailed setup instructions, see [SETUP.md](SETUP.md)

For information about Privileged Gateway Intents configuration, see [PRIVILEGED_INTENTS_GUIDE.md](PRIVILEGED_INTENTS_GUIDE.md)

## License

Copyright (c) 2024 Yamahasxviper
