# DiscordBot — SML Plugin

Integrates a Discord bot into Satisfactory by polling the **Discord REST API** via UE's
built-in `HTTP` module — no WebSocket module required.

## Why HTTP polling instead of WebSockets?

Satisfactory runs on a **custom Unreal Engine build (5.3.2-CSS)** that does not include the
engine's optional `WebSockets` module.  The `HTTP` module *is* available (Coffee Stain's own
server code uses it), so this plugin uses periodic REST API calls instead of a persistent
WebSocket connection to the Discord Gateway.

| Approach | WebSockets | HTTP polling (this plugin) |
|---|---|---|
| Engine dependency | `WebSockets` module (absent in CSS) | `HTTP` module ✅ (confirmed in FactoryGame.Build.cs) |
| Delivery latency | ~0 ms (push) | PollIntervalSeconds (default 5 s) |
| Works on CSS engine | ❌ | ✅ |
| Messages (`content` field) | Yes — requires Message Content Intent | Yes — requires `MESSAGE_CONTENT` bot permission |
| Server member list | Yes — requires Server Members Intent | Yes — requires `GUILD_MEMBERS` bot permission |
| Rich presence | Yes — requires Presence Intent | Best-effort (no live status via REST) |

## CSS Engine Compatibility

| Concern | Status |
|---|---|
| `HTTP` module | ✅ Core engine module, confirmed in FactoryGame.Build.cs line 45 |
| `FJsonObject` / `TryGetNumberField` | ✅ Used by SML itself |
| `UGameInstanceSubsystem` | ✅ Standard UE 4.22+ API |
| Dedicated server builds | ✅ Compiles on all target types (Game, Editor, Server) |
| `EDiscordGatewayIntent` UENUM | ✅ Kept for source compatibility; no runtime effect in HTTP mode |

## First-time Setup

### 1. Enable the plugin in Alpakit

Add the following to your mod's `.uplugin` Plugins array:

```json
{
    "Name": "DiscordBot",
    "Enabled": true
}
```

### 2. Create your Discord bot

1. Go to <https://discord.com/developers/applications> and create a new Application.
2. Navigate to **Bot** and copy your bot token.
3. Under **Bot Permissions**, enable at minimum:
   - `Read Message History` + `View Channels` (for message polling)
   - `Server Members Intent` toggle (for guild member polling)
4. Enable **Developer Mode** in Discord (User Settings → Advanced) so you can copy IDs.

### 3. Configure the bot token and IDs

Create or edit `Config/DefaultDiscordBot.ini` in your project root:

```ini
[/Script/DiscordBot.DiscordBotSubsystem]
BotToken=Bot YOUR_TOKEN_HERE
GuildId=YOUR_GUILD_SERVER_ID
ChannelId=YOUR_CHANNEL_ID
bAutoConnect=true
```

- `GuildId` — right-click your server icon → **Copy Server ID**
- `ChannelId` — right-click the channel → **Copy Channel ID**

### 4. Use from Blueprints

```
GetGameInstance → GetSubsystem (DiscordBotSubsystem) → Connect / Disconnect
```

Or bind delegates on the `DiscordGatewayClient` returned by **GetGatewayClient**:
- `OnMessageReceived(MessageContent)` — new messages in `ChannelId`
- `OnMemberUpdated(UserId)` — members from the guild member list
- `OnPresenceUpdated(UserId)` — best-effort: same members (no live status via REST)
- `OnConnected(bSuccess, ErrorMessage)` — token verification result

### 5. Use from C++

```cpp
#include "DiscordBotSubsystem.h"

UDiscordBotSubsystem* Bot = GetGameInstance()->GetSubsystem<UDiscordBotSubsystem>();

// Connect — reads GuildId/ChannelId from config automatically
Bot->Connect(TEXT("Bot YOUR_TOKEN_HERE"));

// Or set IDs at runtime before connecting
Bot->GetGatewayClient()->ChannelId = TEXT("987654321");
Bot->GetGatewayClient()->GuildId   = TEXT("123456789");
Bot->GetGatewayClient()->PollIntervalSeconds = 3.0f;
Bot->Connect(TEXT("Bot YOUR_TOKEN_HERE"));

// Listen for events
Bot->GetGatewayClient()->OnMessageReceived.AddDynamic(this, &UMyClass::HandleMessage);
```

## Rate Limits

Discord's REST API allows 5 requests per second per route (50 for global).
With the default 5-second poll interval this plugin makes at most 2 requests per cycle
(one for messages, one for members), well within limits.
Lowering `PollIntervalSeconds` below 1.0 is not recommended.

## Building with Alpakit

No special steps required. Run **Alpakit → Package** as normal.
