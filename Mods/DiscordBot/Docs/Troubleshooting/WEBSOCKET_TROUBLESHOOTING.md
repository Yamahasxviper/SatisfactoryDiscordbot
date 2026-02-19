# WebSocket Troubleshooting Guide

## Overview

This document provides solutions for common WebSocket connection issues when using the Discord Bot mod with the CustomWebSocket implementation.

## Verifying Successful Connection

### How to Check if the Bot is Connected

If you have access to the server log files, you can easily verify if the Discord bot has successfully connected to the custom WebSocket by looking for these specific log messages:

**1. Connection Attempt:**
```
========================================
Discord Bot: Attempting to connect to Discord Gateway
Discord Bot: WebSocket URL: wss://gateway.discord.gg/?v=10&encoding=json
Discord Bot: Using CustomWebSocket implementation
========================================
```

**2. CustomWebSocket Connection Established:**
```
========================================
CustomWebSocket: Successfully connected to gateway.discord.gg:443
CustomWebSocket: WebSocket handshake complete!
========================================
```

**3. Discord Gateway Connection Established:**
```
========================================
Discord Bot: CustomWebSocket connection established!
Discord Bot: Waiting for Discord Gateway HELLO message...
========================================
```

**4. Bot Fully Ready (Final Confirmation):**
```
****************************************
*** DISCORD BOT FULLY CONNECTED AND READY! ***
*** CustomWebSocket successfully connected to Discord Gateway ***
*** Session ID: <your_session_id> ***
****************************************
```

**✅ If you see all four log sections, especially the final "DISCORD BOT FULLY CONNECTED AND READY" message, your bot is successfully connected and operational!**

### Log File Locations

- **Windows Server:** `%LOCALAPPDATA%\FactoryGame\Saved\Logs\FactoryGame.log`
- **Linux Server:** `~/.config/Epic/FactoryGame/Saved/Logs/FactoryGame.log`

---

## Connection Issues

### Issue 1: Connection Failed - "Invalid WebSocket URL"

**Symptoms:**
- Bot fails to connect immediately
- Error log: "Invalid WebSocket URL"
- No network activity detected

