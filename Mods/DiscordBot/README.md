# DiscordBot — SML Plugin

Integrates a Discord Gateway bot into Satisfactory using Unreal Engine's built-in **WebSockets** module.

## Custom Engine Compatibility (5.3.2-CSS)

This plugin is written to work with **Coffee Stain Studios' custom Unreal Engine (5.3.2-CSS)**.

| Concern | Status |
|---|---|
| `WebSockets` module | ✅ Core engine module in all UE 5.x builds — declared in `Build.cs`, no plugin enablement required |
| `HTTP` / `Sockets` modules | ✅ Already linked by `FactoryGame`; not required directly |
| `OpenSSL` / `SSL` for `wss://` | ✅ Handled internally by UE's WebSockets module (same as standard UE5) |
| `FJsonObject` / `TryGetNumberField` | ✅ Used by SML itself (see `SaveMetadataPatch.cpp`) |
| `UGameInstanceSubsystem` | ✅ Standard UE 4.22+ API, fully available in 5.3.2-CSS |
| Dedicated server builds | ✅ Compiles and runs on all target types (Game, Editor, Server) |
| `EDiscordGatewayIntent` UENUM | ✅ Uses explicit integer literals (UHT-safe, no bit-shift expressions) |

## First-time Setup

### 1. Enable the plugin in Alpakit

Open the Unreal Editor, go to **Edit → Plugins**, search for *Discord Bot*, and enable it.
Alternatively, add the following to your mod's `.uplugin` Plugins array:

```json
{
    "Name": "DiscordBot",
    "Enabled": true
}
```

### 2. Create your Discord bot

1. Go to <https://discord.com/developers/applications> and create a new Application.
2. Navigate to **Bot** and copy your bot token.
3. Under **Privileged Gateway Intents**, enable all three required intents:
   - **Presence Intent** (bit 8 = 256)
   - **Server Members Intent** (bit 1 = 2)
   - **Message Content Intent** (bit 15 = 32768)

> ⚠️ Without these being enabled in the Developer Portal, Discord will reject the IDENTIFY payload even if the intent bits are set correctly.

### 3. Configure the bot token

Create or edit `Config/DefaultDiscordBot.ini` in your project root:

```ini
[/Script/DiscordBot.DiscordBotSubsystem]
BotToken=Bot YOUR_TOKEN_HERE
GuildId=YOUR_GUILD_ID
bAutoConnect=true
```

Set `bAutoConnect=true` to connect automatically when the game/server starts.

### 4. Use from Blueprints

```
GetGameInstance → GetSubsystem (DiscordBotSubsystem) → Connect / Disconnect
```

Or bind the `OnMessageReceived`, `OnPresenceUpdated`, and `OnMemberUpdated` delegates on the `DiscordGatewayClient` returned by **GetGatewayClient**.

### 5. Use from C++

```cpp
#include "DiscordBotSubsystem.h"

// Get the subsystem
UDiscordBotSubsystem* BotSubsystem = GetGameInstance()->GetSubsystem<UDiscordBotSubsystem>();

// Connect with the three privileged intents (GuildMembers|GuildPresences|MessageContent = 33026)
BotSubsystem->Connect(TEXT("Bot YOUR_TOKEN_HERE"));

// Listen for events
BotSubsystem->GetGatewayClient()->OnMessageReceived.AddDynamic(this, &UMyClass::HandleMessage);
```

## Intent Reference

| Intent Name | Bit | Value | Privileged |
|---|---|---|---|
| GuildMembers (**Server Members**) | 1 | 2 | ✅ Yes |
| GuildPresences (**Presence**) | 8 | 256 | ✅ Yes |
| MessageContent (**Message Content**) | 15 | 32768 | ✅ Yes |
| Combined bitmask | — | **33026** | — |

## Building with Alpakit

This plugin follows the same Alpakit package conventions as SML and ExampleMod.
Run **Alpakit → Package** as normal — no special steps are required.
