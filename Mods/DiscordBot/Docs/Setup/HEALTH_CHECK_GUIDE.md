# Health Check & Status Monitoring Guide

This guide explains how to use the Discord bot's health check and status monitoring features to remotely monitor your Satisfactory server's connection status and internet connectivity.

## Overview

The health check feature allows you to remotely check if:
- **WebSocket is connected**: Verify the Discord bot's WebSocket connection to Discord is active
- **Internet connectivity**: Confirm the server has internet access
- **Server status**: Get current player count and uptime information

This is particularly useful when you need to monitor a server that you don't have direct access to from home.

## Features

### 1. Manual Health Check Command

Users can type commands in Discord to get instant server status:

- **`!status`** - Get current server health status
- **`!health`** - Same as `!status` (alternative command)
- **`!help`** - Show available bot commands

**Example Response:**
```
🏥 Server Health Status

✅ WebSocket Status: Connected
✅ Internet Connectivity: Available
👥 Players Online: 3
⏱️ Session Uptime: 2h 45m

Last checked: 2026-02-19 15:30:45 UTC
```

### 2. Periodic Health Check Alerts

The bot can automatically monitor connection status and send alerts when issues are detected. This prevents channel spam by only sending messages when there's a problem.

**Alert Example:**
```
⚠️ Health Check Alert

❌ WebSocket connection lost!

Alert time: 2026-02-19 15:30:45 UTC
```

## Configuration

Add these settings to your `Config/DiscordBot.ini` file:

```ini
[DiscordBot]

; ===== Health Check / Status Monitoring =====

; Enable bot command for health checks (!status or !health command)
bEnableHealthCheckCommand=true

; Enable periodic health check alerts
; Only sends alerts when issues are detected (no spam)
bEnablePeriodicHealthCheck=false

; Health check interval in seconds (default: 300 = 5 minutes)
HealthCheckInterval=300.0

; Channel ID for periodic health check alerts
HealthCheckChannelId=YOUR_CHANNEL_ID_HERE
```

### Configuration Options

| Setting | Type | Default | Description |
|---------|------|---------|-------------|
| `bEnableHealthCheckCommand` | Boolean | `true` | Enable `!status` and `!health` commands |
| `bEnablePeriodicHealthCheck` | Boolean | `false` | Enable automatic health monitoring |
| `HealthCheckInterval` | Float | `300.0` | How often to check (in seconds) |
| `HealthCheckChannelId` | String | - | Discord channel ID for alerts |

## Usage Examples

### Example 1: Manual Status Checks Only

If you just want to be able to manually check server status:

```ini
bEnableHealthCheckCommand=true
bEnablePeriodicHealthCheck=false
```

Then users can type `!status` in any configured chat channel to get the current status.

### Example 2: Automatic Monitoring

If you want automatic alerts when connection issues occur:

```ini
bEnableHealthCheckCommand=true
bEnablePeriodicHealthCheck=true
HealthCheckInterval=300.0
HealthCheckChannelId=123456789012345678
```

The bot will check every 5 minutes (300 seconds) and send alerts to the specified channel if issues are detected.

### Example 3: Dedicated Monitoring Channel

You can use a separate channel for health alerts:

```ini
; Regular chat channel
ChatChannelId=111111111111111111

; Server notifications
NotificationChannelId=222222222222222222

; Health check alerts
HealthCheckChannelId=333333333333333333
```

## Getting Channel IDs

To find a Discord channel ID:

1. Enable Developer Mode in Discord (User Settings → Advanced → Developer Mode)
2. Right-click on the desired channel
3. Select "Copy ID"
4. Paste the ID into the configuration file

## What Gets Checked

The health check monitors:

1. **WebSocket Connection**
   - Verifies the custom WebSocket connection to Discord Gateway is active
   - Checks if the bot can send/receive messages

2. **Internet Connectivity**
   - Verifies the server has internet access
   - Checks if the server can reach Discord's API endpoints

3. **Server Metrics**
   - Current player count
   - Session uptime (time since bot started)
   - Timestamp of the check

## Troubleshooting

### Commands Not Working

**Problem:** `!status` command doesn't respond

**Solutions:**
- Verify `bEnableHealthCheckCommand=true` in config
- Make sure the bot is connected (check bot presence in Discord)
- Check that you're typing the command in a configured chat channel or any channel the bot can see

### Periodic Alerts Not Sending

**Problem:** Not receiving health check alerts

**Solutions:**
- Verify `bEnablePeriodicHealthCheck=true` in config
- Check that `HealthCheckChannelId` is set to a valid channel ID (not `YOUR_CHANNEL_ID_HERE`)
- Verify `HealthCheckInterval` is greater than 0
- Remember: Alerts only send when issues are detected (no spam when everything is working)

### False Alerts

**Problem:** Getting alerts when server seems fine

**Solutions:**
- Check your server's actual internet connection
- Verify Discord bot token is valid and has proper permissions
- Check server logs for network issues
- Adjust `HealthCheckInterval` if needed (too frequent checks may cause issues)

## Use Cases

### Remote Server Monitoring

If you host a server at a data center or friend's house and don't have direct console access:

1. Enable health check commands
2. Enable periodic health checks
3. Set up a dedicated monitoring channel
4. You'll get alerts on your phone/desktop if the server loses connection

### Development & Testing

During development, use manual `!status` commands to:
- Verify WebSocket connection is stable
- Check internet connectivity
- Monitor player count
- Track uptime

### Production Servers

For production servers:
- Enable both manual commands and periodic checks
- Set a reasonable interval (5-15 minutes)
- Use a dedicated admin/monitoring channel
- Monitor alerts for early warning of issues

## Security Considerations

- Health check commands work in any channel the bot can see messages in
- Commands don't reveal sensitive information (no IPs, tokens, etc.)
- Only connection status and basic metrics are shown
- Consider limiting bot access to specific channels if needed

## Related Documentation

- [Setup Guide](SETUP.md) - Initial bot setup
- [Troubleshooting](../Troubleshooting/WEBSOCKET_ISSUES.md) - WebSocket connection issues
- [Error Logging](../Troubleshooting/ERROR_LOGGING.md) - Detailed error logs

## Example Setup

Complete configuration example:

```ini
[DiscordBot]
BotToken=YOUR_BOT_TOKEN_HERE
bEnabled=true

; Two-way chat
bEnableTwoWayChat=true
ChatChannelId=111111111111111111

; Server notifications  
bEnableServerNotifications=true
NotificationChannelId=222222222222222222

; Health monitoring
bEnableHealthCheckCommand=true
bEnablePeriodicHealthCheck=true
HealthCheckInterval=600.0
HealthCheckChannelId=333333333333333333
```

This setup provides:
- Two-way chat in channel 111...
- Server start/stop notifications in channel 222...
- Health check alerts every 10 minutes in channel 333...
- Manual `!status` commands available in all channels

---

**Need Help?**

- Check [Troubleshooting Guide](../Troubleshooting/WEBSOCKET_ISSUES.md)
- Join our [Discord Server](https://discord.gg/QzcG9nX)
- Report issues on [GitHub](https://github.com/Yamahasxviper/SatisfactoryDiscordbot/issues)
