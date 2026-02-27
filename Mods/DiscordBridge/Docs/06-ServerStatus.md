# DiscordBridge – Server Status Messages & Behaviour

← [Back to index](README.md)

---

## Server Status Messages

These messages are posted to the bridged Discord channel when the server starts or stops.
Leave a value **empty** to disable that notification.

| Setting | Default value | Description |
|---------|---------------|-------------|
| `ServerOnlineMessage` | `:green_circle: Server is now **online**!` | Posted when the dedicated server finishes loading and the bridge connects. |
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

## Behaviour

| Setting | Type | Default | Description |
|---------|------|---------|-------------|
| `IgnoreBotMessages` | bool | `True` | When `True`, messages from Discord bot accounts are silently dropped. This prevents echo loops when other bots are active in the same channel. Set to `False` only if you intentionally want bot messages relayed into the game. |

---

*For further help visit the Satisfactory Modding Discord: <https://discord.gg/xkVJ73E>*
