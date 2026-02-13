# Discord Chat Bridge - Configuration Examples

This guide provides comprehensive examples of different configuration styles you can use with the Discord Chat Bridge mod.

## Quick Reference

All examples use these configuration settings in your `DiscordChatBridge.ini` file:

| Setting | Purpose | Placeholders |
|---------|---------|--------------|
| `DiscordSourceLabel` | Label for Discord messages source | N/A (text value) |
| `GameSourceLabel` | Label for game messages source | N/A (text value) |
| `DiscordNameFormat` | How Discord usernames appear in Satisfactory | `{source}`, `{username}` |
| `GameNameFormat` | How game messages appear in Discord | `{source}`, `{username}`, `{message}` |

**New!** The `{source}` placeholder allows you to use the configured source labels in your message formats, making it easy to change where messages appear to come from (e.g., "Discord" → "Chat", "Game" → "Server").

---

## Configuration Examples

### 1. Default Style (Recommended)

**Use Case:** Standard configuration with clear, professional formatting.

**Configuration:**
```ini
DiscordSourceLabel=Discord
GameSourceLabel=Game
DiscordNameFormat=[{source}] {username}
GameNameFormat=**[{username}]** {message}
```

**Example Output:**

```
In Satisfactory Game:
  [Discord] JohnDoe: Hey everyone, server restart in 5 minutes!
  [Discord] Alice: Thanks for the heads up!

In Discord:
  **[Steve]** Found a limestone deposit at coordinates 1234, 5678
  **[Bob]** On my way to help you mine it!
```

**Why use this?**
- Bold names in Discord make player names stand out
- Clear [Discord] prefix in-game distinguishes Discord messages
- Professional appearance

---

### 2. Minimal Style

**Use Case:** Clean, simple formatting without extra styling.

**Configuration:**
```ini
DiscordSourceLabel=Discord
GameSourceLabel=Server
DiscordNameFormat={username} (from {source})
GameNameFormat={username}: {message}
```

**Example Output:**

```
In Satisfactory Game:
  JohnDoe (from Discord): Hey everyone, server restart in 5 minutes!
  Alice (from Discord): Thanks for the heads up!

In Discord:
  Steve: Found a limestone deposit at coordinates 1234, 5678
  Bob: On my way to help you mine it!
```

**Why use this?**
- Minimal, unobtrusive formatting
- Easy to read
- Similar to standard chat applications

---

### 3. Emoji Style

**Use Case:** Fun, modern style with emoji icons to distinguish sources.

**Configuration:**
```ini
DiscordSourceLabel=💬
GameSourceLabel=🎮
DiscordNameFormat={source} {username}
GameNameFormat={source} **{username}**: {message}
```

**Example Output:**

```
In Satisfactory Game:
  💬 JohnDoe: Hey everyone, server restart in 5 minutes!
  💬 Alice: Thanks for the heads up!

In Discord:
  🎮 **Steve**: Found a limestone deposit at coordinates 1234, 5678
  🎮 **Bob**: On my way to help you mine it!
```

**Why use this?**
- Visual icons quickly identify message source
- Fun and modern appearance
- Emojis work in both Satisfactory and Discord
- Source labels can be pure emoji for a clean look!

**Emoji Ideas for Source Labels:**
- 💬 (speech bubble) - Discord/Chat messages
- 🎮 (game controller) - Game messages
- 📱 (mobile phone) - Discord messages
- 🏭 (factory) - Satisfactory messages
- 🌐 (globe) - External messages
- 🏭 (factory) - Satisfactory messages

---

### 4. IRC/Classic Style

**Use Case:** Traditional IRC-style chat formatting for nostalgic users.

**Configuration:**
```ini
DiscordSourceLabel=Discord
GameSourceLabel=Game
DiscordNameFormat=<{username}@{source}>
GameNameFormat=<{username}> {message}
```

**Example Output:**

```
In Satisfactory Game:
  <JohnDoe@Discord>: Hey everyone, server restart in 5 minutes!
  <Alice@Discord>: Thanks for the heads up!

In Discord:
  <Steve> Found a limestone deposit at coordinates 1234, 5678
  <Bob> On my way to help you mine it!
```

**Why use this?**
- Familiar to IRC/old-school chat users
- Consistent angle bracket format
- Clear username and source delineation

---

### 5. Server Announcement Style

**Use Case:** Professional community servers with multiple chat integrations.

**Configuration:**
```ini
DiscordSourceLabel=DISCORD
GameSourceLabel=SERVER
DiscordNameFormat=[{source}] {username}
GameNameFormat=🏭 [{source}] **{username}**: {message}
```

**Example Output:**

```
In Satisfactory Game:
  [DISCORD] JohnDoe: Hey everyone, server restart in 5 minutes!
  [DISCORD] Alice: Thanks for the heads up!

In Discord:
  🏭 [SERVER] **Steve**: Found a limestone deposit at coordinates 1234, 5678
  🏭 [SERVER] **Bob**: On my way to help you mine it!
```

