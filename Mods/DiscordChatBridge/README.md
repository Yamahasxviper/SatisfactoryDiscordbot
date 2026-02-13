# Discord Chat Bridge for Satisfactory

A mod for Satisfactory that creates a two-way chat bridge between the in-game chat and Discord using the Discord Bot API.

## Features

- ✅ Two-way chat synchronization between Satisfactory and Discord
- ✅ Uses Discord Bot Token (no webhooks required)
- ✅ Configurable through INI file
- ✅ Custom message formatting with player names
- ✅ Prevents message loops by ignoring bot messages
- ✅ Server-side only (no client installation required)
- ✅ **Customizable name formats** - Choose from multiple style presets or create your own!
- ✅ **Server start and stop notifications** - Get notified when the server goes online or offline with custom channel support!
- ✅ **Player count status updates** - Automatically post player count to Discord with customizable format and interval!

## Quick Links

- 🚀 [Quick Start Guide](QUICKSTART.md) - Get up and running in 5 minutes
- 📖 [Setup Guide](SETUP_GUIDE.md) - Detailed step-by-step instructions
- 🎨 [**Configuration Examples**](EXAMPLES.md) - **See different formatting styles and copy-paste configurations!**
- ⚙️ [Default Configuration](Config/DefaultDiscordChatBridge.ini) - Template configuration file

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

; Source labels (what appears as the message origin)
DiscordSourceLabel=Discord
GameSourceLabel=Game

; Format for Discord usernames when displayed in game
; Use {source} for the source label, {username} for the Discord username
DiscordNameFormat=[{source}] {username}

; Format for game player messages when sent to Discord
; Use {source} for the source label, {username} for player name, {message} for content
GameNameFormat=**[{username}]** {message}

; ========== Server Status Notifications ==========
; Enable server start and stop notifications
EnableServerNotifications=false

; Discord Channel ID for server notifications (optional, uses ChannelId if empty)
NotificationChannelId=

; Message format for server start notification
ServerStartMessage=🟢 **Server Started** - The Satisfactory server is now online!

; Message format for server stop notification
ServerStopMessage=🔴 **Server Stopped** - The Satisfactory server is now offline.
```

3. Replace `YOUR_BOT_TOKEN_HERE` with your bot token from the Discord Developer Portal
4. Replace `YOUR_CHANNEL_ID_HERE` with your channel ID
5. Optionally customize the source labels and name formats (see Customization section below)
6. Optionally enable server notifications (see Server Notifications section below)
7. Save the file and restart your Satisfactory server

### Customization

You can customize how player names appear in both Discord and in-game chat:

#### Source Labels
Control what appears as the message origin.
- **DiscordSourceLabel**: Label for Discord messages (default: "Discord")
- **GameSourceLabel**: Label for game messages (default: "Game")
- **Examples**:
  - `Discord` / `Game` → Standard labels
  - `💬` / `🎮` → Pure emoji labels
  - `Community` / `Server` → Custom branding
  - `DC` / `SF` → Short abbreviations

#### Discord Name Format
Controls how Discord usernames appear in the Satisfactory game chat.
- **Default**: `[{source}] {username}`
- **Placeholders**: `{source}` for source label, `{username}` for Discord username
- **Examples**:
  - `[{source}] {username}` → Shows as "[Discord] JohnDoe"
  - `<{username}@{source}>` → Shows as "<JohnDoe@Discord>"
  - `{username} (from {source})` → Shows as "JohnDoe (from Discord)"
  - `{source} {username}` → Shows as "💬 JohnDoe" (if using emoji source label)

#### Game Name Format
Controls how Satisfactory player messages appear in Discord.
- **Default**: `**[{username}]** {message}`
- **Placeholders**: `{source}` for source label, `{username}` for player name, `{message}` for message text
- **Examples**:
  - `**[{username}]** {message}` → Shows as "**[JohnDoe]** Hello world"
  - `{username}: {message}` → Shows as "JohnDoe: Hello world"
  - `[{username}] {message}` → Shows as "[JohnDoe] Hello world"
  - `🎮 **{username}**: {message}` → Shows as "🎮 **JohnDoe**: Hello world"

### Server Notifications

Get notified in Discord when your Satisfactory server starts or stops!

#### Enabling Server Notifications

Set `EnableServerNotifications=true` in your configuration file to enable this feature.

#### Notification Channel

By default, server notifications are sent to the same channel as chat messages. You can optionally configure a separate notification channel:

- **Same as chat channel** (default): Leave `NotificationChannelId` empty
- **Separate notification channel**: Set `NotificationChannelId` to a different channel ID (e.g., for a dedicated server status channel)

Make sure your bot has permissions in both channels if using separate channels.

#### Customizing Notification Messages

You can customize the messages sent when the server starts or stops:

- **ServerStartMessage**: Message sent when the server starts
  - Default: `🟢 **Server Started** - The Satisfactory server is now online!`
  - Examples:
    - `✅ Server is UP! Time to build! 🏭`
    - `🟢 Production has resumed!`
    - `Server online - Let's automate everything! 🤖`

