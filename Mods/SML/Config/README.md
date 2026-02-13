# SML Configuration Settings

This document describes the configuration settings available for the Satisfactory Mod Loader (SML).

## Configuration File Location

The SML configuration file is automatically generated at runtime using the ConfigManager. The file is named `SML.cfg` and is located in the game's configuration directory.

## Configuration Settings

### Existing Settings

#### disabledChatCommands
- **Type:** Array of Strings
- **Default:** `[]`
- **Description:** List of fully qualified chat command names that won't be usable by players in the game. Full command name format is `mod_reference:command_name`. Note that this setting only affects players; other command senders can still use disabled commands.
- **Example:** `["SML:help", "MyMod:spawn"]`

#### enableFunchookLogging
- **Type:** Boolean
- **Default:** `false`
- **Description:** Whether to enable funchook logging for debugging purposes.

### New In-Game Chat Settings

#### chatCommandPrefix
- **Type:** String
- **Default:** `"/"`
- **Description:** The prefix character(s) used for chat commands. Players must type this prefix before command names.
- **Example:** `"/"` means players type `/help`, `"!"` means players type `!help`

#### chatCommandCooldown
- **Type:** Number (Float)
- **Default:** `0`
- **Description:** Cooldown duration in seconds between chat commands per player. Set to `0` to disable cooldown.
- **Example:** `5.0` means players must wait 5 seconds between commands

#### maxChatMessageLength
- **Type:** Number (Integer)
- **Default:** `0`
- **Description:** Maximum length for chat messages in characters. Set to `0` for unlimited length.
- **Example:** `256` limits messages to 256 characters

#### enableChatLogging
- **Type:** Boolean
- **Default:** `false`
- **Description:** Whether to enable chat command logging to file for auditing and debugging purposes.

### New Discord Integration Settings

#### enableDiscordIntegration
- **Type:** Boolean
- **Default:** `false`
- **Description:** Master switch to enable Discord integration features. When enabled, the mod can send game events and chat messages to Discord.

#### discordWebhookURL
- **Type:** String
- **Default:** `""`
- **Description:** Discord webhook URL for sending chat messages and notifications. This is required if Discord integration is enabled. You can create a webhook in your Discord server settings.
- **Example:** `"https://discord.com/api/webhooks/123456789/abcdefghijklmnop"`

#### discordBotToken
- **Type:** String
- **Default:** `""`
- **Description:** Discord bot token for advanced bot features (optional). Leave empty to use webhook-only mode, which is sufficient for most use cases. If you need bidirectional communication (Discord to game), you'll need to provide a bot token.
- **Example:** `"YOUR_BOT_TOKEN_HERE"`
- **Note:** Keep this token secret! Never commit it to version control. You can get a bot token from the Discord Developer Portal.

#### discordChannelID
- **Type:** String
- **Default:** `""`
- **Description:** Discord channel ID for receiving messages. This is required if using a bot token. Right-click a channel in Discord with Developer Mode enabled to copy the channel ID.
- **Example:** `"123456789012345678"`

#### discordMessageFormat
- **Type:** String
- **Default:** `"**{player}**: {message}"`
- **Description:** Template for Discord messages sent from in-game chat. Available placeholders:
  - `{player}` - Player name who sent the message
  - `{message}` - The actual chat message content
  - `{timestamp}` - Timestamp when the message was sent
- **Example:** `"[{timestamp}] {player}: {message}"` or `"🎮 **{player}** says: {message}"`

## Example Configuration

Here's an example configuration with Discord integration enabled:

```json
{
  "disabledChatCommands": ["SML:list"],
  "enableFunchookLogging": false,
  "chatCommandPrefix": "/",
  "chatCommandCooldown": 2.0,
  "maxChatMessageLength": 500,
  "enableChatLogging": true,
  "enableDiscordIntegration": true,
  "discordWebhookURL": "https://discord.com/api/webhooks/YOUR_WEBHOOK_ID/YOUR_WEBHOOK_TOKEN",
  "discordBotToken": "",
  "discordChannelID": "",
  "discordMessageFormat": "🎮 **{player}**: {message}"
}
```

## Security Considerations

- Never commit your `discordBotToken` to version control
- Keep your `discordWebhookURL` private to prevent unauthorized message posting
- Use Discord's rate limiting and permissions to prevent abuse
- Consider using webhook-only mode unless you specifically need bot features

## Notes

- The configuration file is automatically created with default values if it doesn't exist
- Changes to the configuration file require a game restart to take effect
- Missing configuration fields will be automatically added with default values when the configuration is loaded
