# Discord Bot for Satisfactory

A Discord bot integration mod for Satisfactory that uses WebSocket communication with the following Discord Gateway intents:

- **Presence Intent** (1 << 8 = 256) - Allows the bot to receive presence updates
- **Server Members Intent** (1 << 1 = 2) - Allows the bot to receive member join/leave events
- **Message Content Intent** (1 << 15 = 32768) - Allows the bot to read message content

Combined Intent Value: **33026**

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

## Dependencies

- SML (Satisfactory Mod Loader) ^3.11.3
- SocketIOClient plugin (included in Plugins/WebSockets)

## Technical Details

The bot uses the Discord Gateway API v10 and establishes a WebSocket connection with the following intents:

```
Intents = (1 << 8) | (1 << 1) | (1 << 15)  // 256 + 2 + 32768 = 33026
```

### Discord Gateway Operations

- OPCODE 0 (DISPATCH): Receive events
- OPCODE 1 (HEARTBEAT): Keep connection alive
- OPCODE 2 (IDENTIFY): Authenticate with Discord
- OPCODE 10 (HELLO): Receive heartbeat interval
- OPCODE 11 (HEARTBEAT_ACK): Heartbeat acknowledged

## Notes

- The bot token should be kept secure and not committed to version control
- Make sure your Discord bot has the necessary permissions in your server
- The WebSocket implementation in this version is simplified; for production use, consider using a dedicated WebSocket library

## License

Copyright (c) 2024 Yamahasxviper
