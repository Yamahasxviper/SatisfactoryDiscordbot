# DiscordBridge – Whitelist

← [Back to index](README.md)

The built-in whitelist lets you restrict which players can join the server and manage
the list directly from the bridged Discord channel.

> **The whitelist and the ban system are completely independent.**
> You can use either one, both, or neither — enabling or disabling one never affects the other.
>
> | Goal | Config |
> |------|--------|
> | Whitelist only | `WhitelistEnabled=True`, `BanSystemEnabled=False` |
> | Ban system only | `WhitelistEnabled=False`, `BanSystemEnabled=True` *(default)* |
> | Both | `WhitelistEnabled=True`, `BanSystemEnabled=True` |
> | Neither | `WhitelistEnabled=False`, `BanSystemEnabled=False` |

---

## Settings

### `WhitelistEnabled`

Sets the initial whitelist state on the **first** server start (when
`ServerWhitelist.json` does not yet exist). After the first start the
enabled/disabled state is saved in `ServerWhitelist.json` and survives restarts
automatically — so `!whitelist on` / `!whitelist off` changes made from Discord
truly persist.

To force-reset back to this config value: delete `ServerWhitelist.json` and restart the server.

**Default:** `False` (all players can join)

---

### `WhitelistCommandPrefix`

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
| `!whitelist status` | Show the current enabled/disabled state of **both** the whitelist and the ban system |
| `!whitelist role add <discord_id>` | Grant the `WhitelistRoleId` Discord role to a user |
| `!whitelist role remove <discord_id>` | Revoke the `WhitelistRoleId` Discord role from a user |

---

### `WhitelistRoleId`

The snowflake ID of the Discord role used to identify whitelisted members.
Leave **empty** to disable Discord role integration.

**Default:** *(empty)*

When set:
- Discord messages sent to `WhitelistChannelId` are relayed to the game **only when the sender holds this role**.
- The `!whitelist role add/remove <discord_id>` commands assign or revoke this role via the Discord REST API (the bot must have the **Manage Roles** permission on your server).

**How to get the role ID:**
Enable Developer Mode in Discord (User Settings → Advanced → Developer Mode), then right-click the role in Server Settings → Roles and choose **Copy Role ID**.

---

### `WhitelistChannelId`

The snowflake ID of a dedicated Discord channel for whitelisted members.
Leave **empty** to disable the whitelist-only channel.

**Default:** *(empty)*

When set:
- In-game messages from players on the server whitelist are **also** posted to this channel (in addition to the main `ChannelId`).
- Discord messages sent to this channel are relayed to the game **only when the sender holds `WhitelistRoleId`** (if `WhitelistRoleId` is configured).

Get the channel ID the same way as `ChannelId` (right-click the channel in Discord with Developer Mode enabled → **Copy Channel ID**).

---

### `WhitelistKickDiscordMessage`

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

### `WhitelistKickReason`

The reason shown **in-game** to the player when they are kicked for not being on the whitelist.
This is the text the player sees in the disconnected / kicked screen.

**Default:** `You are not on this server's whitelist. Contact the server admin to be added.`

**Example:**

```ini
WhitelistKickReason=You are not whitelisted. DM an admin on Discord to request access.
```

---

### `InGameWhitelistCommandPrefix`

The prefix that triggers whitelist management commands when typed in the **Satisfactory in-game chat**.
This lets server admins manage the whitelist directly from inside the game without needing Discord.
Set to an **empty string** to disable in-game whitelist commands.

**Default:** `!whitelist`

**Supported commands** (type these in the Satisfactory in-game chat):

| Command | Effect |
|---------|--------|
| `!whitelist on` | Enable the whitelist |
| `!whitelist off` | Disable the whitelist (all players can join) |
| `!whitelist add <name>` | Add a player by in-game name |
| `!whitelist remove <name>` | Remove a player by in-game name |
| `!whitelist list` | List all whitelisted players |
| `!whitelist status` | Show the current enabled/disabled state of **both** the whitelist and the ban system |

> **Note:** In-game whitelist commands support the same operations as the Discord commands,
> except for role management (`!whitelist role add/remove`) which is Discord-only.

---

*For further help visit the Satisfactory Modding Discord: <https://discord.gg/xkVJ73E>*
