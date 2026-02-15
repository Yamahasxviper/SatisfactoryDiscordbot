# Discord Chat Bridge for Satisfactory

A mod for Satisfactory that creates a two-way chat bridge between the in-game chat and Discord using the Discord Bot API.

## 🎮 For Players: Zero Installation Required!

**If you're a player connecting to a server with this mod:**
- ✅ You **DO NOT** need to install anything
- ✅ You **DO NOT** need SML on your client
- ✅ You **DO NOT** need any Discord OAuth setup
- ✅ Simply join the server and chat normally - messages sync automatically

**This mod is 100% server-side.** Only the server administrator needs to install and configure it.

> **Technical Note:** This mod uses `"RequiredOnRemote": false` in its plugin descriptor, which tells SML (Satisfactory Mod Loader) that clients can connect without having the mod installed. Combined with the `SpawnOnServer` replication policy in the code, this ensures the mod runs entirely on the server.

## Features

- ✅ Two-way chat synchronization between Satisfactory and Discord
- ✅ Uses Discord Bot Token (no webhooks required)
- ✅ Configurable through INI or TXT file formats
- ✅ **NEW: ServerDefaults TXT configuration** - Simple KEY=VALUE format with automatic SML persistence!
- ✅ Custom message formatting with player names
- ✅ Prevents message loops by ignoring bot messages
- ✅ Server-side only (no client installation required)
- ✅ **Customizable name formats** - Choose from multiple style presets or create your own!
- ✅ **Server start and stop notifications** - Get notified when the server goes online or offline with custom channel support!
- ✅ **Player count status updates** - Automatically post player count to Discord with customizable format and interval!
- ✅ **Discord Gateway bot presence** ⭐ - True "Playing with X players" status via WebSocket!
- ✅ **Bot online/offline status** ⭐ NEW! - Bot shows as online when server is running, offline when stopped!
- ✅ **Settings persist across server restarts** - Powered by SML's configuration system!

## Quick Links

- 🚀 [Quick Start Guide](QUICKSTART.md) - Get up and running in 5 minutes
- 📖 [Setup Guide](SETUP_GUIDE.md) - Detailed step-by-step instructions
- 🎨 [**Configuration Examples**](EXAMPLES.md) - **See different formatting styles and copy-paste configurations!**
- ⚙️ [**ServerDefaults Configuration**](ServerDefaults/README.md) - **NEW! Simple TXT format with SML persistence**
- ⚙️ [Default INI Configuration](Config/DefaultDiscordChatBridge.ini) - Traditional INI template configuration file
- 🔧 [Dependency Explanation](DEPENDENCY_EXPLANATION.md) - Technical details about WebSockets dependency configuration
- 📦 [Packaging Notes](PACKAGING_NOTES.md) - Build and packaging information (documentation files now included!)

> **Note:** All documentation files are now automatically included when you build/package this mod!

## Prerequisites

**For Server Administrators Only:**

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

The Discord Chat Bridge mod supports **two configuration formats**:

### Option 1: ServerDefaults TXT Format (Recommended) ⭐ NEW!

The easiest and most server-friendly option:

1. Navigate to `Mods/DiscordChatBridge/ServerDefaults/DiscordChatBridge.txt` in your project
2. Edit the file using simple `KEY=VALUE` format:

```txt
# Discord Chat Bridge Configuration
BotToken=YOUR_BOT_TOKEN_HERE
ChannelId=YOUR_CHANNEL_ID_HERE
PollIntervalSeconds=2.0
DiscordNameFormat=[Discord] {username}
GameNameFormat=**[{username}]** {message}
```

**Benefits:**
- ✅ Simple KEY=VALUE format (no complex INI sections)
- ✅ Automatically persists via SML configuration system
- ✅ Settings survive server restarts
- ✅ Extensive inline documentation
- ✅ No need to copy files to runtime directories

See [ServerDefaults/README.md](ServerDefaults/README.md) for complete documentation.

### Option 2: Traditional INI Format (Legacy)

1. Navigate to the mod's configuration file:
   - Windows: `%localappdata%/FactoryGame/Saved/Config/WindowsServer/DiscordChatBridge.ini`
   - Linux: `~/.config/Epic/FactoryGame/Saved/Config/LinuxServer/DiscordChatBridge.ini`
   
   If the file doesn't exist, create it or copy from one of these template files:
   - `Mods/DiscordChatBridge/Config/DefaultDiscordChatBridge.ini` (comprehensive template with all options)
   - `Config/WindowsServer/DiscordChatBridge.ini` (project template for Windows servers)
   - `Config/LinuxServer/DiscordChatBridge.ini` (project template for Linux servers)

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

### Discord Gateway Bot Presence ⭐ NEW!

