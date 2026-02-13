# SML Configuration - Chat and Discord Name Formatting

## Overview

The SML configuration now supports custom name formatting for chat and Discord integration, allowing mods to format player names when bridging between in-game chat and Discord.

## Configuration Options

### `inGameChatNameFormat` (string)
Format string for displaying Discord names in in-game chat. Use `{name}` as a placeholder for the actual name.

**Default:** `"{name}"` (no formatting)

**Example:** `"[Discord] {name}"` will display as `[Discord] Username`

### `discordNameFormat` (string)
Format string for displaying in-game names in Discord. Use `{name}` as a placeholder for the actual name.

**Default:** `"{name}"` (no formatting)

**Example:** `"[Game] {name}"` will display as `[Game] PlayerName`

### `enableNameFormatting` (boolean)
Enables or disables name formatting for chat and Discord integration.

**Default:** `false` (disabled)

## Usage Example

To configure name formatting in your SML configuration JSON file:

```json
{
  "enableNameFormatting": true,
  "inGameChatNameFormat": "[Discord] {name}",
  "discordNameFormat": "[Game] {name}"
}
```

## Use Cases

This feature is particularly useful for Discord bot mods that:
- Bridge chat between Discord and the in-game chat system
- Need to distinguish between messages from different sources
- Want to provide context about where messages originated

## Notes

- The `{name}` placeholder will be replaced with the actual player or Discord username
- Name formatting only applies when `enableNameFormatting` is set to `true`
- These settings are read by the SML configuration system and made available to mods
- Individual mods are responsible for implementing the actual name formatting logic using these configuration values
