# Quick Start - Discord Chat Bridge

## What You Need

1. **Satisfactory Dedicated Server** with SML 3.11.3+ installed
2. **Discord Bot** (we'll create this)
3. **5 minutes** to set everything up

## Setup Steps

### 1. Create Discord Bot (2 minutes)

1. Visit https://discord.com/developers/applications
2. Click **"New Application"**, name it (e.g., "Satisfactory Bot")
3. Go to **"Bot"** tab → Click **"Add Bot"**
4. Copy the **bot token** (you'll need this!)
5. Enable **"Message Content Intent"** under Privileged Gateway Intents
6. Save changes

### 2. Invite Bot to Your Server (1 minute)

1. Go to **"OAuth2"** → **"URL Generator"**
2. Select scope: **`bot`**
3. Select permissions: **`Send Messages`**, **`Read Messages`**, **`Read Message History`**
4. Copy the generated URL and open it
5. Select your Discord server and authorize

### 3. Get Channel ID (30 seconds)

1. In Discord: Settings → Advanced → Enable **Developer Mode**
2. Right-click your desired channel → **"Copy ID"**

### 4. Configure the Mod (1 minute)

Create/edit this file on your server:

**Windows:** `%localappdata%\FactoryGame\Saved\Config\WindowsServer\DiscordChatBridge.ini`  
**Linux:** `~/.config/Epic/FactoryGame/Saved/Config/LinuxServer/DiscordChatBridge.ini`

```ini
[/Script/DiscordChatBridge.DiscordChatSubsystem]
BotToken=paste_your_bot_token_here
ChannelId=paste_your_channel_id_here
PollIntervalSeconds=2.0

; Optional: Customize source labels and message formats (see EXAMPLES.md for more styles)
DiscordSourceLabel=Discord
GameSourceLabel=Game
DiscordNameFormat=[{source}] {username}
GameNameFormat=**[{username}]** {message}
```

**Want different formatting?** Check out [EXAMPLES.md](EXAMPLES.md) for emoji styles, minimal styles, and more!

### 5. Start Server

Restart your Satisfactory server. Check logs for:
```
DiscordChatBridge: Module Started
DiscordChatSubsystem: Configuration loaded
DiscordAPI: Started polling for messages
```

## Test It!

1. **Game → Discord:** Type in Satisfactory chat, see it in Discord
2. **Discord → Game:** Type in Discord, see it in-game with blue "[Discord]" prefix

## Troubleshooting

### Not working?

1. **Check bot token** - Make sure it's correct and no extra spaces
2. **Message Content Intent** - Must be enabled in Discord Developer Portal
3. **Bot in server** - Make sure bot is in your Discord server
4. **Correct channel** - Verify the Channel ID is correct
5. **Server logs** - Check for error messages

### Still stuck?

See [SETUP_GUIDE.md](SETUP_GUIDE.md) for detailed troubleshooting.

## That's It!

You now have two-way chat between Satisfactory and Discord! 🎉

---

**Need Help?** Check the [full documentation](README.md) or [setup guide](SETUP_GUIDE.md).
