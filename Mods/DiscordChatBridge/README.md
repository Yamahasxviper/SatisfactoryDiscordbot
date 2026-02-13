# Discord Chat Bridge for Satisfactory

A mod for Satisfactory that creates a two-way chat bridge between the in-game chat and Discord using the Discord Bot API.

## Features

- ✅ Two-way chat synchronization between Satisfactory and Discord
- ✅ Uses Discord Bot Token (no webhooks required)
- ✅ Configurable through INI file
- ✅ Custom message formatting with player names
- ✅ Prevents message loops by ignoring bot messages
- ✅ Server-side only (no client installation required)

## Prerequisites

1. **Discord Bot Setup**
   - Go to [Discord Developer Portal](https://discord.com/developers/applications)
   - Click "New Application" and give it a name
   - Go to the "Bot" section and click "Add Bot"
   - Under the bot's token section, click "Copy" to get your bot token
   - Enable "Message Content Intent" in the bot settings (required to read messages)
   - Go to OAuth2 > URL Generator
   - Select scopes: `bot`
   - Select bot permissions: `Send Messages`, `Read Message History`, `Read Messages/View Channels`
   - Copy the generated URL and use it to invite the bot to your server

2. **Get Your Channel ID**
   - In Discord, enable Developer Mode (Settings > Advanced > Developer Mode)
   - Right-click on the channel you want to use and select "Copy ID"

## Installation

1. Install [Satisfactory Mod Loader (SML)](https://ficsit.app/sml-versions) if you haven't already
2. Download the `DiscordChatBridge` mod
3. Place the mod folder in your Satisfactory mods directory:
   - Windows: `%localappdata%/FactoryGame/Saved/Mods/`
   - Linux: `~/.config/Epic/FactoryGame/Saved/Mods/`

## Configuration

1. Navigate to the mod's configuration file:
   - Windows: `%localappdata%/FactoryGame/Saved/Config/WindowsServer/DiscordChatBridge.ini`
   - Linux: `~/.config/Epic/FactoryGame/Saved/Config/LinuxServer/DiscordChatBridge.ini`
   
   If the file doesn't exist, create it or copy from `Config/DefaultDiscordChatBridge.ini`

2. Edit the configuration file:

```ini
[/Script/DiscordChatBridge.DiscordChatSubsystem]
; Your Discord Bot Token
BotToken=YOUR_BOT_TOKEN_HERE

; Your Discord Channel ID
ChannelId=YOUR_CHANNEL_ID_HERE

; How often to check Discord for new messages (in seconds)
PollIntervalSeconds=2.0

; Format for Discord usernames when displayed in game
; Use {username} as placeholder for the Discord username
DiscordNameFormat=[Discord] {username}

; Format for game player messages when sent to Discord
; Use {username} for player name and {message} for the message content
GameNameFormat=**[{username}]** {message}
```

3. Replace `YOUR_BOT_TOKEN_HERE` with your bot token from the Discord Developer Portal
4. Replace `YOUR_CHANNEL_ID_HERE` with your channel ID
5. Optionally customize the name formats (see Customization section below)
6. Save the file and restart your Satisfactory server

### Customization

You can customize how player names appear in both Discord and in-game chat:

#### Discord Name Format
Controls how Discord usernames appear in the Satisfactory game chat.
- **Default**: `[Discord] {username}`
- **Examples**:
  - `[Discord] {username}` → Shows as "[Discord] JohnDoe"
  - `<{username}>` → Shows as "<JohnDoe>"
  - `{username} (Discord)` → Shows as "JohnDoe (Discord)"
  - `📱 {username}` → Shows as "📱 JohnDoe"

#### Game Name Format
Controls how Satisfactory player messages appear in Discord.
- **Default**: `**[{username}]** {message}`
- **Placeholders**: `{username}` for player name, `{message}` for message text
- **Examples**:
  - `**[{username}]** {message}` → Shows as "**[JohnDoe]** Hello world"
  - `{username}: {message}` → Shows as "JohnDoe: Hello world"
  - `[{username}] {message}` → Shows as "[JohnDoe] Hello world"
  - `🎮 **{username}**: {message}` → Shows as "🎮 **JohnDoe**: Hello world"

## Usage

Once configured and the server is running:

- **In-game to Discord**: Any message typed in the Satisfactory chat will appear in Discord as `**[PlayerName]** message text`
- **Discord to in-game**: Any message typed in the configured Discord channel will appear in-game with a blue "[Discord] Username" prefix

## Troubleshooting

### Messages aren't being sent/received

1. Check the server logs for errors related to `DiscordChatSubsystem` or `DiscordAPI`
2. Verify your bot token and channel ID are correct
3. Make sure the bot has been invited to your server and has permissions in the channel
4. Ensure "Message Content Intent" is enabled in your bot settings

### Bot token or channel ID not recognized

- Make sure there are no extra spaces or quotes in your configuration file
- The bot token should be a long string of characters (without "Bot " prefix in the config)
- The channel ID should be a number

### Permission errors

- Ensure your bot has the following permissions:
  - Send Messages
  - Read Message History
  - Read Messages/View Channels

## Development

This mod is built using the Satisfactory Mod Loader (SML) framework.

### Building from Source

1. Clone the Satisfactory Mod Loader repository
2. Place this mod in the `Mods/` directory
3. Build the project using Unreal Engine with the SML project

### File Structure

```
DiscordChatBridge/
├── Config/
│   └── DefaultDiscordChatBridge.ini    # Default configuration template
├── Source/
│   └── DiscordChatBridge/
│       ├── Public/
│       │   ├── DiscordChatBridgeModule.h
│       │   ├── DiscordAPI.h              # Discord API wrapper
│       │   ├── DiscordChatSubsystem.h    # Main chat bridge subsystem
│       │   └── DiscordChatGameInstanceModule.h
│       ├── Private/
│       │   ├── DiscordChatBridgeModule.cpp
│       │   ├── DiscordAPI.cpp
│       │   ├── DiscordChatSubsystem.cpp
│       │   └── DiscordChatGameInstanceModule.cpp
│       └── DiscordChatBridge.Build.cs
└── DiscordChatBridge.uplugin
```

## Technical Details

- **Polling Interval**: The mod polls Discord for new messages at a configurable interval (default: 2 seconds)
- **Message Loop Prevention**: Bot messages are automatically ignored to prevent infinite loops
- **Server-side Only**: The mod runs only on the server, no client installation needed
- **Discord API Version**: Uses Discord API v10

## Contributing

Contributions are welcome! Please feel free to submit issues or pull requests.

## License

Copyright (c) 2024 Discord Chat Bridge Contributors

## Credits

Created using the Satisfactory Mod Loader framework.
