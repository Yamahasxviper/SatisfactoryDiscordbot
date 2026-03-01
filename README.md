# SatisfactoryModLoader [![CI](https://github.com/satisfactorymodding/SatisfactoryModLoader/actions/workflows/build.yml/badge.svg)](https://github.com/satisfactorymodding/SatisfactoryModLoader/actions/workflows/build.yml)

A tool used to load mods for the game Satisfactory. After Coffee Stain releases a proper Unreal modding API the project will continue as a utilities library.

This repository contains the SatisfactoryModLoader source code,
an ExampleMod with demos of some of the SML utilities,
a collection of editor utilities,
and more.
It also serves as the Unreal project used for developing mods.

## Documentation

Learn how to set up and use this repo on the [modding documentation](https://docs.ficsit.app/).

## Discord Server

Join our [discord server](https://discord.gg/QzcG9nX) to talk about SML and Satisfactory Modding in general.

## Release Notes

### DiscordBridge v1.0.0 – Initial Release

*Compatible with Satisfactory build ≥ 416835 and SML ≥ 3.11.3*

DiscordBridge is a server-only Satisfactory mod that creates a live two-way bridge
between your dedicated server's in-game chat and a Discord text channel via a Discord
bot token. Everything is configured through a single INI file with no external service
or dashboard required.

#### Features

**Two-way chat bridge**
- In-game player messages are forwarded to Discord and Discord messages are relayed into the game in real time.
- Both directions are fully customisable via format strings with placeholder support (`%PlayerName%`, `%Username%`, `%Message%`, `%ServerName%`).
- Discord markdown (bold, italics, code blocks, etc.) is preserved when posting game messages to Discord.
- Bot messages from other Discord bots can be silently dropped (`IgnoreBotMessages`) to prevent echo loops.

**Server status announcements**
- Configurable messages are posted to the bridged channel when the server comes online (`ServerOnlineMessage`) and when it shuts down gracefully (`ServerOfflineMessage`).
- Either notification can be disabled independently by clearing its value.

**Live player-count presence**
- The bot's Discord "Now Playing" activity line displays the current player count in real time.
- The format, update interval (minimum 15 s), and activity verb (Playing / Listening to / Watching / Competing in) are all configurable.
- The presence can be disabled entirely with `ShowPlayerCountInPresence=False`.

**Ban system**
- Manage a server-side ban list directly from the bridged Discord channel or from inside the game using `!ban` commands.
- Banned players are kicked automatically when they attempt to join the server.
- Dedicated Discord channel support (`BanChannelId`) provides an isolated admin audit log for ban events.
- Role-gated commands (`BanCommandRoleId`) restrict who can issue `!ban` commands.
- Role management commands (`!ban role add/remove <discord_id>`) let existing ban admins promote or demote other Discord members without requiring Discord server admin access.
- Configurable kick message shown to the player in-game (`BanKickReason`) and Discord notification posted when a banned player is kicked (`BanKickDiscordMessage`).
- The ban command interface can be disabled independently of ban enforcement (`BanCommandsEnabled=False` keeps bans active without accepting new commands).

| Discord Ban Command | Effect |
|---------------------|--------|
| `!ban on` | Enable the ban system |
| `!ban off` | Disable the ban system |
| `!ban add <name>` | Ban a player by in-game name |
| `!ban remove <name>` | Unban a player by in-game name |
| `!ban list` | List all banned players |
| `!ban status` | Show enabled/disabled state of ban system and whitelist |
| `!ban role add <discord_id>` | Grant the ban admin role to a Discord user |
| `!ban role remove <discord_id>` | Revoke the ban admin role from a Discord user |

In-game ban commands (`!ban on/off/add/remove/list/status`) are also supported, excluding role management which is Discord-only.

**Whitelist**
- Restrict which players can join the server and manage the list from Discord or in-game using `!whitelist` commands.
- Optional Discord role integration (`WhitelistRoleId`): members holding the whitelist role are automatically allowed through the whitelist by display-name matching, without needing a manual `!whitelist add` entry.
- Dedicated whitelist channel (`WhitelistChannelId`) mirrors in-game messages from whitelisted players and accepts Discord messages only from role holders.
- Configurable kick reason (`WhitelistKickReason`) and Discord kick notification (`WhitelistKickDiscordMessage`).

| Discord Whitelist Command | Effect |
|---------------------------|--------|
| `!whitelist on` | Enable the whitelist |
| `!whitelist off` | Disable the whitelist |
| `!whitelist add <name>` | Add a player by in-game name |
| `!whitelist remove <name>` | Remove a player by in-game name |
| `!whitelist list` | List all whitelisted players |
| `!whitelist status` | Show enabled/disabled state of whitelist and ban system |
| `!whitelist role add <discord_id>` | Grant the whitelist role to a Discord user |
| `!whitelist role remove <discord_id>` | Revoke the whitelist role from a Discord user |

In-game whitelist commands (`!whitelist on/off/add/remove/list/status`) are also supported, excluding role management which is Discord-only.

**Configuration**
- Single primary config file (`DefaultDiscordBridge.ini`) covers all settings.
- Separate optional files (`DefaultDiscordBridgeWhitelist.ini`, `DefaultDiscordBridgeBan.ini`) allow whitelist and ban settings to be managed independently. Settings in the separate files take priority over the primary file.
- Automatic credential backup: `BotToken` and `ChannelId` are saved to `<ServerRoot>/FactoryGame/Saved/Config/DiscordBridge.ini` on every startup. If a mod update resets the primary config, the bridge falls back to the backup automatically.
- All ban data is persisted in `<ServerRoot>/FactoryGame/Saved/ServerBanlist.json` and survives server restarts.
- All whitelist data is persisted in `<ServerRoot>/FactoryGame/Saved/ServerWhitelist.json` and survives server restarts.

#### Requirements

| Dependency | Minimum version |
|------------|----------------|
| Satisfactory (dedicated server) | build ≥ 416835 |
| SML | ≥ 3.11.3 |
| SMLWebSocket | ≥ 1.0.0 |

The Discord bot must have the following **Privileged Gateway Intents** enabled in the Discord Developer Portal:
- Presence Intent
- Server Members Intent
- Message Content Intent

The bot also needs **Send Messages** and **Read Message History** permissions in the target channel. The **Manage Roles** permission is required when using `!ban role` or `!whitelist role` commands.

#### Getting Started

See the [Getting Started guide](Mods/DiscordBridge/Docs/01-GettingStarted.md) and the [full documentation](Mods/DiscordBridge/Docs/README.md) for complete setup instructions.

For further help visit the Satisfactory Modding Discord: <https://discord.gg/xkVJ73E>

---

For full release notes, see [Mods/DiscordBridge/RELEASE_NOTES.md](Mods/DiscordBridge/RELEASE_NOTES.md).

## DISCLAIMER

This software is provided by the author "as is". In no event shall the author be liable for any direct, indirect, incidental, special, exemplary, or consequential damages (including, but not limited to procurement of substitute goods or services; loss of use, data, or profits; or business interruption) however caused and on any
theory of liability, whether in contract, strict liability, or tort (including negligence or otherwise) arising in any way out of the use of this software, even if advised of the possibility of such damage.
