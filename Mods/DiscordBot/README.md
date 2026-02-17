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

The Discord Gateway Client can be used in Blueprints or C++:

### Blueprint Usage

1. Add a `DiscordGatewayClient` actor to your level
2. Call `InitializeBot` with your bot token
3. Call `Connect` to establish the connection
4. Use `SendMessage` to send messages to Discord channels

### C++ Usage

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
