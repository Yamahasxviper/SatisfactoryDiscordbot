# DiscordBridge – Ban System

← [Back to index](README.md)

The built-in ban system lets you manage player bans directly from the bridged Discord channel.
Bans are stored in `<ServerRoot>/FactoryGame/Saved/ServerBanlist.json` and persist across
server restarts automatically.

---

## Settings

### `BanSystemEnabled`

Sets the initial ban-system state on the **first** server start (when
`ServerBanlist.json` does not yet exist). After the first start the
enabled/disabled state is saved in `ServerBanlist.json` and survives restarts
automatically — so `!ban on` / `!ban off` changes made from Discord truly persist.

To force-reset back to this config value: delete `ServerBanlist.json` and restart the server.

**Default:** `True` (banned players are kicked on join)

---

### `BanCommandPrefix`

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

### `BanKickDiscordMessage`

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

### `BanKickReason`

The reason shown **in-game** to the player when they are kicked for being banned.
This is the text the player sees in the disconnected / kicked screen.

**Default:** `You are banned from this server.`

**Example:**

```ini
BanKickReason=You have been banned. Contact the server admin to appeal.
```

---

*For further help visit the Satisfactory Modding Discord: <https://discord.gg/xkVJ73E>*
