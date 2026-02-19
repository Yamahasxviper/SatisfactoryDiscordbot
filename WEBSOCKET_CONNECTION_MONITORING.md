# WebSocket Connection Status Monitoring

## Overview

The Discord bot now includes comprehensive logging to help server administrators monitor WebSocket connection status and internet connectivity through log files, especially useful when you don't have direct console access to the server.

## What's Logged

### Connection Establishment
When the bot connects to Discord, you'll see detailed logs showing:

1. **DNS Resolution**
   ```
   [INFO] LogCustomWebSocket: Attempting to resolve host: gateway.discord.gg
   [INFO] LogCustomWebSocket: Performing DNS lookup for: gateway.discord.gg
   [INFO] LogCustomWebSocket: DNS resolution successful: 162.159.xxx.xxx:443
   ```

2. **TCP Connection**
   ```
   [INFO] LogCustomWebSocket: Creating socket...
   [INFO] LogCustomWebSocket: Connecting to gateway.discord.gg:443...
   [INFO] LogCustomWebSocket: ========================================
   [INFO] LogCustomWebSocket: TCP connection established successfully
   [INFO] LogCustomWebSocket: Connected to: gateway.discord.gg:443
   [INFO] LogCustomWebSocket: Internet connection verified
   [INFO] LogCustomWebSocket: ========================================
   ```

3. **WebSocket Handshake**
   ```
   [INFO] LogCustomWebSocket: WebSocket handshake sent, waiting for response...
   [INFO] LogCustomWebSocket: Handshake response received
   [INFO] LogCustomWebSocket: ========================================
   [INFO] LogCustomWebSocket: CustomWebSocket: Successfully connected to gateway.discord.gg:443
   [INFO] LogCustomWebSocket: CustomWebSocket: WebSocket handshake complete!
   [INFO] LogCustomWebSocket: ========================================
   ```

4. **Discord Gateway Ready**
   ```
   [INFO] LogDiscordGatewayCustom: ****************************************
   [INFO] LogDiscordGatewayCustom: *** DISCORD BOT FULLY CONNECTED AND READY! ***
   [INFO] LogDiscordGatewayCustom: *** CustomWebSocket successfully connected to Discord Gateway ***
   [INFO] LogDiscordGatewayCustom: *** Session ID: abc123xyz ***
   [INFO] LogDiscordGatewayCustom: *** Internet connection: ACTIVE ***
   [INFO] LogDiscordGatewayCustom: ****************************************
   ```

### Periodic Status Reports

Every 10 heartbeats (approximately every 7 minutes with default Discord heartbeat interval), the bot logs a comprehensive status report:

```
[INFO] LogDiscordGatewayCustom: ========================================
[INFO] LogDiscordGatewayCustom: Connection Status Report
[INFO] LogDiscordGatewayCustom: Status: CONNECTED
[INFO] LogDiscordGatewayCustom: WebSocket: ACTIVE
[INFO] LogDiscordGatewayCustom: Internet Connection: ACTIVE
[INFO] LogDiscordGatewayCustom: Uptime: 42.5 minutes
[INFO] LogDiscordGatewayCustom: Heartbeats sent: 60
[INFO] LogDiscordGatewayCustom: Session ID: abc123xyz
[INFO] LogDiscordGatewayCustom: ========================================
```

### Connection Issues

When there are connection problems, you'll see detailed error messages:

1. **DNS Resolution Failure** (No Internet or DNS Issues)
   ```
   [ERROR] LogCustomWebSocket: Failed to resolve host: gateway.discord.gg (DNS lookup failed - check internet connection)
   ```

2. **Connection Refused** (No Internet or Firewall Issues)
   ```
   [ERROR] LogCustomWebSocket: Failed to connect to gateway.discord.gg:443 (Connection refused - check internet connection and firewall)
   ```

3. **Heartbeat Issues** (Connection Quality Problems)
   ```
   [WARNING] LogDiscordGatewayCustom: Heartbeat ACK not received, connection may be zombie
   ```

4. **Disconnection**
   ```
   [INFO] LogCustomWebSocket: Disconnecting WebSocket (StatusCode: 1000, Reason: Normal closure)
   [INFO] LogCustomWebSocket: WebSocket disconnected
   ```

## Log File Location

By default, logs are written to:
```
<ProjectSavedDir>/Logs/DiscordBot/DiscordBot_<timestamp>.log
```

You can configure a custom log directory in your `DefaultGame.ini`:
```ini
[DiscordBot]
ErrorLogDirectory=/path/to/your/logs
```

## How to Use for Monitoring

### Check if Bot is Connected
Look for the most recent "Connection Status Report" or "DISCORD BOT FULLY CONNECTED AND READY" message:
- If present and recent (within last ~10 minutes), the bot is connected
- The uptime field shows how long it's been connected

### Check Internet Connectivity
Look for these indicators:
1. "Internet connection verified" in TCP connection logs
2. "Internet connection: ACTIVE" in status reports
3. Successful heartbeat acknowledgments

### Check for Connection Problems
Search the log for:
- "ERROR" messages indicating connection failures
- "WARNING" messages about missed heartbeat ACKs
- DNS resolution failures
- Connection refused errors

### Monitor Connection Stability
- Check the uptime in status reports (longer = more stable)
- Look at heartbeat count (should increment regularly)
- Verify no reconnection attempts in logs

## Troubleshooting

### Bot Won't Connect
1. Check for DNS resolution errors → DNS or internet connectivity issue
2. Check for connection refused → Firewall or internet connectivity issue
3. Check for handshake failures → Discord API or SSL certificate issue

### Bot Disconnects Frequently
1. Look for heartbeat ACK warnings → Network quality issue
2. Check status code in disconnect messages:
   - 1000 = Normal closure
   - 4xxx = Discord-specific error codes (check Discord docs)

### No Status Reports
If you don't see periodic status reports:
1. Bot may not have completed READY handshake
2. Bot may have crashed or stopped
3. Check for ERROR or WARNING messages before the status reports stopped

## Log File Management

- Log files automatically rotate when they reach 10MB
- Old log file is renamed with `.old` extension
- Only one backup file is kept (older backups are deleted)
- All logs are timestamped for easy tracking

## What These Logs Tell You

✅ **DNS Resolution Success** = Internet connection is working, DNS is functional
✅ **TCP Connection Success** = Internet connection is active, can reach Discord servers
✅ **WebSocket Handshake Success** = WebSocket protocol is working correctly
✅ **Discord Ready Message** = Bot is fully authenticated and operational
✅ **Regular Status Reports** = Connection is stable and healthy
✅ **Heartbeat Count Increasing** = Two-way communication is working

❌ **DNS Failures** = Internet down or DNS server issues
❌ **Connection Refused** = Internet down, firewall blocking, or Discord unreachable
❌ **Missing Heartbeat ACKs** = Network quality issues or Discord problems
❌ **Frequent Reconnections** = Unstable network or Discord issues
