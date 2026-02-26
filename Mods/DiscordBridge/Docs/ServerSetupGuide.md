# DiscordBridge – Server Setup & Configuration Guide

This guide explains every setting available in `DefaultDiscordBridge.ini` so you can
get the Discord ↔ Satisfactory chat bridge running and fine-tune it to your liking.

---

## Where is the config file?

After installing the mod the **primary config** lives at:

```
<ServerRoot>/FactoryGame/Mods/DiscordBridge/Config/DefaultDiscordBridge.ini
```

Edit that file, then restart the server. Your changes take effect on the next start.

> **Tip – surviving mod updates**
> The mod automatically writes a backup of your `BotToken` and `ChannelId` to
> `<ServerRoot>/FactoryGame/Saved/Config/DiscordBridge.ini` every time the server
> starts. If a mod update resets the primary config, the bridge falls back to that
> backup so it keeps working until you copy your credentials back.

---

## Step 1 – Create a Discord Bot

1. Go to <https://discord.com/developers/applications> and click **New Application**.
2. Give it a name (e.g. *My Satisfactory Bot*), then open the **Bot** tab.
3. Click **Reset Token** and copy the token – paste it as `BotToken` below.
4. Under **Privileged Gateway Intents** enable all three:
   - **Presence Intent**
   - **Server Members Intent**
   - **Message Content Intent**
5. Under **OAuth2 → URL Generator** tick `bot`, then tick the permissions
   **Send Messages** and **Read Message History**.
6. Open the generated URL in a browser and invite the bot to your server.
7. Enable **Developer Mode** in Discord (User Settings → Advanced), right-click
   the target text channel, and choose **Copy Channel ID**. Paste it as `ChannelId`.

---

## Setting Reference

### CONNECTION

| Setting    | Type   | Default | Description |
|------------|--------|---------|-------------|
| `BotToken` | string | *(empty)* | Your Discord bot token. Treat this like a password – never share it. The bridge will not start if this is empty. |
| `ChannelId` | string | *(empty)* | The snowflake ID of the Discord text channel to bridge. Must be a numeric ID (e.g. `123456789012345678`). |
| `ServerName` | string | *(empty)* | A display name for this server. Used as the `%ServerName%` placeholder in message formats and the player-count presence. Example: `My Satisfactory Server`. |

---

### CHAT TWO-WAY CUSTOMISATION

These settings control how messages appear on each side of the bridge.

#### `GameToDiscordFormat`

Format string applied when an in-game player chat message is forwarded to Discord.

| Placeholder   | Replaced with |
|---------------|---------------|
| `%ServerName%` | The value of `ServerName` |
| `%PlayerName%` | The in-game name of the player who sent the message |
| `%Message%`    | The raw chat message text |

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

#### `DiscordToGameFormat`

Format string used as the **message body** shown in the Satisfactory in-game chat
when a Discord message is relayed into the game.

| Placeholder   | Replaced with |
|---------------|---------------|
| `%Username%`   | The Discord display name of the sender |
| `%PlayerName%` | Alias for `%Username%` |
| `%Message%`    | The Discord message text |

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

#### `DiscordSenderFormat`

Format string used for the **sender name column** displayed in the Satisfactory chat
UI when a Discord message arrives.

| Placeholder   | Replaced with |
|---------------|---------------|
| `%Username%`   | The Discord display name of the sender |
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

### BAN SYSTEM

Controls the built-in player ban system that can be managed from Discord.
Bans are stored in `<ServerRoot>/FactoryGame/Saved/ServerBanlist.json` and
persist across server restarts automatically.

#### `BanSystemEnabled`

Sets the initial ban-system state on the **first** server start (when
`ServerBanlist.json` does not yet exist).  After the first start the
enabled/disabled state is saved in `ServerBanlist.json` and survives restarts
automatically — so `!ban on` / `!ban off` changes made from Discord truly
persist.

To force-reset back to this config value: delete `ServerBanlist.json` and
restart the server.

**Default:** `True` (banned players are kicked on join)

---

#### `BanCommandPrefix`

The prefix that triggers ban management commands when typed in the bridged Discord channel.
Set to an **empty string** to disable Discord-based ban management entirely.

**Default:** `!ban`

**Supported commands** (type these in the bridged Discord channel):

| Command | Effect |
|---------|--------|
| `!ban on` | Enable the ban system — kicked on join (persists across restarts) |
| `!ban off` | Disable the ban system — banned players can join freely (persists across restarts) |
| `!ban add <name>` | Ban a player by in-game name |
| `!ban remove <name>` | Unban a player by in-game name |
| `!ban list` | List all banned players and current enabled/disabled state |
| `!ban status` | Show whether the ban system is currently enabled or disabled |

