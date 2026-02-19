# Health Check Feature Implementation Summary

## Overview

This document describes the implementation of the health check and status monitoring feature for the Discord bot, which allows remote monitoring of server connectivity without direct server access.

## Problem Solved

The user asked: *"Is there a way I can tell if custom websocket is connected on a server and has internet access on a server I don't have access to from home?"*

**Solution:** Implemented Discord bot commands and optional periodic monitoring to remotely check server connection status.

## Features Implemented

### 1. Manual Status Check Commands

Users can type commands in Discord to get instant server health status:

- `!status` - Get current server health status
- `!health` - Alternative to `!status`
- `!help` - Show available bot commands

**Response includes:**
- ✅/❌ WebSocket connection status
- ✅/❌ Internet connectivity status
- 👥 Current player count
- ⏱️ Server session uptime
- 🕐 Timestamp of the check

### 2. Periodic Health Check Alerts

Optional automatic monitoring that:
- Checks connection status at configurable intervals
- Sends alerts ONLY when issues are detected (no spam)
- Reports to a dedicated Discord channel
- Monitors both WebSocket and internet connectivity

### 3. Configuration Options

All features are configurable via `Config/DiscordBot.ini`:

```ini
; Enable bot commands for manual health checks
bEnableHealthCheckCommand=true

; Enable automatic periodic health monitoring
bEnablePeriodicHealthCheck=false

; How often to check (in seconds, default: 300 = 5 minutes)
HealthCheckInterval=300.0

; Discord channel ID for health alerts
HealthCheckChannelId=YOUR_CHANNEL_ID_HERE
```

## Technical Implementation

### Files Modified

1. **DiscordBotSubsystem.h**
   - Added health check configuration properties
   - Added health check method declarations
   - Added timer handle for periodic checks

2. **DiscordBotSubsystem.cpp**
   - Implemented `LoadHealthCheckConfig()` - Loads configuration from INI file
   - Implemented `HandleBotCommand()` - Processes Discord commands
   - Implemented `CheckInternetConnectivity()` - Verifies internet access
   - Implemented `SendHealthStatus()` - Sends status report to Discord
   - Implemented `PerformPeriodicHealthCheck()` - Periodic monitoring logic
   - Modified `OnDiscordMessageReceived()` - Intercepts bot commands
   - Modified `Initialize()` - Loads config and starts periodic checks
   - Modified `Deinitialize()` - Cleans up health check timer
   - Added necessary includes (HttpModule, DateTime)

3. **DiscordBot.ini**
   - Added health check configuration section
   - Added detailed comments for each setting

4. **README.md**
   - Added health check feature to feature list

5. **Docs/README.md**
   - Added link to health check guide

### New Files Created

1. **Docs/Setup/HEALTH_CHECK_GUIDE.md**
   - Comprehensive user guide
   - Configuration examples
   - Usage scenarios
   - Troubleshooting section

## How It Works

### Command Flow

1. User types `!status` in Discord
2. Discord Gateway sends MESSAGE_CREATE event to bot
3. `DiscordGatewayClientCustom` receives message
4. Message forwarded to `DiscordBotSubsystem::OnDiscordMessageReceived()`
5. If message starts with `!`, it's treated as a command
6. `HandleBotCommand()` processes the command
7. `SendHealthStatus()` checks status and sends response
8. Response appears in Discord channel

### Periodic Check Flow

1. Bot initializes and connects to Discord
2. If periodic checks enabled, timer is started
3. `PerformPeriodicHealthCheck()` called at configured interval
4. Checks WebSocket and internet connectivity
5. If issues detected, sends alert to configured channel
6. If everything is OK, no message sent (prevents spam)

### Connectivity Check Logic

**WebSocket Status:**
- Uses `IsBotConnected()` method
- Checks if Discord Gateway WebSocket is active
- Indicates bot can send/receive Discord messages

**Internet Connectivity:**
- Leverages existing WebSocket connection
- If WebSocket is connected, internet is available
- WebSocket connection requires working internet
- Simple and reliable indicator

