# WebSocket Configuration Validator

This document helps you validate your WebSocket configuration before running the Discord Chat Bridge mod.

## Pre-Flight Checklist

Before starting your server, verify the following:

### 1. Discord Bot Configuration

- [ ] **Bot Token Set**: Your bot token is configured in the INI file (not empty)
- [ ] **Channel ID Set**: Your Discord channel ID is configured
- [ ] **Bot Added to Server**: Bot has been invited to your Discord server
- [ ] **Bot Permissions**: Bot has "Send Messages", "Read Messages", and "View Channels" permissions

### 2. Discord Developer Portal Settings

If using Gateway mode (`UseGatewayForPresence=true`), verify:

- [ ] **Presence Intent Enabled**: Go to Bot → Privileged Gateway Intents → ☑ PRESENCE INTENT
- [ ] **Message Content Intent Enabled**: Go to Bot → Privileged Gateway Intents → ☑ MESSAGE CONTENT INTENT

### 3. Configuration File

Location:
- Windows: `%localappdata%/FactoryGame/Saved/Config/WindowsServer/DiscordChatBridge.ini`
- Linux: `~/.config/Epic/FactoryGame/Saved/Config/LinuxServer/DiscordChatBridge.ini`

Verify:
- [ ] Configuration file exists
- [ ] `BotToken` is not empty
- [ ] `ChannelId` is not empty
- [ ] If using Gateway: `EnableBotActivity=true` and `UseGatewayForPresence=true`

### 4. Network Requirements

- [ ] Server can reach `https://discord.com/api/` (for REST API)
- [ ] Server can reach `wss://gateway.discord.gg` (for WebSocket Gateway)
- [ ] Firewall allows outbound WebSocket connections (port 443)
- [ ] No proxy blocking WebSocket traffic

## Configuration Validation Script

Copy this configuration to test your setup:

```ini
[/Script/DiscordChatBridge.DiscordChatSubsystem]
; ========== REQUIRED ==========
BotToken=YOUR_BOT_TOKEN_HERE
ChannelId=YOUR_CHANNEL_ID_HERE

; ========== TEST CONFIGURATION ==========
; Basic settings for testing
PollIntervalSeconds=2.0
EnableServerNotifications=true
ServerStartMessage=🧪 Test: Server Started

; Enable bot activity for testing
EnableBotActivity=true
ActivityUpdateIntervalSeconds=30.0

; ⭐ Enable Gateway WebSocket (THIS IS WHAT WE'RE TESTING!)
UseGatewayForPresence=true
GatewayPresenceFormat=with {playercount} players
GatewayActivityType=0

; Fallback settings if Gateway fails
BotActivityFormat=🎮 **Players Online:** {playercount}
```

## Expected Log Output

### Successful Gateway Connection

When everything is configured correctly, you should see these log messages:

```
[2024.XX.XX-XX.XX.XX:XXX] LogTemp: DiscordChatSubsystem: Initializing
[2024.XX.XX-XX.XX.XX:XXX] LogTemp: DiscordChatSubsystem: Configuration loaded - Channel ID: XXXX, Poll Interval: 2.0s, Notifications: Enabled, Bot Activity: Enabled, Gateway: Enabled
[2024.XX.XX-XX.XX.XX:XXX] LogTemp: DiscordAPI: Initialized with channel ID: XXXX
[2024.XX.XX-XX.XX.XX:XXX] LogTemp: DiscordAPI: Gateway presence enabled, creating Gateway connection
[2024.XX.XX-XX.XX.XX:XXX] LogTemp: DiscordGateway: Initialized
[2024.XX.XX-XX.XX.XX:XXX] LogTemp: DiscordGateway: Connecting to Discord Gateway...
[2024.XX.XX-XX.XX.XX:XXX] LogTemp: DiscordGateway: WebSocket connected, waiting for HELLO...
[2024.XX.XX-XX.XX.XX:XXX] LogTemp: DiscordGateway: Received HELLO, heartbeat interval: 41250 ms
[2024.XX.XX-XX.XX.XX:XXX] LogTemp: DiscordGateway: Heartbeat started (interval: 41.3 seconds, initial delay: XX.X seconds)
[2024.XX.XX-XX.XX.XX:XXX] LogTemp: DiscordGateway: Sending IDENTIFY...
[2024.XX.XX-XX.XX.XX:XXX] LogTemp: DiscordGateway: READY received, Session ID: XXXX
[2024.XX.XX-XX.XX.XX:XXX] LogTemp: DiscordAPI: Gateway connected successfully
[2024.XX.XX-XX.XX.XX:XXX] LogTemp: DiscordAPI: Updated bot presence via Gateway: Type=0, Text=with 0 players
```

✅ **SUCCESS**: If you see these logs, WebSockets are working perfectly!

### Common Error Patterns

#### Error 1: Missing Presence Intent

```
[LogTemp] Error: DiscordGateway: Connection closed - Code: 4014, Reason: Disallowed intent(s)
```

**Solution**: Enable "Presence Intent" in Discord Developer Portal → Bot Settings → Privileged Gateway Intents

#### Error 2: Invalid Bot Token

```
[LogTemp] Error: DiscordGateway: Connection closed - Code: 4004, Reason: Authentication failed
```

**Solution**: 
- Verify bot token is correct (copy from Discord Developer Portal)
- Ensure no extra spaces or quotes in configuration
- Check that token hasn't been regenerated

#### Error 3: Network/Firewall Issues

```
[LogTemp] Error: DiscordGateway: Connection error: Connection failed
```

**Solution**:
- Check firewall allows outbound connections to `wss://gateway.discord.gg`
- Verify server has internet connectivity
- Test: `curl https://discord.com/api/v10/gateway`

