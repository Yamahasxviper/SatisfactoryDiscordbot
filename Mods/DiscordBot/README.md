# DiscordBot — SML Plugin

Integrates a Discord bot into Satisfactory by polling the **Discord REST API** via UE's
built-in `HTTP` module — no WebSocket module required.

## All Three Privileged Intents — Without WebSockets

**Yes, all three intents are fully covered** using the Discord REST API v10.
No `WebSockets` module is needed.

| Intent | REST endpoint used | Delegate fired |
|---|---|---|
| **Message Content** | `GET /channels/{id}/messages?after=…&limit=100` | `OnMessageReceived(Content, AuthorId)` |
| **Server Members** | `GET /guilds/{id}/members?limit=1000` | `OnMemberUpdated(UserId)` |
| **Presence** (per-user) | Message author extracted from each message | `OnPresenceUpdated(AuthorId)` |
| **Presence** (aggregate) | `GET /guilds/{id}?with_counts=true` | `OnPresenceCount(OnlineCount, MemberCount)` |

### 1. Message Content Intent
Every poll cycle calls `GET /channels/{ChannelId}/messages?after={lastId}&limit=100`.
Each message object in the response contains the full `content` field.
`OnMessageReceived(MessageContent, AuthorId)` fires for every new message.

### 2. Server Members Intent
Every poll cycle calls `GET /guilds/{GuildId}/members?limit=1000`.
This endpoint **requires** the `GUILD_MEMBERS` privileged intent toggle in the
Discord Developer Portal (Bot → Privileged Gateway Intents).
`OnMemberUpdated(UserId)` fires for each guild member.

### 3. Presence Intent — two REST signals

**Per-user:** When a user sends a message they are demonstrably online.
`OnMessagesResponse` extracts `author.id` from every new message and fires
`OnPresenceUpdated(AuthorId)` for that user.

**Aggregate:** `GET /guilds/{GuildId}?with_counts=true` returns the total
online count for the server.
`OnPresenceCount(OnlineCount, MemberCount)` fires each poll cycle.
`GatewayClient->ApproximatePresenceCount` and `GatewayClient->ApproximateMemberCount`
are also readable directly as Blueprint-accessible properties.

## Why HTTP polling instead of WebSockets?

Satisfactory runs on a **custom Unreal Engine build (5.3.2-CSS)** that does not include the
engine's optional `WebSockets` module.  The `HTTP` module *is* available (Coffee Stain's own
server code uses it), so this plugin uses periodic REST API calls instead of a persistent
WebSocket connection to the Discord Gateway.

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
- `OnMessageReceived(MessageContent, AuthorId)` — new messages in `ChannelId` (**Message Content Intent**)
- `OnMemberUpdated(UserId)` — each guild member from the member list (**Server Members Intent**)
- `OnPresenceUpdated(AuthorId)` — user who posted a message (is demonstrably online) (**Presence Intent**, per-user)
- `OnPresenceCount(OnlineCount, MemberCount)` — total online members in the guild (**Presence Intent**, aggregate)
- `OnConnected(bSuccess, ErrorMessage)` — token verification result

### 5. Use from C++

```cpp
#include "DiscordBotSubsystem.h"

UDiscordBotSubsystem* Bot = GetGameInstance()->GetSubsystem<UDiscordBotSubsystem>();

// Connect — reads GuildId/ChannelId from config automatically
Bot->Connect(TEXT("Bot YOUR_TOKEN_HERE"));

// Or set IDs at runtime before connecting
Bot->GetGatewayClient()->ChannelId          = TEXT("987654321");
Bot->GetGatewayClient()->GuildId            = TEXT("123456789");
Bot->GetGatewayClient()->PollIntervalSeconds = 3.0f;
Bot->Connect(TEXT("Bot YOUR_TOKEN_HERE"));

// Message Content Intent: content + author ID of each new message
Bot->GetGatewayClient()->OnMessageReceived.AddDynamic(this, &UMyClass::HandleMessage);
// Signature: void HandleMessage(const FString& Content, const FString& AuthorId)

// Server Members Intent: each guild member
Bot->GetGatewayClient()->OnMemberUpdated.AddDynamic(this, &UMyClass::HandleMember);
// Signature: void HandleMember(const FString& UserId)

// Presence Intent (per-user): users who sent a message (demonstrably online)
Bot->GetGatewayClient()->OnPresenceUpdated.AddDynamic(this, &UMyClass::HandlePresence);
// Signature: void HandlePresence(const FString& UserId)

// Presence Intent (aggregate): total online count for the guild
Bot->GetGatewayClient()->OnPresenceCount.AddDynamic(this, &UMyClass::HandlePresenceCount);
// Signature: void HandlePresenceCount(int32 OnlineCount, int32 MemberCount)

// Or read the cached values directly:
int32 Online  = Bot->GetGatewayClient()->ApproximatePresenceCount;
int32 Members = Bot->GetGatewayClient()->ApproximateMemberCount;
```

## Rate Limits

Discord's REST API allows 5 requests per second per route (50 for global).
With the default 5-second poll interval this plugin makes at most 3 requests per cycle
(one for messages, one for members), well within limits.
Lowering `PollIntervalSeconds` below 1.0 is not recommended.

## Building with Alpakit

No special steps required. Run **Alpakit → Package** as normal.
