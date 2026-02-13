# Example Configurations

This directory contains ready-to-use configuration file examples for different chat formatting styles.

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

## Need More?

Check out [EXAMPLES.md](../EXAMPLES.md) in the parent directory for:
- Detailed visual examples
- More customization ideas
- Tips and best practices
- Troubleshooting help
