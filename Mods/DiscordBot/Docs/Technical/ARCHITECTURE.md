# Two-Way Chat System Architecture

This document provides a visual overview of the two-way chat system architecture.

## System Components

```
┌─────────────────────────────────────────────────────────────────┐
│                    Satisfactory Game Server                      │
│                                                                  │
│  ┌────────────────────────────────────────────────────────────┐ │
│  │                  Game Instance Subsystem                    │ │
│  │                                                              │ │
│  │  ┌──────────────────────────────────────────────────────┐  │ │
│  │  │         UDiscordBotSubsystem (Manager)              │  │ │
│  │  │                                                      │  │ │
│  │  │  • Manages Discord connection                       │  │ │
│  │  │  • Coordinates message relay                        │  │ │
│  │  │  • Loads configuration                              │  │ │
│  │  │  • Formats sender names                             │  │ │
│  │  │                                                      │  │ │
│  │  │  Methods:                                            │  │ │
│  │  │  - OnDiscordMessageReceived()                       │  │ │
│  │  │  - OnGameChatMessage()                              │  │ │
│  │  │  - LoadTwoWayChatConfig()                           │  │ │
│  │  │                                                      │  │ │
│  │  └───────────┬──────────────────────────┬──────────────┘  │ │
│  │              │                          │                  │ │
│  │              │                          │                  │ │
│  │  ┌───────────▼──────────┐   ┌──────────▼───────────────┐  │ │
│  │  │ UDiscordChatRelay   │   │ ADiscordGateway          │  │ │
│  │  │ (Game→Discord)       │   │ ClientCustom             │  │ │
│  │  │                      │   │ (Discord→Game)           │  │ │
│  │  │ • Hooks FGChatMgr   │   │                          │  │ │
│  │  │ • Filters player    │   │ • WebSocket to Discord   │  │ │
│  │  │   messages          │   │ • Handles MESSAGE_CREATE │  │ │
│  │  │ • Forwards to       │   │ • Filters bot messages   │  │ │
│  │  │   subsystem         │   │ • HTTP API for sending   │  │ │
│  │  │                      │   │                          │  │ │
│  │  └───────────┬──────────┘   └──────────┬───────────────┘  │ │
│  │              │                          │                  │ │
│  └──────────────┼──────────────────────────┼──────────────────┘ │
│                 │                          │                    │
│  ┌──────────────▼──────────┐   ┌──────────▼───────────────┐   │
│  │   AFGChatManager        │   │   Config/DiscordBot.ini  │   │
│  │   (Satisfactory)        │   │                          │   │
│  │                         │   │ • Bot token              │   │
│  │ • In-game chat system   │   │ • Channel IDs            │   │
│  │ • Broadcasts messages   │   │ • Sender formats         │   │
│  │ • OnChatMessageAdded    │   │ • Feature toggle         │   │
│  │   event                 │   │                          │   │
│  └─────────────────────────┘   └──────────────────────────┘   │
│                                                                 │
└─────────────────────────────────────────────────────────────────┘
                              │
                              │ WSS
                              │
                              ▼
                    ┌─────────────────────┐
                    │  Discord Gateway    │
                    │  (WebSocket)        │
                    │                     │
                    │  • MESSAGE_CREATE   │
                    │  • HEARTBEAT        │
                    │  • IDENTIFY         │
                    └──────────┬──────────┘
                              │
                              │ HTTPS
                              │
                              ▼
                    ┌─────────────────────┐
                    │  Discord HTTP API   │
                    │                     │
                    │  • POST /messages   │
                    └─────────────────────┘
                              │
                              │
                              ▼
                    ┌─────────────────────┐
                    │  Discord Channel(s) │
                    │                     │
                    │  Channel 1          │
                    │  Channel 2          │
                    │  Channel N          │
                    └─────────────────────┘
```

## Message Flow: Discord → Game