---

#### `BanKickDiscordMessage`

The message posted to the **main** Discord channel whenever a banned player attempts to join and is kicked.
Leave **empty** to disable this notification.

**Default:** `:hammer: **%PlayerName%** is banned from this server and was kicked.`

| Placeholder | Replaced with |
|-------------|---------------|
| `%PlayerName%` | The in-game name of the banned player who was kicked |

**Example:**

```ini
BanKickDiscordMessage=:no_entry: **%PlayerName%** is banned and was removed.
```

---

#### `BanKickReason`

The reason shown **in-game** to the player when they are kicked for being banned.
This is the text the player sees in the disconnected / kicked screen.

**Default:** `You are banned from this server.`

**Example:**

```ini
BanKickReason=You have been banned. Contact the server admin to appeal.
```

---

### WHITELIST

Controls the built-in server whitelist that can be managed from Discord.

#### `WhitelistCommandPrefix`

The prefix that triggers whitelist management commands when typed in the bridged Discord channel.
Set to an **empty string** to disable Discord-based whitelist management entirely.

**Default:** `!whitelist`

**Supported commands** (type these in the bridged Discord channel):

| Command | Effect |
|---------|--------|
| `!whitelist on` | Enable the whitelist (only whitelisted players can join) |
| `!whitelist off` | Disable the whitelist (all players can join) |
| `!whitelist add <name>` | Add a player by in-game name |
| `!whitelist remove <name>` | Remove a player by in-game name |
| `!whitelist list` | List all whitelisted players |
| `!whitelist status` | Show whether the whitelist is currently enabled or disabled |
| `!whitelist role add <discord_id>` | Grant the `WhitelistRoleId` Discord role to a user |
| `!whitelist role remove <discord_id>` | Revoke the `WhitelistRoleId` Discord role from a user |

---

#### `WhitelistRoleId`

The snowflake ID of the Discord role used to identify whitelisted members.
Leave **empty** to disable Discord role integration.

**Default:** *(empty)*

When set:
- Discord messages sent to `WhitelistChannelId` are relayed to the game **only when the sender holds this role**.
- The `!whitelist role add/remove <discord_id>` commands assign or revoke this role via the Discord REST API (the bot must have the **Manage Roles** permission on your server).

**How to get the role ID:**
Enable Developer Mode in Discord (User Settings → Advanced → Developer Mode), then right-click the role in Server Settings → Roles and choose **Copy Role ID**.

---

#### `WhitelistChannelId`

The snowflake ID of a dedicated Discord channel for whitelisted members.
Leave **empty** to disable the whitelist-only channel.

**Default:** *(empty)*

When set:
- In-game messages from players on the server whitelist are **also** posted to this channel (in addition to the main `ChannelId`).
- Discord messages sent to this channel are relayed to the game **only when the sender holds `WhitelistRoleId`** (if `WhitelistRoleId` is configured).

Get the channel ID the same way as `ChannelId` (right-click the channel in Discord with Developer Mode enabled → **Copy Channel ID**).

---

#### `WhitelistKickDiscordMessage`

The message posted to the **main** Discord channel whenever a non-whitelisted player attempts to join and is kicked.
Leave **empty** to disable this notification.

**Default:** `:boot: **%PlayerName%** tried to join but is not on the whitelist and was kicked.`

| Placeholder | Replaced with |
|-------------|---------------|
| `%PlayerName%` | The in-game name of the player who was kicked |

**Example:**

```ini
WhitelistKickDiscordMessage=:no_entry: **%PlayerName%** is not whitelisted and was removed from the server.
```

---

### BEHAVIOUR

| Setting             | Type | Default | Description |
|---------------------|------|---------|-------------|
| `bIgnoreBotMessages` | bool | `True` | When `True`, messages from Discord bot accounts are silently dropped. This prevents echo loops when other bots are active in the same channel. Set to `False` only if you intentionally want bot messages relayed into the game. |

---

### SERVER STATUS MESSAGES

These messages are posted to the bridged Discord channel when the server starts or stops.
Leave a value **empty** to disable that notification.

| Setting               | Default value | Description |
|-----------------------|---------------|-------------|
| `ServerOnlineMessage`  | `:green_circle: Server is now **online**!` | Posted when the dedicated server finishes loading and the bridge connects. |
| `ServerOfflineMessage` | `:red_circle: Server is now **offline**.` | Posted when the server shuts down gracefully. |

**Examples:**

