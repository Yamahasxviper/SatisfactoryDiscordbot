# DiscordBridge – Getting Started

← [Back to index](README.md)

This guide covers how to install the mod, where the configuration file lives, and how to create the Discord bot that powers the bridge.

---

## Step 0 – Install the mod

The easiest way to install DiscordBridge is through the **Satisfactory Mod Manager (SMM)**:

1. Download and install [SMM](https://smm.ficsit.app/) if you have not done so already.
2. Open SMM, select your Satisfactory installation, then search for **DiscordBridge** in the mods list.
3. Click **Install**. SMM handles all dependencies (SML, SMLWebSocket) automatically.
4. Launch / restart your dedicated server. The mod loads automatically on startup.

> **Dedicated server without SMM**
> Copy the extracted mod folder (`DiscordBridge/`) into
> `<ServerRoot>/FactoryGame/Mods/` manually.
> Ensure SML and SMLWebSocket are present in that same `Mods/` directory.

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
3. Click **Reset Token** and copy the token – paste it as `BotToken` in the config.
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

## Next steps

Once your bot is created, configure the connection in [Connection Settings](02-ConnectionSettings.md).

---

*For further help visit the Satisfactory Modding Discord: <https://discord.gg/xkVJ73E>*