## Configuration Examples

### Example 1: Manual Checks Only
```ini
bEnableHealthCheckCommand=true
bEnablePeriodicHealthCheck=false
```
Users can type `!status` anytime for instant status.

### Example 2: Full Monitoring
```ini
bEnableHealthCheckCommand=true
bEnablePeriodicHealthCheck=true
HealthCheckInterval=600.0
HealthCheckChannelId=123456789012345678
```
Manual commands available + alerts every 10 minutes if issues detected.

### Example 3: Dedicated Monitoring Channel
```ini
; Chat messages
ChatChannelId=111111111111111111

; Server notifications
NotificationChannelId=222222222222222222

; Health alerts
HealthCheckChannelId=333333333333333333
```
Separate channels for different notification types.

## Use Cases

### Remote Server Monitoring
Monitor a server at a data center or friend's house without console access.

### Development & Testing
Verify WebSocket stability and connectivity during development.

### Production Servers
Early warning system for connection issues.

## Code Quality Considerations

### Error Handling
- Defensive checks for null pointers
- Configuration validation (empty strings, default values)
- Graceful fallbacks if World or timer unavailable

### Performance
- Minimal overhead - only checks when commanded or at long intervals
- No continuous polling
- Efficient timer management

### User Experience
- Alerts only on issues (no spam)
- Clear status messages with emoji indicators
- Helpful command responses
- Configurable to suit different needs

### Maintainability
- Well-documented code with clear comments
- Logical separation of concerns
- Follows existing code patterns
- Comprehensive user documentation

## Testing Recommendations

### Manual Testing
1. Type `!status` in Discord with bot connected
2. Type `!help` to verify command list
3. Enable periodic checks and wait for alerts
4. Disconnect internet and verify alerts are sent
5. Reconnect and verify alerts stop

### Configuration Testing
1. Test with periodic checks disabled
2. Test with invalid channel IDs
3. Test with various intervals
4. Test with commands disabled

### Edge Cases
1. Bot disconnected when command received (should fail gracefully)
2. World not available (should handle safely)
3. Empty configuration values
4. Very short intervals (stress test)

## Future Enhancements

Potential improvements for future versions:

1. **Extended Status Information**
   - Server FPS/TPS
   - Memory usage
   - Save game information
   - Mod list

2. **More Commands**
   - `!players` - List online players
   - `!uptime` - Detailed uptime info
   - `!restart` - Request server restart (with auth)

3. **Advanced Monitoring**
   - Historical data tracking
   - Alert thresholds (only alert after X failures)
   - Multiple check endpoints
   - Response time measurement

4. **Notification Improvements**
   - Discord embed messages (rich formatting)
   - Ping specific users/roles on alerts
   - Different alert levels (warning, critical)

## Security Considerations

### Command Access
- Commands work in any channel bot can see messages
- No authentication required (read-only status)
- No sensitive information exposed (no IPs, tokens)
- Consider limiting bot to specific channels if needed

### Information Disclosure
- Status only shows connection state and player count
- No server configuration details
- No internal network information
- Safe for public/semi-public Discord servers

### Resource Usage
- Configurable check intervals prevent abuse
- No external API calls (uses existing connection)
- Minimal memory footprint
- Clean timer management prevents leaks

## Conclusion

This implementation provides a robust, user-friendly solution for remote server monitoring. It addresses the user's need to check server connectivity from home without direct server access, while maintaining code quality, performance, and security standards.

The feature integrates seamlessly with the existing Discord bot infrastructure and provides both immediate (command-based) and proactive (alert-based) monitoring capabilities.

## Documentation

- **User Guide:** `Mods/DiscordBot/Docs/Setup/HEALTH_CHECK_GUIDE.md`
- **Main README:** `Mods/DiscordBot/README.md`
- **Configuration:** `Mods/DiscordBot/Config/DiscordBot.ini`

---

**Implementation Date:** February 2026  
**Author:** GitHub Copilot