#### Error 4: Missing Configuration

```
[LogTemp] Warning: DiscordAPI: Cannot initialize Gateway - bot token is empty
```

**Solution**: Set `BotToken` in configuration file

#### Error 5: Gateway Not Enabled

```
[LogTemp] Log: DiscordAPI: Gateway presence disabled in configuration
```

**Solution**: Set `UseGatewayForPresence=true` in configuration file

## Manual Testing Steps

### Step 1: Start Server

1. Ensure configuration is complete
2. Start your Satisfactory dedicated server
3. Watch the server logs during startup

### Step 2: Verify Connection

Look for these success indicators:

1. **Log Message**: "Gateway connected successfully"
2. **Discord**: Bot status should show online (green dot)
3. **Discord**: Bot presence should show "Playing with 0 players"

### Step 3: Test Presence Updates

1. Join the game server
2. Wait up to `ActivityUpdateIntervalSeconds` (default: 60 seconds)
3. Check Discord - bot presence should update to "Playing with 1 player"
4. Have another player join
5. Bot presence should update to "Playing with 2 players"

### Step 4: Test Reconnection

1. Note: Gateway is connected
2. Temporarily block outbound connections (firewall test)
3. Wait for connection to drop
4. Log should show: "Gateway disconnected"
5. Restore connections
6. Log should show reconnection attempt
7. Verify: "Gateway connected successfully"

## Troubleshooting Commands

### Check Discord API Status
```bash
curl https://discordstatus.com/api/v2/status.json
```

### Test Gateway Endpoint
```bash
curl https://discord.com/api/v10/gateway
# Expected response: {"url":"wss://gateway.discord.gg"}
```

### Test Bot Token (Linux/Mac)
```bash
TOKEN="YOUR_BOT_TOKEN_HERE"
curl -H "Authorization: Bot $TOKEN" https://discord.com/api/v10/users/@me
# Expected: JSON response with bot user info
```

### Test Bot Token (PowerShell)
```powershell
$TOKEN = "YOUR_BOT_TOKEN_HERE"
$headers = @{ "Authorization" = "Bot $TOKEN" }
Invoke-RestMethod -Uri "https://discord.com/api/v10/users/@me" -Headers $headers
# Expected: Bot user info
```

## Configuration Verification Checklist

Copy and complete this checklist:

```
✅ Bot token is valid and not expired
✅ Channel ID is correct (right-click channel → Copy ID)
✅ Bot is in the Discord server
✅ Bot has permissions in the channel
✅ Presence Intent is enabled in Discord Developer Portal
✅ Message Content Intent is enabled in Discord Developer Portal
✅ Configuration file exists in correct location
✅ BotToken field is not empty
✅ ChannelId field is not empty
✅ EnableBotActivity=true
✅ UseGatewayForPresence=true
✅ Server has internet connectivity
✅ Firewall allows WebSocket connections
✅ No errors in server logs during startup
✅ Bot shows online in Discord
✅ Bot presence updates when players join/leave
```

## Support Resources

If you've verified everything and still have issues:

1. **Check Server Logs**: Look for error messages starting with "DiscordGateway:" or "DiscordAPI:"
2. **Review Documentation**: 
   - [WEBSOCKET_USAGE.md](WEBSOCKET_USAGE.md) - Comprehensive usage guide
   - [WEBSOCKET_EXAMPLES.md](WEBSOCKET_EXAMPLES.md) - Code examples
   - [SETUP_GUIDE.md](SETUP_GUIDE.md) - Setup instructions
3. **Discord API Status**: Check https://discordstatus.com for Discord outages
4. **GitHub Issues**: Search existing issues or create a new one
5. **Community Support**: Join the Satisfactory Modding Discord server

## Quick Reference: Configuration Values

### GatewayActivityType Values
- `0` = Playing (Shows: "Playing [text]")
- `1` = Streaming (Shows: "Streaming [text]")
- `2` = Listening (Shows: "Listening to [text]")
- `3` = Watching (Shows: "Watching [text]")
- `5` = Competing (Shows: "Competing in [text]")

### Recommended Settings
- `PollIntervalSeconds`: 2.0 (for REST API message polling)
- `ActivityUpdateIntervalSeconds`: 60-300 (for presence updates)
- Lower values = more responsive but higher API usage
- Higher values = less responsive but more efficient

## Success Indicators

Your WebSocket setup is working if:

✅ Server starts without errors
✅ Logs show "Gateway connected successfully"
✅ Bot appears online in Discord (green dot)
✅ Bot presence shows in member list
✅ Bot presence updates when players join/leave
✅ No repeated disconnection/reconnection in logs
✅ In-game chat messages appear in Discord
✅ Discord messages appear in-game

## Common Misconceptions

❌ **"WebSockets need to be built with the project"**
→ ✅ WebSockets is a UE plugin, builds with engine

❌ **"I need to add WebSockets to FactoryGame.Build.cs"**
→ ✅ Only mods using WebSockets need to add the dependency

❌ **"Gateway mode posts messages to Discord"**
→ ✅ Gateway updates bot presence, doesn't post messages

❌ **"REST API mode provides bot presence"**
→ ✅ REST API only posts messages, no presence updates

❌ **"I need a public IP for WebSockets"**
→ ✅ WebSocket is outbound connection, no inbound ports needed

## Conclusion

This validation guide helps ensure your WebSocket configuration is correct before running the server. By following the checklist and understanding the expected log output, you can quickly identify and resolve configuration issues.

**Remember**: WebSockets are already fully implemented and functional! This guide just helps you verify your specific configuration.
