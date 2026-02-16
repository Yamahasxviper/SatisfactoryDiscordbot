# Discord Chat Bridge - Setup Guide

## 🎯 Who Is This Guide For?

**This guide is for SERVER ADMINISTRATORS ONLY.**

### For Players:
If you're a player who wants to join a server with this mod:
- ✅ You need to do **NOTHING**
- ✅ No installation required
- ✅ No Discord setup required
- ✅ Just join the server and play normally

### For Server Administrators:
If you're setting up a Satisfactory server with Discord chat integration, follow this guide to:
- Install the mod on your server
- Create and configure a Discord bot
- Set up the chat bridge

---

## Quick Start Guide

### Step 1: Create a Discord Bot

1. Go to [Discord Developer Portal](https://discord.com/developers/applications)
2. Click **"New Application"** button
3. Give your application a name (e.g., "Satisfactory Server Bot")
4. Click **"Create"**

### Step 2: Configure Bot Settings

1. In your application, go to the **"Bot"** section from the left sidebar
2. Click **"Add Bot"** and confirm
3. Under the bot's username, find the **Token** section
4. Click **"Reset Token"** (if needed) then **"Copy"** to copy your bot token
   - **IMPORTANT:** Keep this token secret! Anyone with this token can control your bot
5. Scroll down to **"Privileged Gateway Intents"**
6. Enable the following intents:
   - ☑ **"Message Content Intent"** (required for reading Discord messages)
   - ☑ **"Presence Intent"** (required ONLY if you want to use Gateway/WebSocket features)
7. Click **"Save Changes"**

**About Presence Intent:** This is only needed if you plan to enable `UseGatewayForPresence=true` in your configuration to show bot status like "Playing with 5 players" next to the bot's name. If you're using REST API only (the default), Presence Intent is not required.

### Step 3: Set Bot Permissions

1. Go to the **"OAuth2"** section from the left sidebar
2. Click on **"URL Generator"**
3. Under **"Scopes"**, select:
   - ☑ `bot`
4. Under **"Bot Permissions"**, select:
   - ☑ `Send Messages`
   - ☑ `Read Messages/View Channels`
   - ☑ `Read Message History`
5. Copy the generated URL at the bottom
6. Open the URL in your browser and select the Discord server to invite the bot to
7. Authorize the bot

### Step 4: Get Your Channel ID

1. In Discord, go to **Settings > Advanced**
2. Enable **"Developer Mode"**
3. Right-click on the channel you want to use for the bridge
4. Click **"Copy ID"**
5. Save this ID for the next step

### Step 5: Install the Mod (Server Only)

**Important:** Install this mod **only on your dedicated server**. Players do not need to install anything.

1. Make sure [Satisfactory Mod Loader (SML)](https://ficsit.app/sml-versions) is installed **on your server**
2. Download or build the DiscordChatBridge mod
3. Place the mod folder in your **server's** Satisfactory mods directory:
   - **Windows Server:** `%localappdata%\FactoryGame\Saved\Mods\`
   - **Linux Server:** `~/.config/Epic/FactoryGame/Saved/Mods\`
   
**Note for Players:** If you're a player connecting to a server, you don't need to install anything in these directories. The mod is server-side only.

### Step 6: Configure the Mod

1. Navigate to the mod's configuration directory:
   - **Windows Server:** `%localappdata%\FactoryGame\Saved\Config\WindowsServer\`
   - **Linux Server:** `~/.config/Epic/FactoryGame/Saved/Config/LinuxServer/`

2. Create a file named `DiscordChatBridge.ini` with the following content:

   **Note:** If you've compiled the project from source, you can copy the template file from:
   - `Mods/DiscordChatBridge/Config/DefaultDiscordChatBridge.ini`
   
   The configuration is platform-agnostic - the same settings work on both Windows and Linux servers.
   
   Create a new file with the following content:

```ini
[/Script/DiscordChatBridge.DiscordChatSubsystem]
BotToken=YOUR_BOT_TOKEN_FROM_STEP_2
ChannelId=YOUR_CHANNEL_ID_FROM_STEP_4
PollIntervalSeconds=2.0
DiscordNameFormat=[Discord] {username}
GameNameFormat=**[{username}]** {message}
```

3. Replace `YOUR_BOT_TOKEN_FROM_STEP_2` with the token you copied in Step 2
4. Replace `YOUR_CHANNEL_ID_FROM_STEP_4` with the channel ID you copied in Step 4
5. (Optional) Customize the name formats - see "Advanced Configuration" section below

### Step 7: Start Your Server

1. Start your Satisfactory dedicated server
2. Check the server logs for messages like:
   ```
   DiscordChatBridge: Module Started
   DiscordChatSubsystem: Initializing
   DiscordChatSubsystem: Configuration loaded - Channel ID: [your ID], Poll Interval: 2.0s
   DiscordAPI: Initialized with channel ID: [your ID]
   DiscordAPI: Started polling for messages
   ```

### Step 8: Test the Bridge

1. **Test Game → Discord:**
   - Join your Satisfactory server
   - Type a message in the in-game chat
   - Check if it appears in your Discord channel

2. **Test Discord → Game:**
   - Type a message in your Discord channel
   - Check if it appears in the Satisfactory in-game chat with a blue "[Discord] Username" prefix

## Troubleshooting

### Bot isn't sending messages to Discord

**Check these common issues:**

1. **Invalid Bot Token**
   - Make sure you copied the entire token from the Discord Developer Portal
   - Token should be a long string like: `MTIzNDU2Nzg5MDEyMzQ1Njc4OQ.GhIjKl.MnOp...`
   - Don't include "Bot " prefix in the config file

2. **Invalid Channel ID**
   - Channel ID should be a number only
   - Make sure Developer Mode is enabled in Discord
   - Try copying the ID again

3. **Missing Permissions**
   - Make sure the bot has "Send Messages" permission in the channel
   - Check if the channel is visible to the bot's role

4. **Check Server Logs**
   - Look for error messages starting with "DiscordAPI:" or "DiscordChatSubsystem:"
   - Common errors will indicate what's wrong

### Bot isn't receiving messages from Discord

**Check these common issues:**

1. **Message Content Intent Not Enabled**
   - This is the most common issue!
   - Go to Discord Developer Portal > Your App > Bot
   - Enable "Message Content Intent"
   - Save changes and restart your Satisfactory server

2. **Bot Not in Server**
   - Make sure the bot is actually in your Discord server
   - You should see it in the member list

3. **Wrong Channel**
   - Double-check you're typing in the correct channel
   - The Channel ID in the config must match the channel you're using

### Messages aren't appearing in-game

**Check these common issues:**

1. **Polling Not Started**
   - Check server logs for "DiscordAPI: Started polling for messages"
   - If you don't see this, there might be a configuration issue

2. **Server Not Running**
   - The mod only works on dedicated servers, not in single-player

3. **Chat Manager Not Found**
   - Check logs for "Failed to get chat manager"
   - This might indicate a compatibility issue with the game version

### Configuration Changes Not Taking Effect

1. **Restart Required**
   - Configuration is only loaded when the server starts
   - You must restart the server after changing the config file

2. **Wrong File Location**
   - Make sure the config file is in the correct directory for your platform
   - The file must be named exactly `DiscordChatBridge.ini`
   - The file must be in the `WindowsServer` or `LinuxServer` subdirectory (not just `Config`)

### Rate Limiting

If you're getting rate limited by Discord:

1. **Increase Poll Interval**
   - Change `PollIntervalSeconds` to a higher value (e.g., 5.0)
   - This reduces the number of API calls to Discord

2. **Too Many Messages**
   - Discord has rate limits on sending messages
   - The bot handles this gracefully but may delay some messages

## Advanced Configuration

### Customizing Name Formats

You can customize how player names are displayed in both Discord and in-game chat by modifying the configuration file.

#### Discord Name Format

Controls how Discord usernames appear in the Satisfactory game chat.

**Setting**: `DiscordNameFormat`  
**Default**: `[Discord] {username}`  
**Placeholder**: `{username}` - Discord username

**Examples**:
```ini
; Default format - adds [Discord] prefix
DiscordNameFormat=[Discord] {username}
; Result: "[Discord] JohnDoe: Hello!"

; Simple brackets
DiscordNameFormat=<{username}>
; Result: "<JohnDoe>: Hello!"

; Suffix style
DiscordNameFormat={username} (Discord)
; Result: "JohnDoe (Discord): Hello!"

; With emoji
DiscordNameFormat=📱 {username}
; Result: "📱 JohnDoe: Hello!"
```

#### Game Name Format

Controls how Satisfactory player messages appear in Discord.

**Setting**: `GameNameFormat`  
**Default**: `**[{username}]** {message}`  
**Placeholders**: 
- `{username}` - Player name from Satisfactory
- `{message}` - The actual message content

**Examples**:
```ini
; Default format - bold player name in brackets
GameNameFormat=**[{username}]** {message}
; Result in Discord: "**[JohnDoe]** Hello world"

; Simple colon separator
GameNameFormat={username}: {message}
; Result in Discord: "JohnDoe: Hello world"

; Plain brackets
GameNameFormat=[{username}] {message}
; Result in Discord: "[JohnDoe] Hello world"

; With emoji and bold
GameNameFormat=🎮 **{username}**: {message}
; Result in Discord: "🎮 **JohnDoe**: Hello world"

; Italics style
GameNameFormat=_{username}_: {message}
; Result in Discord: "_JohnDoe_: Hello world"
```

**Note**: After changing these settings, you must restart your Satisfactory server for the changes to take effect.

### Optimal Poll Interval

The `PollIntervalSeconds` setting controls how often the mod checks Discord for new messages:

- **2.0 seconds** (default): Good balance, ~30 API calls per minute
- **1.0 seconds**: Most responsive, ~60 API calls per minute
- **5.0 seconds**: Conservative, ~12 API calls per minute

Lower values make the bridge more responsive but use more API calls. Discord's rate limits are generous, so 2.0 seconds is usually fine.

### Multiple Channels

Currently, the mod supports one channel per server. To use multiple channels:

1. Run multiple Satisfactory server instances
2. Configure each with a different channel ID
3. Alternatively, fork the mod and modify it to support multiple channels

## Security Notes

### Protecting Your Bot Token

- **Never** commit your bot token to version control
- **Never** share your bot token publicly
- Store the config file outside of any git repositories
- If your token is compromised, reset it immediately in the Discord Developer Portal

### Bot Permissions

Give your bot only the permissions it needs:
- ✅ Send Messages
- ✅ Read Message History
- ✅ View Channels
- ❌ Administrator (not needed)
- ❌ Manage Server (not needed)

## Getting Help

If you're still having issues:

1. Check the server logs thoroughly
2. Verify your Discord bot settings
3. Test your bot token with a simple Discord bot test (outside of Satisfactory)
4. Open an issue on the GitHub repository with:
   - Server logs (remove your bot token!)
   - Your configuration (remove your bot token!)
   - Description of the issue

## Next Steps

Once everything is working:

- Consider setting up automated server status messages
- Create custom commands for your bot
- Integrate with other Discord features
- Share your setup with other server admins!

Enjoy your Discord-Satisfactory chat bridge! 🎮💬