**Root Causes:**
1. Gateway URL is malformed
2. URL parsing failed (invalid protocol)
3. Missing scheme (ws:// or wss://)

**Solutions:**

1. **Verify Gateway URL Format**
   - Ensure URL starts with `wss://` (secure) or `ws://` (insecure)
   - Correct format: `wss://gateway.discord.gg/?v=10&encoding=json`
   - Check `Config/DiscordBot.ini` for correct GatewayURL setting

   ```ini
   [DiscordBot]
   GatewayURL=wss://gateway.discord.gg/?v=10&encoding=json
   ```

2. **Check for Typos**
   - Verify hostname spelling: `gateway.discord.gg` (not `discordgateway.gg`, etc.)
   - Ensure no extra spaces before/after URL
   - Validate query parameters are correctly formatted

3. **Verify Protocol Scheme**
   - Use `wss://` for secure WebSocket (required for Discord)
   - `ws://` is only for local development without TLS
   - Never use `http://` or `https://` for WebSocket

---

### Issue 2: TCP Connection Failed

**Symptoms:**
- Error log: "TCP connection failed"
- Bot cannot reach the Discord server
- Network timeout occurs

**Root Causes:**
1. No internet connectivity
2. Firewall blocking outbound connections
3. DNS resolution failure
4. ISP/network restrictions

**Solutions:**

1. **Check Internet Connectivity**
   ```bash
   # Windows
   ping gateway.discord.gg
   
   # Linux/Mac
   ping -c 4 gateway.discord.gg
   ```
   - Should see responses from Discord's servers
   - If no response, check internet connection

2. **Verify Firewall Settings**
   - Allow outbound connections on port 443 (HTTPS/WebSocket)
   - Whitelist `gateway.discord.gg` domain if using domain-based firewall
   - Check Windows Firewall or equivalent

3. **DNS Resolution**
   - Test DNS: `nslookup gateway.discord.gg`
   - Should return Discord's IP address
   - If fails, try alternate DNS (8.8.8.8, 1.1.1.1)

4. **Check Discord Endpoint Availability**
   - Discord Gateway may be temporarily down
   - Check Discord status page: https://discordstatus.com
   - Wait a few minutes and retry

5. **For Dedicated Servers**
   - Ensure server has outbound internet access
   - Check server firewall rules
   - Verify ISP allows connections to Discord

---

### Issue 3: TLS/SSL Handshake Failed

**Symptoms:**
- Error log: "TLS handshake failed"
- Connection drops immediately after TCP connection
- Certificate validation errors

**Root Causes:**
1. SSL/TLS library issues (OpenSSL)
2. Certificate validation problems
3. System time is incorrect (certificate expiration check fails)
4. Proxy/MITM interfering with handshake

**Solutions:**

1. **Verify System Time**
   - Incorrect time causes certificate validation to fail
   
   ```bash
   # Windows
   Get-Date
   
   # Linux/Mac
   date
   ```
   - Should be within a few seconds of accurate time
   - Sync with NTP server if wrong

2. **Check OpenSSL Availability**
   - Unreal Engine requires OpenSSL library
   - Usually included with engine installation
   - For Linux, verify: `ldconfig -p | grep ssl`

3. **For Corporate Proxy**
   - If using corporate network proxy:
     - Configure proxy in OS network settings
     - Ensure proxy allows WebSocket connections
     - Some corporate proxies block WebSocket entirely

---

### Issue 4: WebSocket Handshake Failed

**Symptoms:**
- Error log: "WebSocket handshake failed"
- TCP and TLS connections succeed, but WebSocket protocol fails
- "Invalid handshake response" errors

**Root Causes:**
1. Server doesn't respond with HTTP 101
2. Required headers missing from response
3. Sec-WebSocket-Accept key mismatch
4. Server rejected the connection

**Solutions:**

1. **Check WebSocket Headers**
   - Verify request includes required headers:
     - `Upgrade: websocket`
     - `Connection: Upgrade`
     - `Sec-WebSocket-Version: 13`
     - `Sec-WebSocket-Key: <base64-encoded-key>`

2. **Verify Response Status**
   - Server should respond with: `HTTP/1.1 101 Switching Protocols`
   - If returning other status (400, 403, 500), Discord rejected request
   - Check bot token validity

3. **Validate URL Query Parameters**
   - Discord expects: `?v=10&encoding=json`
   - Missing or incorrect parameters cause rejection
   - Some proxy/firewall may strip query parameters

---

### Issue 5: Frequent Disconnections

**Symptoms:**
- Bot connects but drops after random intervals
- Connection resets without warning
- "Received close frame" messages in logs

**Root Causes:**
1. Heartbeat not being sent
2. Heartbeat interval too short/long
3. Network instability
4. Discord timeout (no heartbeat response)
5. Rate limiting from Discord

**Solutions:**

1. **Check Heartbeat Interval**
   - Discord sends `heartbeat_interval` in HELLO message (typically ~41 seconds)
   - Heartbeat must be sent within this interval
   - Look for "heartbeat_interval" in logs

2. **Verify Heartbeat Is Being Sent**
   - Enable verbose logging to see heartbeat messages
   - Check logs for periodic heartbeat opcodes
   - If no heartbeat sent, check heartbeat timer initialization

3. **Check Network Stability**
   - Run network diagnostics: `ping -t gateway.discord.gg`
   - Monitor packet loss (should be <1%)
   - Check for packet fragmentation issues

4. **Detect Rate Limiting**
   - Discord may disconnect if rate limits exceeded
   - Check for "rate limited" messages
   - Implement backoff in message sending
   - Max ~10 messages per 10 seconds

---

### Issue 6: Bot Not Receiving Events

**Symptoms:**
- Bot connects successfully
- No MESSAGE_CREATE events received
- Guild events not firing
- Appears online but doesn't respond

**Root Causes:**
1. Intents not enabled in Discord Developer Portal
2. Intents not correctly configured in bot
3. Permission issues on Discord server
4. Event handlers not bound properly

**Solutions:**

1. **Enable Intents in Discord Developer Portal**
   - Navigate to: Discord Developer Portal → Your App → Bot
   - Scroll to "Privileged Gateway Intents"
   - Enable required intents:
     - ☑️ **PRESENCE INTENT** (bit 8, value 256)
     - ☑️ **SERVER MEMBERS INTENT** (bit 1, value 2)
     - ☑️ **MESSAGE CONTENT INTENT** (bit 15, value 32768)
   - Save changes and wait for sync (may take a few seconds)

2. **Verify Bot Intents Value**
   - Code should use: `#define DISCORD_INTENTS_COMBINED 33027`
   - Breakdown:
     - Guilds (1 << 0) = 1
     - Server Members (1 << 1) = 2 [PRIVILEGED]
     - Presences (1 << 8) = 256 [PRIVILEGED]
     - Message Content (1 << 15) = 32768 [PRIVILEGED]
     - **Total: 33027**

3. **Check Event Handlers**
   - Ensure event callbacks are registered before connecting
   - Verify delegate binding syntax
   - Check for null handler crashes

4. **Check Bot Permissions**
   - Bot must be in the Discord server
   - Server role must have appropriate permissions
   - Message reading permission required for MESSAGE_CREATE

5. **Verify READY Event**
   - After IDENTIFY, should receive READY event
   - If no READY, intents may be rejected
   - Check logs for "READY event received"

---

## Identification Sequence Errors

### Issue 7: Bot Token Invalid or Expired

**Symptoms:**
- Connection fails after HELLO, during IDENTIFY
- Discord closes connection immediately
- "Invalid token" or "Unauthorized" errors

**Root Causes:**
1. Bot token is incorrect
2. Token has been regenerated (old token now invalid)
3. Token copied incorrectly (extra spaces, wrong format)

**Solutions:**

1. **Verify Token Format**
   - Token format should be similar to: `MzExxx.Gxxxx.Xxxxx`
   - Should contain only alphanumeric characters, dots, hyphens, underscores
   - No spaces before or after

2. **Check Token in Config**
   ```ini
   [DiscordBot]
   BotToken=YOUR_ACTUAL_TOKEN_HERE
   ```
   - No leading/trailing spaces
   - Exact token from Developer Portal
   - Verify copy/paste didn't include whitespace

3. **Regenerate Token if Needed**
   - Discord Developer Portal → Bot → Token
   - Click "Regenerate"
   - Old token immediately invalidated
   - Use new token in config
   - Restart bot with new token

4. **Never Commit Token to Git**
   - Add `Config/DiscordBot.ini` to `.gitignore`
   - Use environment variables for CI/CD
   - If token leaked, regenerate immediately

---

### Issue 8: Invalid Intents Error

**Symptoms:**
- IDENTIFY succeeds but READY never arrives
- Discord disconnects with "Invalid intents" message
- Bot comes online but receives no events

**Root Causes:**
1. Intents value doesn't match Discord configuration
2. Privileged intents not enabled in Developer Portal
3. Wrong intents calculation

**Solutions:**

1. **Cross-Check Intents Value**
   - Code value must match Portal settings
   - If code has intents, Portal must have them enabled
   - Misalignment causes immediate rejection

2. **Recalculate Intent Bits**
   ```
   GUILDS = 1 << 0 = 1
   GUILD_MEMBERS = 1 << 1 = 2
   GUILD_PRESENCES = 1 << 8 = 256
   GUILD_MESSAGE_CONTENT = 1 << 15 = 32768
   
   Combined = 1 | 2 | 256 | 32768 = 33027
   ```

3. **Re-enable Intents in Portal**
   - Go to Discord Developer Portal
   - Bot section → Privileged Gateway Intents
   - Disable all → Save → Re-enable all → Save
   - Forces Discord to resync intents
   - Wait 30 seconds before reconnecting bot

---

## Message Sending Issues

### Issue 9: Failed to Send Message

**Symptoms:**
- `SendMessage()` called but message doesn't appear
- Error log: "Failed to send message"
- HTTP request fails or returns error code

**Root Causes:**
1. Bot not connected to Discord
2. Invalid channel ID
3. Missing permissions on Discord server
4. HTTP API request failed
5. Rate limiting

**Solutions:**

1. **Check Connection Status**
   - Verify bot is connected before sending messages
   - Look for "Connected" or "READY" status in logs

2. **Verify Channel ID**
   - Right-click channel in Discord → Copy Channel ID
   - Must be snowflake (18-digit number as string)
   - Wrong format: "general" or "my_channel" (need numeric ID)
   - Format: `"123456789012345678"`

3. **Check Bot Permissions**
   - Server → Channel → Permissions
   - Bot role must have:
     - ☑️ Send Messages
     - ☑️ Send Messages in Threads
     - ☑️ Embed Links (if using embeds)
     - ☑️ View Channel

4. **Verify HTTP Endpoint**
   - Messages use HTTP POST to Discord REST API
   - Endpoint: `https://discord.com/api/v10/channels/{channel_id}/messages`
   - Requires valid bot token in Authorization header

5. **Check Rate Limiting**
   - Discord limits: ~10 messages per 10 seconds per channel
   - Wait between messages if sending rapidly
   - Implement exponential backoff for retries

---

## Platform-Specific Issues

### Issue 10: Linux-Specific Connection Issues

**Symptoms:**
- Works on Windows but fails on Linux
- Socket errors on Linux servers

**Root Causes:**
1. OpenSSL not installed on Linux
2. Socket permissions issues
3. System resource limits (file descriptors)

**Solutions:**

1. **Verify OpenSSL Installation**
   ```bash
   ldconfig -p | grep ssl
   ```
   - Should show libssl.so entries
   - Install if missing: `sudo apt-get install libssl-dev`

2. **Check File Descriptor Limit**
   ```bash
   ulimit -n
   ```
   - Should be at least 1024
   - Increase if needed: `ulimit -n 4096`

3. **Check Network Interface**
   ```bash
   ip addr show
   ```
   - Verify network interface is up
   - Check for IPv6 issues (force IPv4 if needed)

---

### Issue 11: Dedicated Server Connection Issues

**Symptoms:**
- Bot works in offline play mode
- Fails on dedicated server instance
- Network access differences

**Root Causes:**
1. Dedicated server has no internet access
2. Firewall rules block outbound connections
3. DNS not available on server

**Solutions:**

1. **Verify Outbound Connectivity**
   - Test from server: `ping gateway.discord.gg`
   - Test: `curl -I https://gateway.discord.gg`

2. **Check Firewall Rules**
   - Server firewall must allow:
     - Outbound: Port 443 to gateway.discord.gg
   - Whitelist Discord domain if using domain firewall

3. **Verify DNS Resolution**
   - Test: `nslookup gateway.discord.gg`
   - If fails, configure DNS settings
   - Add nameserver: `nameserver 8.8.8.8`

---

### Issue 12: CustomWebSocket Module Not Found

**Symptoms:**
- Error log: "CRITICAL ERROR: CustomWebSocket plugin not found!"
- Bot fails to initialize completely
- No WebSocket connection attempts made
- Error appears during module startup

**Root Causes:**
1. CustomWebSocket mod not installed via mod manager (most common)
2. CustomWebSocket mod is installed but not enabled
3. Mods are installed in different locations
4. Module loading order issue (rare)

**Solutions:**

1. **Install CustomWebSocket Mod (Most Common Solution)**
   
   ⚠️ **IMPORTANT**: DiscordBot and CustomWebSocket are **TWO SEPARATE MODS**.
   
   Both must be installed via the Satisfactory Mod Manager (SMM):
   
   - Open Satisfactory Mod Manager (SMM)
   - Search for "CustomWebSocket"
   - Click "Install" or "Enable"
   - Search for "DiscordBot" 
   - Click "Install" or "Enable"
   - Restart your game/server
   
   **Note**: Installing DiscordBot alone is NOT enough. You MUST also install CustomWebSocket.

2. **Verify Both Mods Are Enabled**
   
   In SMM, check that both mods show as enabled:
   - ✅ CustomWebSocket - Enabled
   - ✅ DiscordBot - Enabled
   
   If one is disabled, enable it and restart.

3. **Check Installation Location**
   
   Both mods should be installed in the same mods directory:
   - **Windows**: `%LOCALAPPDATA%/FactoryGame/Saved/Paks/Mods/`
   - **Linux**: `~/.config/Epic/FactoryGame/Saved/Paks/Mods/`
   
   If you manually installed the mods, ensure both are in the same location.

4. **Verify Mod Versions**
   
   - Make sure you have compatible versions of both mods
   - Update both mods to the latest version via SMM
   - Incompatible versions may cause loading failures

5. **Check Game Logs**
   
   Look for detailed error messages in the game logs:
   ```
   LogDiscordBot: CRITICAL ERROR: CustomWebSocket plugin not found!
   LogDiscordBot: SOLUTION:
   LogDiscordBot:   1. Install the CustomWebSocket mod from the Satisfactory Mod Manager (SMM)
   LogDiscordBot:   2. Make sure both DiscordBot and CustomWebSocket are enabled
   LogDiscordBot:   3. Restart your game/server
   ```
   
   The logs will show the plugin location if it's found but not loaded properly.

6. **Reinstall Both Mods**
   
   If problems persist:
   - Uninstall both DiscordBot and CustomWebSocket via SMM
   - Restart the game/server
   - Reinstall both mods via SMM
   - Restart again

**Expected Log Output (Success):**
```
LogDiscordBot: DiscordBot module starting up
LogDiscordBot: CustomWebSocket module already loaded from: C:/Path/To/Mods/CustomWebSocket
LogDiscordBot: Error logging initialized at: Saved/Logs/DiscordBot
```

**Expected Log Output (Auto-Load Success):**
```
LogDiscordBot: DiscordBot module starting up
LogDiscordBot: Warning: CustomWebSocket module not loaded, attempting to load...
LogDiscordBot: CustomWebSocket module loaded successfully from: C:/Path/To/Mods/CustomWebSocket
LogDiscordBot: Error logging initialized at: Saved/Logs/DiscordBot
```

**Error Log Output (Not Installed):**
```
LogDiscordBot: DiscordBot module starting up
LogDiscordBot: Warning: CustomWebSocket module not loaded, attempting to load...
LogDiscordBot: Error: ========================================================================================
LogDiscordBot: Error: CRITICAL ERROR: CustomWebSocket plugin not found!
LogDiscordBot: Error: ========================================================================================
LogDiscordBot: Error: The DiscordBot mod requires the CustomWebSocket mod to be installed.
LogDiscordBot: Error:
LogDiscordBot: Error: SOLUTION:
LogDiscordBot: Error:   1. Install the CustomWebSocket mod from the Satisfactory Mod Manager (SMM)
LogDiscordBot: Error:   2. Make sure both DiscordBot and CustomWebSocket are enabled
LogDiscordBot: Error:   3. Restart your game/server
LogDiscordBot: Error:
LogDiscordBot: Error: NOTE: DiscordBot and CustomWebSocket are separate mods and must BOTH be installed.
LogDiscordBot: Error: They should be installed in the same mods directory (typically via mod manager).
LogDiscordBot: Error: ========================================================================================
```

**Error Log Output (Not Enabled):**
```
LogDiscordBot: DiscordBot module starting up
LogDiscordBot: Warning: CustomWebSocket module not loaded, attempting to load...
LogDiscordBot: Error: ========================================================================================
LogDiscordBot: Error: ERROR: CustomWebSocket plugin found but not enabled!
LogDiscordBot: Error: ========================================================================================
LogDiscordBot: Error: Plugin location: C:/Path/To/Mods/CustomWebSocket
LogDiscordBot: Error:
LogDiscordBot: Error: SOLUTION:
LogDiscordBot: Error:   1. Enable the CustomWebSocket mod in your mod manager
LogDiscordBot: Error:   2. Restart your game/server
LogDiscordBot: Error: ========================================================================================
```

---

## Logging and Debugging

### Enable Detailed Logging

**Check Log Files:**
```
Saved/Logs/DiscordBot/
DiscordBot_YYYYMMDD_HHMMSS.log
```

**Key Lines to Look For:**
```
[LOG] Connecting to gateway.discord.gg:443
[LOG] TCP connection established
[LOG] WebSocket handshake complete!
[LOG] Received HELLO with heartbeat_interval
[LOG] IDENTIFY sent
[LOG] Received READY event
[LOG] Sending heartbeat
```

**Error Patterns:**
```
[ERROR] Failed to resolve host
[ERROR] TLS handshake failed
[ERROR] WebSocket handshake failed
[ERROR] Invalid intents
[ERROR] Unauthorized (invalid token)
```

---

## Quick Diagnostic Checklist

Use this checklist to systematically diagnose issues:

- [ ] **Plugin Installation**
  - CustomWebSocket plugin folder exists in Mods/
  - CustomWebSocket.uplugin file present
  - All source files in place

- [ ] **Module Loading**
  - Check logs for "CustomWebSocket module already loaded" or "loaded successfully"
  - No "Failed to load CustomWebSocket module!" errors

- [ ] **Internet Connectivity**
  - `ping gateway.discord.gg` returns responses
  - No packet loss or timeouts

- [ ] **DNS Resolution**
  - `nslookup gateway.discord.gg` returns IP
  - Matches Discord's actual servers

- [ ] **Firewall/Port Access**
  - Port 443 open for outbound
  - No proxy blocking WebSocket
  - No ISP restrictions

- [ ] **Bot Token**
  - Token valid in Discord Developer Portal
  - Correct format
  - No leading/trailing spaces in config

- [ ] **Intents Configuration**
  - All 3 privileged intents enabled in Portal
  - Intent value matches code (33027)

- [ ] **Bot Permissions**
  - Bot in Discord server
  - Role has required permissions
  - Can view channels and send messages

- [ ] **Network Stability**
  - Consistent connectivity
  - Low packet loss (<1%)
  - Stable RTT to Discord servers

---

## Common Error Messages Reference

| Error Message | Cause | Solution |
|---------------|-------|----------|
| "Invalid WebSocket URL" | URL malformed | Use `wss://gateway.discord.gg/?v=10&encoding=json` |
| "TCP connection failed" | No internet/firewall | Check connectivity and firewall |
| "TLS handshake failed" | SSL/TLS issue | Verify system time, OpenSSL installed |
| "WebSocket handshake failed" | Server rejected | Check token, intents |
| "Invalid handshake response" | Wrong HTTP status | Check token and intents again |
| "Failed to resolve host" | DNS failure | Check DNS, internet, firewall |
| "Not connected" | Attempted message before connect | Wait for READY event |
| "Failed to load CustomWebSocket module!" | Plugin missing or not installed | Install CustomWebSocket plugin in Mods folder |

---

## Additional Resources

- [Discord Developer Portal](https://discord.com/developers/applications)
- [Discord Gateway Documentation](https://discord.com/developers/docs/topics/gateway)
- [WebSocket RFC 6455](https://datatracker.ietf.org/doc/html/rfc6455)
- [CustomWebSocket Plugin Documentation](CUSTOM_WEBSOCKET.md)
- [Privileged Intents Guide](PRIVILEGED_INTENTS_GUIDE.md)
- [WebSocket Compatibility](WEBSOCKET_COMPATIBILITY.md)
- [Error Logging Guide](ERROR_LOGGING.md)

---

**Last Updated:** 2024-03-15  
**Status:** Production Ready