- **ServerStopMessage**: Message sent when the server stops
  - Default: `🔴 **Server Stopped** - The Satisfactory server is now offline.`
  - Examples:
    - `❌ Server is DOWN for maintenance 🔧`
    - `🔴 Production has stopped!`
    - `Server offline - Back soon! 💤`

#### Example Configuration

```ini
[/Script/DiscordChatBridge.DiscordChatSubsystem]
BotToken=YOUR_BOT_TOKEN_HERE
ChannelId=YOUR_CHAT_CHANNEL_ID

; Enable server notifications
EnableServerNotifications=true

; Optional: Use a separate channel for notifications
NotificationChannelId=YOUR_NOTIFICATION_CHANNEL_ID

; Custom notification messages
ServerStartMessage=🟢 **Satisfactory Server Online!** Ready to build! 🏭
ServerStopMessage=🔴 **Satisfactory Server Offline** - Scheduled maintenance 🔧
```

### Player Count Status Updates

Keep your Discord community informed about server activity with automatic player count updates!

#### Enabling Player Count Updates

Set `EnableBotActivity=true` in your configuration file to enable this feature.

#### Update Interval

Configure how often the player count is updated (in seconds):
- **ActivityUpdateIntervalSeconds**: Time between updates (default: 60 seconds)
- **Recommended**: 60-300 seconds to avoid spam

#### Status Channel

By default, player count updates are posted to the same channel as chat messages. You can optionally configure a separate status channel:

- **Same as chat channel** (default): Leave `BotActivityChannelId` empty
- **Separate status channel**: Set `BotActivityChannelId` to a different channel ID (e.g., for a dedicated server status channel)

Make sure your bot has "Send Messages" permission in the status channel.

#### Customizing Status Messages

You can customize the format of player count messages using the `BotActivityFormat` setting:

- **Default**: `🎮 **Players Online:** {playercount}`
- **Placeholder**: `{playercount}` will be replaced with the actual number of players
- **Examples**:
  - `🎮 **Players Online:** {playercount}`
  - `👥 Current Players: {playercount}`
  - `Server has {playercount} player(s) online`
  - `🏭 Building with {playercount} engineers!`

#### Example Configuration

```ini
[/Script/DiscordChatBridge.DiscordChatSubsystem]
BotToken=YOUR_BOT_TOKEN_HERE
ChannelId=YOUR_CHAT_CHANNEL_ID

; Enable player count updates
EnableBotActivity=true

; Update every 2 minutes
ActivityUpdateIntervalSeconds=120.0

; Optional: Use a separate channel for status updates
BotActivityChannelId=YOUR_STATUS_CHANNEL_ID

; Custom status message format
BotActivityFormat=🎮 **Players Online:** {playercount} | 🏭 Let's build!
```

#### Note on Implementation

The current implementation posts player count updates as messages to a Discord channel. For a true "Playing with X players" status in the bot's profile (like you see with game integrations), Discord Gateway WebSocket would be required, which is not currently implemented in this REST API-based mod. The message-based approach provides the same information in a visible and configurable way.

## Usage

Once configured and the server is running:

- **In-game to Discord**: Any message typed in the Satisfactory chat will appear in Discord as `**[PlayerName]** message text`
- **Discord to in-game**: Any message typed in the configured Discord channel will appear in-game with a blue "[Discord] Username" prefix
- **Server Notifications** (if enabled): The bot will send a notification to Discord when the server starts or stops
- **Player Count Updates** (if enabled): The bot will periodically post the current player count to the configured channel

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
- If using a separate notification channel, ensure the bot has permissions in both channels

### Server notifications not appearing

1. Verify that `EnableServerNotifications` is set to `true` in your configuration
2. Check the server logs for notification-related messages
3. If using a separate notification channel, verify the `NotificationChannelId` is correct
4. Ensure your bot has "Send Messages" permission in the notification channel
5. Note that the server stop notification may not be sent if the server crashes unexpectedly

### Player count updates not appearing

1. Verify that `EnableBotActivity` is set to `true` in your configuration
2. Check the server logs for player count update messages
3. If using a separate status channel, verify the `BotActivityChannelId` is correct
4. Ensure your bot has "Send Messages" permission in the status channel
5. Check that `ActivityUpdateIntervalSeconds` is set to a reasonable value (60-300 seconds recommended)
6. The first update should appear immediately when the server starts

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
