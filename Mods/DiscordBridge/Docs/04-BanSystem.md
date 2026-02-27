# DiscordBridge – Ban System

← [Back to index](README.md)

The built-in ban system lets you manage player bans directly from the bridged Discord channel.
Bans are stored in `<ServerRoot>/FactoryGame/Saved/ServerBanlist.json` and persist across
server restarts automatically.

> **The ban system and the whitelist are completely independent.**
> You can use either one, both, or neither — enabling or disabling one never affects the other.
>
> | Goal | Config |
> |------|--------|
> | Ban system only | `WhitelistEnabled=False`, `BanSystemEnabled=True` *(default)* |
> | Whitelist only | `WhitelistEnabled=True`, `BanSystemEnabled=False` |
> | Both | `WhitelistEnabled=True`, `BanSystemEnabled=True` |
> | Neither | `WhitelistEnabled=False`, `BanSystemEnabled=False` |

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

### `BanCommandRoleId`

The snowflake ID of the Discord role whose members are allowed to run `!ban` management commands.

**Default:** *(empty — !ban commands are disabled for all Discord users)*

When set, **only members who hold this role** can run `!ban` commands in the bridged Discord channel. When left empty, `!ban` commands are fully disabled (deny-by-default) — no one can run them until a role ID is provided.

> **IMPORTANT:** Holding this role does **not** bypass the ban check when joining the game. If a role-holder's in-game name is on the ban list they will still be kicked.

**How to get the role ID:**
Enable Developer Mode in Discord (User Settings → Advanced → Developer Mode), then right-click the role in Server Settings → Roles and choose **Copy Role ID**.

**Example:**
```ini
BanCommandRoleId=987654321098765432
```

The ban admin role and the whitelist admin role are **completely independent** — you can assign different roles to each, or use the same role for both.

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
| `!ban status` | Show the current enabled/disabled state of **both** the ban system and the whitelist |
| `!ban role add <discord_id>` | Grant the `BanRoleId` Discord role to a user |
| `!ban role remove <discord_id>` | Revoke the `BanRoleId` Discord role from a user |

---

### `BanCommandsEnabled`

Master on/off switch for the **ban command interface** — controls whether `!ban` Discord and in-game commands are accepted at all.

**Default:** `True`

| Value | Effect |
|-------|--------|
| `True` | `!ban` commands work normally (still gated by `BanCommandRoleId`) |
| `False` | All `!ban` commands are silently ignored; existing bans are **still enforced** on join; `BanSystemEnabled` is unaffected |

This gives a clean "on/off from config" for the command interface, independently of whether ban enforcement itself is active. Unlike `BanCommandPrefix=` (which disables commands by removing the trigger word), `BanCommandsEnabled=False` provides an explicit config toggle that's easy to find and understand.

**Example — disable ban commands, keep enforcement:**
```ini
BanSystemEnabled=True
BanCommandsEnabled=False
```

---

### `BanChannelId`

The snowflake ID of a **dedicated Discord channel** for ban management.
Leave **empty** to disable the ban-only channel.

**Default:** *(empty)*

When set:
- `!ban` commands issued from this channel are accepted. The sender must still hold `BanCommandRoleId`. Responses are sent back to this channel (not the main channel).
- Ban-kick notifications are **also** posted here in addition to the main `ChannelId`, giving admins a focused audit log of all ban events.
- Non-ban-command messages in this channel are silently ignored (the channel is admin-only and not bridged to game chat).

**How to get the channel ID:**
Enable Developer Mode in Discord (User Settings → Advanced → Developer Mode), then right-click the channel and choose **Copy Channel ID**.

**Example:**
```ini
BanChannelId=567890123456789012
```

> **Tip:** You can use `BanChannelId` together with `BanCommandRoleId` for a fully locked-down ban management workflow: create a private admin channel, add the bot to it, set `BanChannelId` to its ID and `BanCommandRoleId` to your admin role — only admins can see the channel and only they can issue ban commands from it.

---

### `BanRoleId`

The snowflake ID of the Discord role granted or revoked by `!ban role add/remove <discord_id>` commands.
Leave **empty** to disable Discord ban-role management.

**Default:** *(empty)*

When set:
- `!ban role add <discord_user_id>` grants this role to a Discord member.
- `!ban role remove <discord_user_id>` revokes this role from a Discord member.
- The bot must have the **Manage Roles** permission in your Discord server.

**Typical setup:** set `BanRoleId` to the same snowflake as `BanCommandRoleId`.  This lets ban-command administrators promote or demote each other from within Discord, without needing direct access to Discord server settings.

**How to get the role ID:**
Enable Developer Mode in Discord (User Settings → Advanced → Developer Mode), then right-click the role in Server Settings → Roles and choose **Copy Role ID**.

**Example:**
```ini
BanCommandRoleId=987654321098765432
BanRoleId=987654321098765432   ; same role — admins can grant each other ban access
```

---

The snowflake ID of a **dedicated Discord channel** for ban management.
Leave **empty** to disable the ban-only channel.

**Default:** *(empty)*

When set:
- `!ban` commands issued from this channel are accepted. The sender must still hold `BanCommandRoleId`. Responses are sent back to this channel (not the main channel).
- Ban-kick notifications are **also** posted here in addition to the main `ChannelId`, giving admins a focused audit log of all ban events.
- Non-ban-command messages in this channel are silently ignored (the channel is admin-only and not bridged to game chat).

**How to get the channel ID:**
Enable Developer Mode in Discord (User Settings → Advanced → Developer Mode), then right-click the channel and choose **Copy Channel ID**.

**Example:**
```ini
BanChannelId=567890123456789012
```

> **Tip:** You can use `BanChannelId` together with `BanCommandRoleId` for a fully locked-down ban management workflow: create a private admin channel, add the bot to it, set `BanChannelId` to its ID and `BanCommandRoleId` to your admin role — only admins can see the channel and only they can issue ban commands from it.

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

### `InGameBanCommandPrefix`

The prefix that triggers ban management commands when typed in the **Satisfactory in-game chat**.
This lets server admins manage bans directly from inside the game without needing Discord.
Set to an **empty string** to disable in-game ban commands.

**Default:** `!ban`

**Supported commands** (type these in the Satisfactory in-game chat):

| Command | Effect |
|---------|--------|
| `!ban on` | Enable the ban system |
| `!ban off` | Disable the ban system |
| `!ban add <name>` | Ban a player by in-game name |
| `!ban remove <name>` | Unban a player by in-game name |
| `!ban list` | List all banned players |
| `!ban status` | Show the current enabled/disabled state of **both** the ban system and the whitelist |

> **Note:** In-game ban commands support the same operations as the Discord commands,
> except for role management which is Discord-only.

---

*For further help visit the Satisfactory Modding Discord: <https://discord.gg/xkVJ73E>*