**Why use this?**
- Professional appearance for community servers
- Server branding with factory emoji
- Clear message source identification
- Easy to change source labels (e.g., DISCORD → CHAT, SERVER → FACTORY)

---

## Advanced Customization Ideas

### Using the {source} Placeholder

The `{source}` placeholder is powerful because it separates the source label from the format:

```ini
; Change the source label without changing the format
DiscordSourceLabel=Community Chat
GameSourceLabel=Factory Server

; Format stays the same
DiscordNameFormat=[{source}] {username}
; Results in: "[Community Chat] JohnDoe"
```

This makes it easy to:
- Rebrand your server (Discord → Community, Game → MyServer)
- Support multiple languages (Discord → ディスコード)
- Use pure emoji as source labels (see Emoji Style above)
- Match your community's terminology

### Custom Source Labels

You can create custom source labels for your server:

```ini
; Short abbreviations
DiscordSourceLabel=DC
GameSourceLabel=SF

; Full names
DiscordSourceLabel=Community Discord
GameSourceLabel=Satisfactory Server

; With emojis
DiscordSourceLabel=💬 Discord
GameSourceLabel=🏭 Factory
DiscordNameFormat=[DC] {username}
GameNameFormat=[GAME] {username}: {message}
```

### Different Bracket Styles

Mix and match bracket types:

```ini
DiscordNameFormat=《{username}》
GameNameFormat=【{username}】{message}
```

### Combined Styles

Mix emojis with formatting:

```ini
DiscordNameFormat=💬 [{username}]
GameNameFormat=🎮 **「{username}」** {message}
```

### Markdown in Discord

Discord supports markdown formatting:

```ini
; Italics
GameNameFormat=_{username}_: {message}

; Bold
GameNameFormat=**{username}**: {message}

; Bold + Italics
GameNameFormat=**_{username}_**: {message}

; Code block
GameNameFormat=`{username}`: {message}
```

---

## Comparison Table

| Style | Source Labels | In-Game Appearance | Discord Appearance | Best For |
|-------|---------------|-------------------|-------------------|----------|
| **Default** | Discord/Game | `[Discord] Name: msg` | `**[Name]** msg` | Most users |
| **Minimal** | Discord/Server | `Name (from Discord): msg` | `Name: msg` | Clean look |
| **Emoji** | 💬/🎮 | `💬 Name: msg` | `🎮 **Name**: msg` | Fun servers |
| **IRC** | Discord/Game | `<Name@Discord>: msg` | `<Name> msg` | Classic users |
| **Server** | DISCORD/SERVER | `[DISCORD] Name: msg` | `🏭 [SERVER] **Name**: msg` | Communities |

---

## Tips and Best Practices

### 1. Keep It Readable
- Don't make names too long or complex
- Ensure there's clear separation between username and message
- Test your format before using it on your server

### 2. Consider Your Audience
- Gaming community? Use emoji style
- Professional server? Use server/default style
- Mixed audience? Use minimal style

### 3. Test in Both Places
After changing configuration:
1. Restart your Satisfactory server
2. Send a test message from Discord to game
3. Send a test message from game to Discord
4. Verify both look good

### 4. Unicode Support
Most emoji and Unicode characters work:
- ✅ Emojis: 🎮 💬 🏭 📱 ⚙️
- ✅ Special brackets: 【】《》「」
- ✅ Arrows and symbols: → ← ➜ ✦ ★

### 5. Discord Markdown
Discord supports these in your messages:
- `**bold**` → **bold**
- `*italic*` → *italic*
- `***bold italic***` → ***bold italic***
- `__underline__` → underline
- `` `code` `` → `code`

---

## Copy-Paste Ready Configurations

### For Quick Setup

Copy one of these complete configurations to your `DiscordChatBridge.ini`:

<details>
<summary>📋 Default Style</summary>

```ini
[/Script/DiscordChatBridge.DiscordChatSubsystem]
BotToken=YOUR_BOT_TOKEN_HERE
ChannelId=YOUR_CHANNEL_ID_HERE
PollIntervalSeconds=2.0
DiscordSourceLabel=Discord
GameSourceLabel=Game
DiscordNameFormat=[{source}] {username}
GameNameFormat=**[{username}]** {message}
```
</details>

<details>
<summary>📋 Emoji Style</summary>

```ini
[/Script/DiscordChatBridge.DiscordChatSubsystem]
BotToken=YOUR_BOT_TOKEN_HERE
ChannelId=YOUR_CHANNEL_ID_HERE
PollIntervalSeconds=2.0
DiscordSourceLabel=💬
GameSourceLabel=🎮
DiscordNameFormat={source} {username}
GameNameFormat={source} **{username}**: {message}
```
</details>

<details>
<summary>📋 Minimal Style</summary>

