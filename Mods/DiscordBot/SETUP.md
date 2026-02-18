# Discord Bot Setup Guide

This guide will help you set up and use the Discord Bot mod for Satisfactory.

> ⚠️ **Server-Side Mod Notice**: This mod is configured as a **server-side-only mod**. Only the server administrator needs to install and configure this mod. Players joining the server **do not need to install this mod** to play on a modded server. The Discord bot functionality runs entirely server-side.

## Prerequisites

1. **Discord Bot Account**
   - Go to [Discord Developer Portal](https://discord.com/developers/applications)
   - Create a new application
   - Navigate to the "Bot" section
   - Click "Add Bot"
   - **IMPORTANT**: Enable the following Privileged Gateway Intents:
     - `PRESENCE INTENT`
     - `SERVER MEMBERS INTENT`
     - `MESSAGE CONTENT INTENT`
   - Copy your bot token (you'll need this later)

2. **Invite Bot to Your Server**
   - In the Developer Portal, go to "OAuth2" > "URL Generator"
   - Select scopes: `bot`
   - Select bot permissions based on your needs (at minimum: Send Messages, Read Messages)
   - Copy the generated URL and open it in your browser to invite the bot

## Installation

### For Server Administrators

1. **Enable the Mod on the Server**
   - Install the mod in your Satisfactory dedicated server's mod directory
   - Make sure the mod is enabled in your mod manager
   - **Note**: The mod is marked as `"Remote": true` in its uplugin file, making it server-side-only

2. **Configure the Bot Token**
   - Navigate to: `Mods/DiscordBot/Config/DiscordBot.ini`
   - Replace `YOUR_BOT_TOKEN_HERE` with your actual bot token:
     ```ini
     [DiscordBot]
     BotToken=your_actual_bot_token_here
     bEnabled=true
     ```

3. **Build the Project** (for developers)
   - Build the Satisfactory project using Unreal Engine
   - The Discord Bot module will be compiled automatically
   - If you encounter build errors, ensure all dependencies are properly installed

### For Players

**No installation required!** This is a server-side-only mod. Players can join the server with or without this mod installed. The Discord bot integration works entirely from the server side and does not require any client-side components.

## Usage Examples

### Automatic Mode

The bot will automatically connect when the game starts if:
- `bEnabled=true` in the config file
- A valid bot token is provided

### Blueprint Usage

1. **Get the Bot Subsystem**
   ```
   Get Game Instance -> Get Subsystem (Discord Bot Subsystem)
   ```

2. **Send a Message**
   ```
   Discord Bot Subsystem -> SendDiscordMessage
   - ChannelId: "your_channel_id"
   - Message: "Hello from Satisfactory!"
   ```

3. **Check Connection Status**
   ```
   Discord Bot Subsystem -> IsBotConnected -> Branch
   ```

### C++ Usage

```cpp
#include "DiscordBotSubsystem.h"

// Get the subsystem
UDiscordBotSubsystem* BotSubsystem = GetWorld()->GetGameInstance()->GetSubsystem<UDiscordBotSubsystem>();

// Initialize (if not auto-started)
BotSubsystem->InitializeAndConnect(TEXT("YOUR_BOT_TOKEN"));

// Send a message
BotSubsystem->SendDiscordMessage(TEXT("123456789012345678"), TEXT("Hello!"));

// Check status
bool bConnected = BotSubsystem->IsBotConnected();
```

## Finding Channel IDs

1. Enable Developer Mode in Discord:
   - User Settings > Advanced > Developer Mode (toggle on)

2. Right-click any channel and select "Copy ID"

3. Use this ID in your SendMessage calls

## Troubleshooting

### Bot Token Invalid
- **Error**: `Failed to get gateway URL` or `401 Unauthorized`
- **Solution**: Double-check your bot token in the config file
- Make sure there are no extra spaces or quotes around the token

### Missing Intents
- **Error**: Connection succeeds but bot doesn't receive events
- **Solution**: Verify all three intents are enabled in Discord Developer Portal:
  - PRESENCE INTENT
  - SERVER MEMBERS INTENT  
  - MESSAGE CONTENT INTENT

### Build Errors
- **Error**: Build errors or module dependencies missing
- **Solution**: Ensure the project dependencies are properly configured

### Bot Not Responding
- **Error**: Bot appears online but doesn't respond
- **Solution**: 
  - Check the game logs for errors
  - Verify the bot has permissions in your Discord server
  - Make sure the channel ID is correct

## Current Implementation

1. **Native WebSocket Implementation** (DiscordGatewayClientNative): 
   - Production-ready implementation using Unreal's native WebSocket module
   - Full Discord Gateway protocol support
   - Compatible with Satisfactory's custom CSS Unreal Engine 5.3.2

2. **Reference Implementation** (DiscordGatewayClient):
   - Educational reference showing Discord Gateway structure
   - Not for production use

3. **Event Handling**: Basic event handling is implemented. You can extend the `HandleGatewayEvent` method in `DiscordGatewayClientNative.cpp` to handle more Discord events.

4. **Reconnection**: Automatic reconnection on disconnect is not yet implemented.

## Advanced Configuration

Edit `Mods/DiscordBot/Config/DiscordBot.ini`:

```ini
[DiscordBot]
; Your Discord bot token
BotToken=YOUR_BOT_TOKEN_HERE

; Discord Gateway WebSocket URL
GatewayURL=wss://gateway.discord.gg

; Enable/Disable the bot
bEnabled=true

; Log verbosity (0=Error, 1=Warning, 2=Log, 3=Verbose)
LogLevel=2
```

## Next Steps

1. **Customize Events**: Modify `DiscordGatewayClientNative.cpp` to handle Discord events like messages, presence updates, etc.

2. **Add Commands**: Implement slash commands or message commands

3. **Integrate with Game**: Connect Discord events to Satisfactory gameplay events

4. **Extend Features**: Add support for embeds, attachments, reactions, etc.

## Support

For issues and questions:
- Check the [main README](README.md) for technical details
- Review the code in `Source/DiscordBot/` for implementation details
- Check Satisfactory modding documentation at [docs.ficsit.app](https://docs.ficsit.app)

## Security Notice

⚠️ **IMPORTANT**: Never commit your bot token to version control. Always keep it in the config file which should be ignored by git. The `.ini` files in mod Config directories are typically not committed to preserve local configurations.
