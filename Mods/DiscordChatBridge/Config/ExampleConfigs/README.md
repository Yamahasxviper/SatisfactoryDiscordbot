# Example Configurations

This directory contains ready-to-use configuration file examples for different chat formatting styles.

> **💡 Note:** These examples use the traditional Config/ approach (Option 2). If you prefer the simpler config/ folder approach (Option 1 - Recommended), you can copy the relevant settings from these examples into `Mods/DiscordChatBridge/config/DiscordChatBridge.ini` instead. See [Which Config Should I Use?](../../README.md#-which-config-should-i-use) for guidance.

## How to Use

1. Choose the style that fits your server
2. Copy the `.ini` file to your server's configuration directory:
   - **Windows:** `%localappdata%\FactoryGame\Saved\Config\WindowsServer\DiscordChatBridge.ini`
   - **Linux:** `~/.config/Epic/FactoryGame/Saved/Config/LinuxServer/DiscordChatBridge.ini`
3. Edit the file and replace:
   - `YOUR_BOT_TOKEN_HERE` with your Discord bot token
   - `YOUR_CHANNEL_ID_HERE` with your Discord channel ID
4. Restart your Satisfactory server

## Available Styles

### `default.ini` - Standard Style ⭐ Recommended
- **In-game:** `[Discord] JohnDoe: Hello!`
- **Discord:** `**[PlayerName]** Hello!`
- Professional appearance with bold names in Discord

### `minimal.ini` - Clean & Simple
- **In-game:** `JohnDoe (from Discord): Hello!`
- **Discord:** `PlayerName: Hello!`
- Minimal formatting, easy to read

### `emoji.ini` - Fun with Emojis 🎮
- **In-game:** `📱 JohnDoe: Hello!`
- **Discord:** `🎮 **PlayerName**: Hello!`
- Visual icons distinguish message sources

### `irc-style.ini` - Classic Chat Style
- **In-game:** `<JohnDoe@Discord>: Hello!`
- **Discord:** `<PlayerName> Hello!`
- Traditional IRC-style with angle brackets

### `server-style.ini` - Community Server Style
- **In-game:** `[DISCORD] JohnDoe: Hello!`
- **Discord:** `🏭 [SERVER] **PlayerName**: Hello!`
- Professional branding for community servers

### `server-notifications.ini` - With Server Status Notifications 🔔
- Includes all features plus server start/stop notifications
- **Notifications:** `🟢 Satisfactory Server Online!` / `🔴 Server Offline`
- Supports separate notification channel
- Perfect for dedicated servers with status updates

### `player-count.ini` - With Player Count Status Updates 🎮
- Includes all features plus automatic player count updates
- **Status Updates:** `🎮 Players Online: 5`
- Posts player count to Discord at regular intervals
- Configurable update frequency and message format
- Optional separate status channel
- Perfect for keeping your community informed about server activity

### `gateway-presence.ini` - With Discord Gateway Bot Presence ⭐ NEW!
- **TRUE Discord bot presence via WebSocket Gateway**
- **Bot shows:** `Playing with 5 players` (next to bot name in member list)
- Real-time presence updates
- Professional Discord integration
- Persistent WebSocket connection
- More resource-intensive than REST-only mode
- **Perfect for:** Servers wanting authentic Discord bot presence
- **Requires:** Presence Intent enabled in Discord Developer Portal

## Need More?

Check out [EXAMPLES.md](../../help/EXAMPLES.md) in the help directory for:
- Detailed visual examples
- More customization ideas
- Tips and best practices
- Troubleshooting help