```
1. Discord User              → Sends message in Discord channel
                               "Hello from Discord!"
                               │
                               ▼
2. Discord Gateway           → Sends MESSAGE_CREATE event via WebSocket
                               {
                                 "op": 0,
                                 "t": "MESSAGE_CREATE",
                                 "d": {
                                   "channel_id": "123456",
                                   "content": "Hello from Discord!",
                                   "author": {
                                     "id": "789",
                                     "username": "JohnDoe",
                                     "bot": false
                                   }
                                 }
                               }
                               │
                               ▼
3. ADiscordGatewayClientCustom → Receives & processes event
   HandleMessageCreate()         • Check if bot message → NO
                               • Check if empty → NO
                               • Extract: channel_id, username, content
                               │
                               ▼
4. UDiscordBotSubsystem       → Validates & formats
   OnDiscordMessageReceived()   • Check channel in config → YES
                               • Format: "[Discord] JohnDoe"
                               │
                               ▼
5. AFGChatManager             → Broadcasts to game
   BroadcastChatMessage()       Creates FChatMessageStruct:
                               • MessageType: CMT_CustomMessage
                               • MessageSender: "[Discord] JohnDoe"
                               • MessageText: "Hello from Discord!"
                               • MessageSenderColor: Light Blue
                               │
                               ▼
6. In-Game Chat Window        → Displays message
                               [Discord] JohnDoe: Hello from Discord!
                               (in light blue color)
```

## Message Flow: Game → Discord

```
1. Player in Game            → Types message in-game
                               "Hello from Satisfactory!"
                               │
                               ▼
2. AFGChatManager            → Processes player message
   BroadcastChatMessage()      Creates FChatMessageStruct:
                               • MessageType: CMT_PlayerMessage
                               • MessageSender: "PlayerOne"
                               • MessageText: "Hello from Satisfactory!"
                               │
                               ▼
3. AFGChatManager            → Fires event
   OnChatMessageAdded.Broadcast()
                               │
                               ▼
4. UDiscordChatRelay         → Intercepts event
   OnChatMessageAdded()        • Check message type → PlayerMessage ✓
                               • Extract: sender, message
                               │
                               ▼
5. UDiscordBotSubsystem      → Formats & sends
   OnGameChatMessage()         • Format: "**PlayerOne**: Hello from Satisfactory!"
                               • For each configured channel:
                                 └─> SendDiscordMessage()
                               │
                               ▼
6. ADiscordGatewayClientCustom → Sends via HTTP API
   SendMessageHTTP()            POST https://discord.com/api/v10/channels/{id}/messages
                               Authorization: Bot {token}
                               Body: {"content": "**PlayerOne**: Hello from Satisfactory!"}
                               │
                               ▼
7. Discord API               → Delivers to channel
                               │
                               ▼
8. Discord Channel(s)        → Displays message
                               **PlayerOne**: Hello from Satisfactory!
```

## Configuration Flow

```
Server Start
     │
     ▼
UDiscordBotSubsystem::Initialize()
     │
     ├─> LoadTwoWayChatConfig()
     │   │
     │   ├─> Read Config/DiscordBot.ini
     │   │   │
     │   │   ├─> bEnableTwoWayChat = true
     │   │   ├─> ChatChannelId[] = [123, 456, 789]
     │   │   ├─> DiscordSenderFormat = "[Discord] {username}"
     │   │   └─> GameSenderFormat = "{playername}"
     │   │
     │   └─> Store in member variables
     │
     ├─> InitializeAndConnect(BotToken)
     │   │
     │   └─> Spawn ADiscordGatewayClientCustom
     │       └─> Connect to Discord Gateway
     │
     └─> if (bEnableTwoWayChat)
         │
         └─> Create & Initialize UDiscordChatRelay
             └─> Bind to AFGChatManager::OnChatMessageAdded
```

## Security Layers

```
Discord → Game Security:
┌─────────────────────────────┐
│ 1. Bot Message Filter       │ ← Checks 'bot' field & bot user ID
├─────────────────────────────┤
│ 2. Empty Message Filter     │ ← Rejects empty content
├─────────────────────────────┤
│ 3. Channel Whitelist        │ ← Only configured channels
├─────────────────────────────┤
│ 4. JSON Validation          │ ← HasField checks before access
├─────────────────────────────┤
│ 5. Safe String Operations   │ ← FString::Replace (no buffer overflow)
└─────────────────────────────┘

Game → Discord Security:
┌─────────────────────────────┐
│ 1. Message Type Filter      │ ← Only CMT_PlayerMessage
├─────────────────────────────┤
│ 2. Bot Connection Check     │ ← Must be connected
├─────────────────────────────┤
│ 3. Channel Validation       │ ← Only to configured channels
├─────────────────────────────┤
│ 4. Format String Safety     │ ← Replace, not sprintf
└─────────────────────────────┘
```

