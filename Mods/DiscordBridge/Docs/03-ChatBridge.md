# DiscordBridge – Chat Bridge (Message Formatting)

← [Back to index](README.md)

These settings control how messages are formatted when they cross the bridge in
either direction.

---

## `GameToDiscordFormat`

Format string applied when an in-game player chat message is forwarded to Discord.

| Placeholder | Replaced with |
|-------------|---------------|
| `%ServerName%` | The value of `ServerName` |
| `%PlayerName%` | The in-game name of the player who sent the message |
| `%Message%` | The raw chat message text |

**Default:** `**%PlayerName%**: %Message%`

**Examples:**

```ini
; Bold name, plain message
GameToDiscordFormat=**%PlayerName%**: %Message%

; Include server label
GameToDiscordFormat=**[%ServerName%] %PlayerName%**: %Message%

; Code-formatted name
GameToDiscordFormat=`[%PlayerName%]` %Message%
```

---

## `DiscordToGameFormat`

Format string used as the **message body** shown in the Satisfactory in-game chat
when a Discord message is relayed into the game.

| Placeholder | Replaced with |
|-------------|---------------|
| `%Username%` | The Discord display name of the sender |
| `%PlayerName%` | Alias for `%Username%` |
| `%Message%` | The Discord message text |

**Default:** `%Message%` *(just the raw message)*

**Examples:**

```ini
; Raw message only (default)
DiscordToGameFormat=%Message%

; Prepend the sender name
DiscordToGameFormat=%Username%: %Message%

; Tag Discord messages
DiscordToGameFormat=[Discord] %Message%
```

---

## `DiscordSenderFormat`

Format string used for the **sender name column** displayed in the Satisfactory chat
UI when a Discord message arrives.

| Placeholder | Replaced with |
|-------------|---------------|
| `%Username%` | The Discord display name of the sender |
| `%PlayerName%` | Alias for `%Username%` |

**Default:** `[Discord] %Username%`

**Examples:**

```ini
; Default – prefixed with [Discord]
DiscordSenderFormat=[Discord] %Username%

; Name only, no prefix
DiscordSenderFormat=%Username%

; Branded prefix
DiscordSenderFormat=[Satisfactory] %Username%
```

---

*For further help visit the Satisfactory Modding Discord: <https://discord.gg/xkVJ73E>*
