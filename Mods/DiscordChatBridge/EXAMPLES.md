# Discord Chat Bridge - Configuration Examples

This guide provides comprehensive examples of different configuration styles you can use with the Discord Chat Bridge mod.

## Quick Reference

All examples use these configuration settings in your `DiscordChatBridge.ini` file:

| Setting | Purpose | Placeholders |
|---------|---------|--------------|
| `DiscordNameFormat` | How Discord usernames appear in Satisfactory | `{username}` |
| `GameNameFormat` | How game messages appear in Discord | `{username}`, `{message}` |

---

## Configuration Examples

### 1. Default Style (Recommended)

**Use Case:** Standard configuration with clear, professional formatting.

**Configuration:**
```ini
DiscordNameFormat=[Discord] {username}
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
DiscordNameFormat={username} (from Discord)
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
DiscordNameFormat=📱 {username}
GameNameFormat=🎮 **{username}**: {message}
```

**Example Output:**

```
In Satisfactory Game:
  📱 JohnDoe: Hey everyone, server restart in 5 minutes!
  📱 Alice: Thanks for the heads up!

In Discord:
  🎮 **Steve**: Found a limestone deposit at coordinates 1234, 5678
  🎮 **Bob**: On my way to help you mine it!
```

**Why use this?**
- Visual icons quickly identify message source
- Fun and modern appearance
- Emojis work in both Satisfactory and Discord

**Emoji Ideas:**
- 📱 (mobile phone) - Discord messages
- 🎮 (game controller) - Game messages
- 💬 (speech bubble) - Chat messages
- 🌐 (globe) - External messages
- 🏭 (factory) - Satisfactory messages

---

### 4. IRC/Classic Style

**Use Case:** Traditional IRC-style chat formatting for nostalgic users.

**Configuration:**
```ini
DiscordNameFormat=<{username}@Discord>
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
- Clear username delineation

---

### 5. Server Announcement Style

**Use Case:** Professional community servers with multiple chat integrations.

**Configuration:**
```ini
DiscordNameFormat=[DISCORD] {username}
GameNameFormat=🏭 [SERVER] **{username}**: {message}
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

---

## Advanced Customization Ideas

### Custom Prefixes

You can create custom prefixes for your server:

```ini
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

| Style | In-Game Appearance | Discord Appearance | Best For |
|-------|-------------------|-------------------|----------|
| **Default** | `[Discord] Name: msg` | `**[Name]** msg` | Most users |
| **Minimal** | `Name (from Discord): msg` | `Name: msg` | Clean look |
| **Emoji** | `📱 Name: msg` | `🎮 **Name**: msg` | Fun servers |
| **IRC** | `<Name@Discord>: msg` | `<Name> msg` | Classic users |
| **Server** | `[DISCORD] Name: msg` | `🏭 [SERVER] **Name**: msg` | Communities |

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
DiscordNameFormat=[Discord] {username}
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
DiscordNameFormat=📱 {username}
GameNameFormat=🎮 **{username}**: {message}
```
</details>

<details>
<summary>📋 Minimal Style</summary>

```ini
[/Script/DiscordChatBridge.DiscordChatSubsystem]
BotToken=YOUR_BOT_TOKEN_HERE
ChannelId=YOUR_CHANNEL_ID_HERE
PollIntervalSeconds=2.0
DiscordNameFormat={username} (from Discord)
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
DiscordNameFormat=<{username}@Discord>
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

## Need More Help?

- Check [README.md](README.md) for full documentation
- See [SETUP_GUIDE.md](SETUP_GUIDE.md) for detailed setup instructions
- Visit [QUICKSTART.md](QUICKSTART.md) for fast setup

---

**Have a cool configuration?** Share it with the community! Open a PR or issue with your configuration style.