## Data Structures

### Configuration (Runtime)
```cpp
class UDiscordBotSubsystem {
    bool bTwoWayChatEnabled;                // Feature toggle
    TArray<FString> ChatChannelIds;         // ["123", "456", "789"]
    FString DiscordSenderFormat;            // "[Discord] {username}"
    FString GameSenderFormat;               // "{playername}"
    UDiscordChatRelay* ChatRelay;           // Game chat hook
    ADiscordGatewayClient* GatewayClient;   // Discord connection
};
```

### Discord Message (Incoming)
```json
{
  "channel_id": "123456789",
  "content": "Hello!",
  "author": {
    "id": "987654321",
    "username": "JohnDoe",
    "bot": false
  }
}
```

### Game Message (Outgoing to Discord)
```json
{
  "content": "**PlayerOne**: Hello from game!"
}
```

### In-Game Chat Message
```cpp
struct FChatMessageStruct {
    FText MessageText;                      // "Hello!"
    EFGChatMessageType MessageType;         // CMT_CustomMessage
    FText MessageSender;                    // "[Discord] JohnDoe"
    FLinearColor MessageSenderColor;        // (0.4, 0.6, 1.0) - Light Blue
    float ServerTimeStamp;                  // Auto-set
    bool bIsLocalPlayerMessage;             // Auto-set
};
```

## Performance Characteristics

### Discord → Game
- **Latency**: WebSocket (real-time) + in-game broadcast
- **Overhead**: < 1ms per message (JSON parsing + formatting)
- **Scalability**: O(1) per message

### Game → Discord
- **Latency**: Event capture + HTTP POST
- **Overhead**: < 1ms per message (formatting + HTTP async)
- **Scalability**: O(n) where n = number of channels

### Memory Usage
- **Static**: ~1KB for subsystem + relay
- **Per Message**: ~500 bytes temporary (JSON buffers)
- **Connection**: ~10KB WebSocket buffer

## Error Handling

```
┌─────────────────────────────────────────────┐
│           Error Scenarios                    │
├─────────────────────────────────────────────┤
│ 1. ChatManager Not Found                    │
│    → Log warning, don't initialize relay    │
├─────────────────────────────────────────────┤
│ 2. Discord Connection Lost                  │
│    → Log error, stop relaying to Discord    │
├─────────────────────────────────────────────┤
│ 3. Invalid Channel ID                       │
│    → Skip that channel, continue others     │
├─────────────────────────────────────────────┤
│ 4. HTTP Request Failed                      │
│    → Log error, message not sent            │
├─────────────────────────────────────────────┤
│ 5. Malformed JSON                           │
│    → Log error, skip message                │
└─────────────────────────────────────────────┘
```

## Key Design Decisions

1. **Subsystem Pattern**: Used GameInstanceSubsystem for lifecycle management
2. **Delegate Binding**: Used Unreal's delegate system for loose coupling
3. **Async HTTP**: Non-blocking message sending
4. **Whitelist Approach**: Only configured channels (security)
5. **Format Strings**: Simple placeholder replacement (extensible)
6. **Color Coding**: Visual distinction for Discord messages
7. **Type Filtering**: Only player messages relay (prevents clutter)
8. **Bot Filtering**: Prevents infinite loops

## Future Enhancement Opportunities

1. **Rich Text**: Support Discord markdown → Unreal rich text
2. **Embeds**: Send embeds instead of plain text
3. **User Mentions**: Discord @mentions → in-game highlights
4. **Rate Limiting**: Throttle messages per channel
5. **Message History**: Sync recent messages on connect
6. **Edit/Delete**: Sync message modifications
7. **Threads**: Support Discord thread channels
8. **Per-Channel Config**: Different formats per channel

---

This architecture provides a solid foundation for two-way communication while maintaining security, performance, and maintainability.