For a true Discord bot presence (shows "Playing with X players" next to the bot's name in the member list), you can now enable Discord Gateway WebSocket support!

#### What is Gateway Mode?

Discord Gateway is a WebSocket-based connection that enables real-time bot presence updates. Instead of posting messages to a channel, your bot's status will show directly in Discord as:

```
🤖 YourBotName - Playing with 5 players
```

This appears:
- Next to the bot's name in the member list
- In the bot's user profile
- Exactly like official game integrations

**Bot Online/Offline Status:**
When using Gateway mode, the bot will automatically show as **online** when the server is running and **offline** when the server stops. This gives your community instant visual feedback about server availability without needing to check messages or channels.

#### Enabling Gateway Mode

Set `UseGatewayForPresence=true` in your configuration:

```ini
[/Script/DiscordChatBridge.DiscordChatSubsystem]
BotToken=YOUR_BOT_TOKEN_HERE
ChannelId=YOUR_CHAT_CHANNEL_ID

; Enable bot activity updates
EnableBotActivity=true

; ⭐ Enable Gateway for true bot presence
UseGatewayForPresence=true

; Customize presence text (optional)
GatewayPresenceFormat=with {playercount} players

; Customize activity type (optional, 0=Playing, 3=Watching, etc.)
GatewayActivityType=0

; How often to update presence
ActivityUpdateIntervalSeconds=60.0
```

#### Gateway vs REST API Comparison

| Feature | REST API Mode<br>(UseGatewayForPresence=false) | Gateway Mode<br>(UseGatewayForPresence=true) |
|---------|-------------------------------------------|----------------------------------------|
| **Display** | Channel messages | Bot presence status |
| **Visibility** | In chat channel | In member list & profile |
| **Format** | Customizable via BotActivityFormat | Customizable via GatewayPresenceFormat |
| **Activity Type** | N/A | Customizable (Playing, Watching, etc.) |
| **Connection** | HTTP requests only | Persistent WebSocket |
| **Resources** | Minimal | Slightly higher |
| **Reliability** | High (stateless) | High (auto-reconnects) |
| **Discord Feel** | Posts messages | Native integration |

#### Customizing Gateway Presence

When using Gateway mode, you can customize both the presence text and activity type:

**GatewayPresenceFormat** - Text that appears in the presence
- Use `{playercount}` placeholder for number of players
- Default: `with {playercount} players`
- Examples:
  - `with {playercount} players` → "Playing with 5 players"
  - `{playercount} online` → "Playing 5 online"
  - `Satisfactory | {playercount} building` → "Playing Satisfactory | 5 building"

**GatewayActivityType** - Type of activity shown
- `0` = Playing (default) - Shows as "Playing [text]"
- `1` = Streaming - Shows as "Streaming [text]"
- `2` = Listening - Shows as "Listening to [text]"
- `3` = Watching - Shows as "Watching [text]"
- `5` = Competing - Shows as "Competing in [text]"

**Example Configurations:**

```ini
# Default: "Playing with 5 players"
GatewayActivityType=0
GatewayPresenceFormat=with {playercount} players

# "Watching 5 players"
GatewayActivityType=3
GatewayPresenceFormat={playercount} players

# "Competing in Satisfactory"
GatewayActivityType=5
GatewayPresenceFormat=Satisfactory
```

#### Requirements for Gateway Mode

1. **Bot Permissions in Discord Developer Portal**:
   - Navigate to your bot in Discord Developer Portal
   - Go to "Bot" settings
   - Enable **"Presence Intent"** (required for Gateway)
   - Enable **"Message Content Intent"** (for reading messages)

2. **Configuration**:
   - Set `EnableBotActivity=true`
   - Set `UseGatewayForPresence=true`
   - Configure `GatewayPresenceFormat` (optional, has default)
   - Configure `GatewayActivityType` (optional, default is 0 = Playing)
   - Configure `ActivityUpdateIntervalSeconds` (60-300 seconds recommended)

#### Gateway Connection Details

When Gateway is enabled, the mod will:
1. Establish WebSocket connection to `wss://gateway.discord.gg`
2. Send IDENTIFY payload with your bot token
3. Maintain connection with periodic heartbeats (~40 seconds)
4. Send PRESENCE_UPDATE when player count changes
5. Auto-reconnect if connection drops

#### Troubleshooting Gateway Mode

**Bot presence doesn't show:**
- Check server logs for "Gateway connected successfully"
- Verify "Presence Intent" is enabled in Discord Developer Portal
- Ensure `EnableBotActivity=true` and `UseGatewayForPresence=true`
- Wait up to `ActivityUpdateIntervalSeconds` for first update

**Connection keeps dropping:**
- Check server logs for error messages
- Verify bot token is correct
- Ensure server has stable internet connection
- Gateway will automatically attempt to reconnect

**Bot shows offline:**
- Gateway connection may be disconnected
- Check logs for WebSocket errors
- Verify firewall allows WebSocket connections

#### Example Configuration

See `Config/ExampleConfigs/gateway-presence.ini` for a complete example configuration with Gateway mode enabled.

## Usage

Once configured and the server is running:

- **In-game to Discord**: Any message typed in the Satisfactory chat will appear in Discord as `**[PlayerName]** message text`
- **Discord to in-game**: Any message typed in the configured Discord channel will appear in-game with a blue "[Discord] Username" prefix
- **Server Notifications** (if enabled): The bot will send a notification to Discord when the server starts or stops
- **Player Count Updates** (if enabled): 
  - REST API mode: Posts to channel periodically
  - Gateway mode: Updates bot presence in real-time
- **Bot Online/Offline Status** (Gateway mode only): 
  - Bot shows as **online** when the server is running
  - Bot shows as **offline** when the server stops
  - Provides instant visual feedback about server availability

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

## Frequently Asked Questions (FAQ)

### Do players need to install this mod to join my server?

**No!** This is a **server-side only** mod. Players can join your server with a completely vanilla Satisfactory client (no mods, no SML). The chat bridge works automatically for all players.

**Technical Details:** The mod uses `"RequiredOnRemote": false` in its plugin descriptor, which tells SML that clients don't need the mod. Combined with the `SpawnOnServer` replication policy in the code, this ensures vanilla clients can connect and the mod only runs on the server.

### Do players need to install SML (Satisfactory Mod Loader)?

**No!** Only the server needs SML installed. Players can connect with the base game without any modifications.

### Do players need to set up Discord OAuth or a bot token?

**No!** Only the server administrator needs to create the Discord bot and configure the token. Players don't need any Discord developer setup - they just type in Discord or in-game and messages sync automatically.

### What does "OAuth" mean in the context of this mod?

OAuth is used **only by the server administrator** to create and authorize the Discord bot in the Discord Developer Portal. This is a one-time setup done on the server side. Regular players never interact with OAuth.

### Can players chat without Discord?

**Yes!** Players can:
- Chat in-game only (without Discord)
- Chat in Discord only (messages appear in-game)
- Chat in both places - they're independent

The mod simply bridges the two chats together. No Discord account is required to play on the server.

### What do I need to tell players who want to join?

Simply tell them:
1. The server address/IP
2. (Optional) The Discord channel URL if they want to chat from Discord

That's it! No installation instructions needed for players.

### I'm a player, what do I install?

**Nothing!** If you're connecting to a server that has this mod, you don't need to install anything. Just launch Satisfactory and join the server normally.

### What if the server rejects me saying I need mods?

If you're being rejected with a "missing mod" or "version mismatch" error:

1. **Verify it's this mod causing the issue** - The server may have OTHER mods that require client installation
2. **Check the server updated the mod** - The server needs version 1.0.0+ with `RequiredOnRemote: false` in the plugin descriptor
3. **Try after server restart** - The server may need to restart after updating the mod
4. **Check with server admin** - They may have other mods requiring client installation

This Discord Chat Bridge mod specifically does NOT require client installation when properly configured with `RequiredOnRemote: false`.

## Development

This mod is built using the Satisfactory Mod Loader (SML) framework.

### Building from Source

1. Clone the Satisfactory Mod Loader repository
2. Place this mod in the `Mods/` directory
3. Build the project using Unreal Engine with the SML project

**Build Requirements:**

This mod requires the **WebSockets** plugin, which is a built-in Unreal Engine plugin. If you encounter build errors about WebSockets being missing:

- See the main repository's [BUILD_REQUIREMENTS.md](../../BUILD_REQUIREMENTS.md) for detailed information
- The WebSockets plugin must be present in your Unreal Engine installation (it builds with the engine, not with this project)
- For local development, ensure you're using UE 5.3.2-CSS or a compatible version that includes WebSockets
- For automated builds, the CI workflow handles this automatically

**WebSockets Dependency:**

For technical details about how and why WebSockets is configured, see [DEPENDENCY_EXPLANATION.md](DEPENDENCY_EXPLANATION.md).

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
│       │   ├── DiscordGateway.h          # Discord Gateway WebSocket client
│       │   ├── DiscordChatSubsystem.h    # Main chat bridge subsystem
│       │   └── DiscordChatGameInstanceModule.h
│       ├── Private/
│       │   ├── DiscordChatBridgeModule.cpp
│       │   ├── DiscordAPI.cpp
│       │   ├── DiscordGateway.cpp        # Gateway WebSocket implementation
│       │   ├── DiscordChatSubsystem.cpp
│       │   └── DiscordChatGameInstanceModule.cpp
│       └── DiscordChatBridge.Build.cs
└── DiscordChatBridge.uplugin
```

## Technical Details

- **Polling Interval**: The mod polls Discord for new messages at a configurable interval (default: 2 seconds)
- **Message Loop Prevention**: Bot messages are automatically ignored to prevent infinite loops
- **Discord API Version**: Uses Discord API v10 (REST) and Gateway v10 (WebSocket)
- **Gateway Connection**: WebSocket to wss://gateway.discord.gg with automatic heartbeat and reconnection
- **Presence Updates**: Opcode 3 (PRESENCE_UPDATE) with activity type 0 (Playing)
- **Server-side Only**: The mod runs only on the server, no client installation needed
- **Discord API Version**: Uses Discord API v10

## Contributing

Contributions are welcome! Please feel free to submit issues or pull requests.

## License

Copyright (c) 2024 Discord Chat Bridge Contributors

## Credits

Created using the Satisfactory Mod Loader framework.