```ini
[/Script/DiscordChatBridge.DiscordChatSubsystem]
BotToken=YOUR_BOT_TOKEN_HERE
ChannelId=YOUR_CHANNEL_ID_HERE
PollIntervalSeconds=2.0
DiscordSourceLabel=Discord
GameSourceLabel=Server
DiscordNameFormat={username} (from {source})
GameNameFormat={username}: {message}
```
</details>

<details>
<summary>📋 IRC Style</summary>

```ini
[/Script/DiscordChatBridge.DiscordChatSubsystem]
BotToken=YOUR_BOT_TOKEN_HERE
ChannelId=YOUR_CHANNEL_ID_HERE
PollIntervalSeconds=2.0
DiscordSourceLabel=Discord
GameSourceLabel=Game
DiscordNameFormat=<{username}@{source}>
GameNameFormat=<{username}> {message}
```
</details>

---

## Troubleshooting

### Format Not Working?

1. **Check syntax** - Make sure placeholders are exactly `{username}` and `{message}`
2. **Restart server** - Configuration is loaded at startup
3. **Check logs** - Look for configuration errors in server logs

### Strange Characters?

1. **Encoding issues** - Save your INI file as UTF-8
2. **Font support** - Some emojis may not display in-game
3. **Discord renders fine** - Game display limitations don't affect Discord

### Can't See Changes?

1. **Server restart required** - Changes only apply after restart
2. **Clear game cache** - Sometimes old messages are cached
3. **Test with new messages** - Old messages won't update

---

## Server Notifications 🔔

The Discord Chat Bridge now supports server start and stop notifications!

### Basic Setup

Enable notifications in your configuration:

```ini
[/Script/DiscordChatBridge.DiscordChatSubsystem]
BotToken=YOUR_BOT_TOKEN_HERE
ChannelId=YOUR_CHANNEL_ID

; Enable server notifications
EnableServerNotifications=true

; Use default messages or customize below
ServerStartMessage=🟢 **Server Started** - The Satisfactory server is now online!
ServerStopMessage=🔴 **Server Stopped** - The Satisfactory server is now offline.
```

**Result:**
- When server starts: `🟢 **Server Started** - The Satisfactory server is now online!`
- When server stops: `🔴 **Server Stopped** - The Satisfactory server is now offline.`

### Separate Notification Channel

Send notifications to a different channel (e.g., a dedicated status channel):

```ini
; Chat messages go to this channel
ChannelId=YOUR_CHAT_CHANNEL_ID

; Enable notifications
EnableServerNotifications=true

; Notifications go to this channel
NotificationChannelId=YOUR_STATUS_CHANNEL_ID

; Custom messages
ServerStartMessage=✅ Factory is ONLINE! 🏭 Time to automate!
ServerStopMessage=⛔ Factory is OFFLINE 🔧 Maintenance in progress...
```

**Benefits:**
- Keep status updates separate from chat
- Easy for players to check server status
- No spam in the main chat channel

### Creative Notification Examples

#### Minimalist
```ini
ServerStartMessage=Server Online
ServerStopMessage=Server Offline
```

#### Enthusiastic
```ini
ServerStartMessage=🎉 **LET'S BUILD!** The factory awaits! 🏭⚙️
ServerStopMessage=😴 **Naptime!** Server's taking a break. Back soon! 💤
```

#### Professional
```ini
ServerStartMessage=🟢 **[ONLINE]** Satisfactory Server - All systems operational
ServerStopMessage=🔴 **[OFFLINE]** Satisfactory Server - Scheduled maintenance
```

#### Fun & Creative
```ini
ServerStartMessage=🚀 **BLAST OFF!** The conveyor belts are rolling! Let's automate! 🤖
ServerStopMessage=🛑 **FACTORY SHUTDOWN** - Time for the machines to rest! 😴
```

#### With Timestamps (Discord auto-adds these)
```ini
ServerStartMessage=🟢 Server is now ONLINE!
ServerStopMessage=🔴 Server has gone OFFLINE
```

### Tips for Notifications

1. **Keep it concise** - Players will see these frequently
2. **Use emojis** - Visual indicators are helpful (🟢🔴✅❌🏭)
3. **Be clear** - Make the status obvious at a glance
4. **Test both messages** - Make sure they look good in Discord
5. **Consider your audience** - Match your server's tone/style

### Important Notes

- ⚠️ **Server stop notification** may not be sent if the server crashes unexpectedly
- 🔑 **Bot permissions** - Make sure your bot has "Send Messages" in the notification channel
- 🔄 **Testing** - Restart your server to test the start notification
- 📝 **One channel is fine** - You don't need a separate notification channel (it's optional)

---

## Need More Help?

- Check [README.md](README.md) for full documentation
- See [SETUP_GUIDE.md](SETUP_GUIDE.md) for detailed setup instructions
- Visit [QUICKSTART.md](QUICKSTART.md) for fast setup

---

**Have a cool configuration?** Share it with the community! Open a PR or issue with your configuration style.
