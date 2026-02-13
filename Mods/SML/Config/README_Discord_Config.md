# Discord and In-Game Chat Configuration

This document describes the custom configuration settings available for Discord integration and in-game chat customization in Satisfactory Mod Loader (SML).

## Configuration File

The SML configuration is stored in a JSON file. You can edit this file to customize Discord integration and chat behavior.

## Discord Integration Settings

### `enableDiscordIntegration`
- **Type:** Boolean
- **Default:** `false`
- **Description:** Master switch to enable or disable Discord integration features. When disabled, all other Discord-related settings are ignored.

### `discordWebhookURL`
- **Type:** String
- **Default:** `""` (empty)
- **Description:** The Discord webhook URL for sending messages to Discord. Get this from your Discord server settings.
- **Example:** `"https://discord.com/api/webhooks/123456789/abcdefghijk"`

### `discordBotToken`
- **Type:** String
- **Default:** `""` (empty)
- **Description:** Optional Discord bot token for advanced features. Leave empty if using webhook-only mode.
- **Security Note:** Keep this token secret and never share it publicly.

### `discordChannelID`
- **Type:** String
- **Default:** `""` (empty)
- **Description:** Optional Discord channel ID for receiving messages from Discord. Leave empty if not receiving messages from Discord to the game.

## In-Game Chat Settings

### `relayChatToDiscord`
- **Type:** Boolean
- **Default:** `true`
- **Description:** When enabled, in-game chat messages will be sent to Discord via the configured webhook.

### `relayDiscordToChat`
- **Type:** Boolean
- **Default:** `true`
- **Description:** When enabled, messages from Discord will be displayed in the in-game chat.

### `chatToDiscordPrefix`
- **Type:** String
- **Default:** `"[Game] "`
- **Description:** Prefix added to messages sent from the game to Discord.
- **Example:** `"[Satisfactory] "` or `"🎮 "`

### `chatToDiscordSuffix`
- **Type:** String
- **Default:** `""` (empty)
- **Description:** Suffix added to messages sent from the game to Discord.

### `discordToChatPrefix`
- **Type:** String
- **Default:** `"[Discord] "`
- **Description:** Prefix added to Discord messages displayed in the game chat.
- **Example:** `"[💬] "` or `"[Discord User] "`

### `discordToChatSuffix`
- **Type:** String
- **Default:** `""` (empty)
- **Description:** Suffix added to Discord messages displayed in the game chat.

### `filterSystemMessages`
- **Type:** Boolean
- **Default:** `true`
- **Description:** When enabled, only player messages are sent to Discord. System messages (like server announcements) are filtered out.

### `includePlayerNameInDiscord`
- **Type:** Boolean
- **Default:** `true`
- **Description:** When enabled, player names are included in messages sent to Discord.

### `discordMessageFormat`
- **Type:** String
- **Default:** `"**{PlayerName}**: {Message}"`
- **Description:** Custom format for chat messages sent to Discord. Supports the following placeholders:
  - `{PlayerName}` - Name of the player who sent the message
  - `{Message}` - The message content
  - `{Timestamp}` - Time when the message was sent
- **Examples:**
  - `"**{PlayerName}**: {Message}"` (default, bold player name)
  - `"{PlayerName} says: {Message}"`
  - `"[{Timestamp}] {PlayerName}: {Message}"`

## Example Configuration

Here's a complete example of a Discord configuration:

```json
{
  "disabledChatCommands": [],
  "enableFunchookLogging": false,
  "enableDiscordIntegration": true,
  "discordWebhookURL": "https://discord.com/api/webhooks/123456789012345678/abcdefghijklmnopqrstuvwxyz",
  "discordBotToken": "",
  "discordChannelID": "",
  "relayChatToDiscord": true,
  "relayDiscordToChat": true,
  "chatToDiscordPrefix": "[Satisfactory] ",
  "chatToDiscordSuffix": "",
  "discordToChatPrefix": "[Discord] ",
  "discordToChatSuffix": "",
  "filterSystemMessages": true,
  "includePlayerNameInDiscord": true,
  "discordMessageFormat": "**{PlayerName}**: {Message}"
}
```

## How to Get a Discord Webhook URL

1. Open your Discord server
2. Go to Server Settings → Integrations → Webhooks
3. Click "New Webhook"
4. Customize the webhook (name, channel, avatar)
5. Click "Copy Webhook URL"
6. Paste the URL into the `discordWebhookURL` configuration field

## Security Considerations

- **Never share your webhook URL or bot token publicly** - anyone with these credentials can send messages to your Discord server
- Store configuration files securely and don't commit them to public repositories
- Use environment-specific configuration files for different servers
- Regularly rotate bot tokens and webhooks if they're compromised

## Troubleshooting

### Messages aren't appearing in Discord
- Verify `enableDiscordIntegration` is set to `true`
- Check that `relayChatToDiscord` is enabled
- Ensure the `discordWebhookURL` is valid and hasn't expired
- Verify the webhook still exists in your Discord server

### Discord messages aren't appearing in-game
- Verify `relayDiscordToChat` is enabled
- Check that the Discord bot has proper permissions
- Ensure `discordBotToken` and `discordChannelID` are correctly configured

### System messages are being sent to Discord
- Set `filterSystemMessages` to `true` to filter out system messages