```ini
; Custom emoji + text
ServerOnlineMessage=🟢 **%ServerName%** is back online!
ServerOfflineMessage=🔴 **%ServerName%** has gone offline.

; Disable offline notification entirely
ServerOfflineMessage=
```

---

### PLAYER COUNT PRESENCE

Controls the **Discord bot status** (the "Now Playing" activity line shown on the
bot's profile).

| Setting | Type | Default | Description |
|---------|------|---------|-------------|
| `bShowPlayerCountInPresence` | bool | `True` | When `True`, the bot's Discord status is refreshed periodically to show the current player count. Set to `False` to leave the bot status blank. |
| `PlayerCountPresenceFormat` | string | *(empty)* | Text shown in the bot's Discord presence. Use `%PlayerCount%` for the live player count and `%ServerName%` for the server name. Leave empty to show nothing. |
| `PlayerCountUpdateIntervalSeconds` | float | `60.0` | How often (in seconds) the presence is refreshed. **Minimum is 15 seconds.** Values below 15 are clamped to 15 to respect Discord's rate limits. |
| `PlayerCountActivityType` | int | `0` | Controls the activity verb shown before the presence text in Discord. See table below. |

**Activity type values:**

| Value | Discord verb |
|-------|-------------|
| `0`   | Playing |
| `2`   | Listening to |
| `3`   | Watching |
| `5`   | Competing in |

**Examples:**

```ini
; "Playing Satisfactory with 4 players"
PlayerCountPresenceFormat=Satisfactory with %PlayerCount% players
PlayerCountActivityType=0

; "Watching My Server – 4 online"
PlayerCountPresenceFormat=%ServerName% – %PlayerCount% online
PlayerCountActivityType=3

; Disable presence entirely
bShowPlayerCountInPresence=False
```

---

## Troubleshooting

### The bridge does not start / no messages are relayed

1. Check that `BotToken` and `ChannelId` are both set in the config file.
2. Confirm the bot is **in your Discord server** and has **Send Messages** and
   **Read Message History** permissions in the target channel.
3. Make sure all three **Privileged Gateway Intents** are enabled in the Discord
   Developer Portal (Presence, Server Members, Message Content).
4. Look in the server log (`FactoryGame.log`) for lines starting with `LogDiscordBridge`
   – they will contain details about any connection errors.

### Messages go one way only (game → Discord works, Discord → game doesn't)

- Verify **Message Content Intent** is enabled. Without it Discord does not send
  message content to bots, so the bridge cannot read Discord messages.

### The bot shows "offline" in Discord even while the server is running

- Discord caches presence state. Wait up to a minute or try restarting your Discord client.
- Make sure `bShowPlayerCountInPresence=True` and `PlayerCountPresenceFormat` is
  not empty.

### Messages from other bots are relayed into the game (echo loop)

- Set `bIgnoreBotMessages=True` (this is the default). This drops messages from
  any Discord account that has the `bot` flag set.

### The config gets reset after a mod update

- This is expected behaviour for the primary config. Your credentials are saved
  automatically to `<ServerRoot>/FactoryGame/Saved/Config/DiscordBridge.ini` each
  session and will be restored automatically. If you want to be safe, keep a
  separate copy of your `BotToken` and `ChannelId` somewhere secure.

### Whitelist commands are not recognised / players are not being kicked

1. Make sure `WhitelistCommandPrefix` is set (default is `!whitelist`) and not empty.
2. Confirm the whitelist is **enabled** (`!whitelist status` in the Discord channel).
3. If using `WhitelistRoleId`, verify the bot has the **Manage Roles** permission on your Discord server.
4. Players are only kicked on **join** – the whitelist is checked when a player connects, not while they are already in the game.

### Ban commands are not recognised / banned players can still join

1. Make sure `BanCommandPrefix` is set (default is `!ban`) and not empty.
2. Run `!ban status` in the Discord channel to confirm the ban system is **enabled**. If it shows disabled, run `!ban on`.
3. Remember that `BanSystemEnabled` in the config file is only applied on the **very first** server start (when `ServerBanlist.json` does not exist). After that, use `!ban on` / `!ban off` — the state is saved automatically in `ServerBanlist.json`.
4. If you need to force the system back to the config-file value, stop the server, delete `<ServerRoot>/FactoryGame/Saved/ServerBanlist.json`, and restart.
5. Players are only kicked on **join** — a ban takes effect the next time a banned player tries to connect, not while they are already in the game.

### Log verbosity

Add the following to your server's `DefaultEngine.ini` to increase log detail:

```ini
[Core.Log]
LogDiscordBridge=Verbose
```

---

*For further help visit the Satisfactory Modding Discord: <https://discord.gg/xkVJ73E>*
