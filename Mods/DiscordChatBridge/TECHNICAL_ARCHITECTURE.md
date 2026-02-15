# Discord Chat Bridge - Technical Architecture

## Server-Side Only Architecture

This document explains the technical implementation that makes the Discord Chat Bridge mod **server-side only**, meaning players do **NOT** need to install this mod (or SML) to connect to servers running it.

## Code Evidence

### 1. Plugin Descriptor - RequiredOnRemote Field

**File:** `DiscordChatBridge.uplugin`  
**Line 19:**
```json
"RequiredOnRemote": false,
```

This is the **CRITICAL** field that tells SML the mod is optional on remote clients. Without this field, SML defaults to `bRequiredOnRemote = true` and will **reject clients** that don't have the mod installed, even if the code uses server-only spawning.

**SML Source Reference:**  
**File:** `Mods/SML/Source/SML/Private/ModLoading/ModLoadingLibrary.cpp`  
**Line 47:**
```cpp
Source->TryGetBoolField(TEXT("RequiredOnRemote"), bRequiredOnRemote);
```

**Line 19:** (default value when field is missing)
```cpp
this->bRequiredOnRemote = true;
```

### 2. Replication Policy

### 2. Replication Policy

**File:** `Source/DiscordChatBridge/Private/DiscordChatSubsystem.cpp`  
**Line 15:**
```cpp
ReplicationPolicy = ESubsystemReplicationPolicy::SpawnOnServer;
```

This explicitly sets the subsystem to spawn **only on the server** with **no replication** to clients.

### 3. Replication Policy Definition

### 3. Replication Policy Definition

**File:** `Mods/SML/Source/SML/Public/Subsystem/ModSubsystem.h`  
**Lines 9-10:**
```cpp
/** Subsystem will be spawned on server and will not be replicated to remote clients */
SpawnOnServer UMETA(DisplayName = "Spawn on Server, Do Not Replicate"),
```

The SML framework defines `SpawnOnServer` policy as:
- ✅ Spawns on server
- ❌ Does NOT spawn on clients
- ❌ Does NOT replicate to clients

### 4. Module Type

### 4. Module Type

**File:** `DiscordChatBridge.uplugin`  
**Lines 21-24:**
```json
"Modules": [
    {
        "Name": "DiscordChatBridge",
        "Type": "Runtime",
        "LoadingPhase": "PostDefault"
    }
]
```

The module is marked as `Runtime` type, which means it loads during normal game runtime, but the replication policy controls where it spawns.

## The Two-Part Solution

For a mod to be truly server-side only in SML, **BOTH** of these must be configured:

### Part 1: Plugin Descriptor (SML Network Layer)
```json
"RequiredOnRemote": false
```
This tells SML's network manager that clients can join without having the mod. Without this, SML will reject clients during the connection handshake.

### Part 2: Code Replication Policy (Unreal Spawning)
```cpp
ReplicationPolicy = ESubsystemReplicationPolicy::SpawnOnServer;
```
This tells Unreal/SML where to spawn the subsystem actor. With `SpawnOnServer`, it only spawns on the server.

**Both are required!** Without `RequiredOnRemote: false`, clients will be blocked by SML before the game even attempts to spawn the subsystem.

## How It Works

### Server Behavior
1. Server loads SML (Satisfactory Mod Loader)
2. SML spawns `ADiscordChatSubsystem` (due to `SpawnOnServer` policy)
3. Subsystem initializes Discord API connection
4. Subsystem hooks into game's chat manager (`AFGChatManager`)
5. When players send chat messages, subsystem forwards them to Discord
6. When Discord messages arrive, subsystem injects them into game chat

### Client Behavior
1. Client connects to server (no SML or mod required)
2. Client sees chat messages through normal Satisfactory chat replication
3. Client is unaware of Discord integration
4. From client perspective, "[Discord] Username" messages look like any other game message

## Chat Message Flow

### Game → Discord
```
Player types in-game
    ↓
AFGChatManager receives message (replicated to all clients by game)
    ↓
[SERVER ONLY] DiscordChatSubsystem::OnGameChatMessageAdded() called
    ↓
[SERVER ONLY] DiscordAPI::SendMessage() sends HTTP request to Discord
    ↓
Message appears in Discord channel
```

### Discord → Game
```
User types in Discord channel
    ↓
[SERVER ONLY] DiscordAPI polls Discord REST API
    ↓
[SERVER ONLY] DiscordChatSubsystem::OnDiscordMessageReceived() called
    ↓
[SERVER ONLY] Creates FChatMessageStruct with username formatted as "[Discord] Username"
    ↓
[SERVER ONLY] AFGChatManager->AddChatMessageToHistory()
    ↓
Game's built-in chat replication sends message to all clients
    ↓
All players see message (including vanilla clients)
```

## Why Clients Don't Need The Mod

### The Previous Problem (Before Adding RequiredOnRemote: false)

Even though the code used `SpawnOnServer` replication policy, clients were still being **rejected** by SML during connection. Here's why:

**Connection Flow:**
1. Client attempts to connect to server
2. **SML Network Manager runs version/mod check** (happens BEFORE Unreal spawns any actors)
3. Server checks: Does client have all mods marked with `RequiredOnRemote: true`?
4. If Discord Chat Bridge is missing from client: **CONNECTION REJECTED**
5. Client never gets to the game, subsystem never attempts to spawn

The `SpawnOnServer` policy only controls **where Unreal spawns the actor** (step that never happened because connection was rejected at step 4).

### The Solution

Adding `"RequiredOnRemote": false` to the plugin descriptor fixes this:

**New Connection Flow:**
1. Client attempts to connect to server
2. SML Network Manager runs version/mod check
3. Server sees `RequiredOnRemote: false` for Discord Chat Bridge
4. **Connection ACCEPTED** (mod is optional on client)
5. Client joins the game successfully
6. Server spawns `ADiscordChatSubsystem` (due to `SpawnOnServer` policy)
7. Client does NOT spawn subsystem (as expected)
8. Chat works normally using game's built-in replication

### 1. No Client-Side Code Execution
The `SpawnOnServer` policy ensures the subsystem never instantiates on clients. Clients never execute any Discord Chat Bridge code.

### 2. Uses Native Game Chat System
The mod doesn't create a custom chat UI or networking. It piggybacks on Satisfactory's existing `AFGChatManager`, which already replicates messages to all clients (vanilla or modded).

### 3. Server-Only Operations
All Discord operations happen server-side:
- Discord API authentication (bot token)
- HTTP requests to Discord
- Discord message polling
- WebSocket Gateway connection (for presence)

Clients never need Discord credentials or network access to Discord.

## Comparison with Other Mod Types

### Server-Side Only (This Mod)
```
ReplicationPolicy = ESubsystemReplicationPolicy::SpawnOnServer;
```
- ✅ Server spawns subsystem
- ❌ Clients do NOT spawn subsystem
- ✅ Clients can be vanilla (no SML/mods needed)
- **Example:** Discord Chat Bridge

### Server-Side with Replication
```
ReplicationPolicy = ESubsystemReplicationPolicy::SpawnOnServer_Replicate;
```
- ✅ Server spawns subsystem
- ✅ Clients receive replicated state
- ❌ Clients still need the mod (to understand replicated data)
- **Example:** Server-managed gameplay systems with client UI

### Client-Side Only
```
ReplicationPolicy = ESubsystemReplicationPolicy::SpawnOnClient;
```
- ❌ Server does NOT spawn subsystem
- ✅ Clients spawn subsystem
- ✅ Clients MUST have the mod
- **Example:** Client-only UI enhancements

### Both Server and Client
```
ReplicationPolicy = ESubsystemReplicationPolicy::SpawnLocal;
```
- ✅ Server spawns subsystem
- ✅ Clients spawn subsystem
- ✅ Clients MUST have the mod
- **Example:** Mods that add new items/buildings

## Common Misconceptions

### "Players need SML to join modded servers"
**FALSE** for this mod. Players can join with vanilla Satisfactory clients.

SML is only required for:
- Mods that add content clients need to see (items, buildings, textures)
- Mods that use client-side policies (SpawnOnClient, SpawnLocal)
- Mods that modify client-side game behavior

### "If the server has mods, clients need mods"
**FALSE** - depends on the mod's replication policy. Server-side only mods (like this one) don't require clients to have anything.

### "OAuth means clients need Discord setup"
**FALSE** - OAuth is used by the server administrator to create the Discord bot. Players never interact with Discord's developer portal or OAuth flows. They just play the game normally.

## Testing Confirmation

You can test this yourself:

1. **Server Setup:**
   - Install Satisfactory dedicated server
   - Install SML on server
   - Install Discord Chat Bridge mod on server
   - Configure Discord bot token

2. **Client Setup:**
   - Install vanilla Satisfactory (no SML, no mods)
   - Connect to server using server IP

3. **Expected Result:**
   - ✅ Client connects successfully
   - ✅ Client can see all chat messages including Discord messages
   - ✅ Client can send chat messages that appear in Discord
   - ✅ No errors or version mismatches

## Conclusion

The Discord Chat Bridge mod is **definitively server-side only** by design and implementation, **but required a configuration fix** to work properly.

### The Fix

**Added to DiscordChatBridge.uplugin:**
```json
"RequiredOnRemote": false,
```

This critical field was missing, causing SML to default to `bRequiredOnRemote = true`, which blocked clients from connecting even though the code was correctly using `SpawnOnServer` policy.

### How to Verify

Players joining a server with this mod (after the fix):
- ✅ Do NOT need to install SML
- ✅ Do NOT need to install this mod
- ✅ Do NOT need any Discord configuration
- ✅ Can use vanilla Satisfactory client
- ✅ Will see and can participate in Discord chat automatically
- ✅ **WILL NOT** be rejected by SML's network manager

### Testing

You can test this yourself:

1. **Server Setup:**
   - Install Satisfactory dedicated server
   - Install SML on server
   - Install Discord Chat Bridge mod on server (with `RequiredOnRemote: false`)
   - Configure Discord bot token

2. **Client Setup:**
   - Install vanilla Satisfactory (no SML, no mods)
   - Connect to server using server IP

3. **Expected Result:**
   - ✅ Client connects successfully (no version mismatch or mod required errors)
   - ✅ Client can see all chat messages including Discord messages
   - ✅ Client can send chat messages that appear in Discord
   - ✅ No errors or version mismatches

### If Issues Persist

If players are still experiencing connection issues after this fix, it's likely due to:
- **Other mods** on the server that DO require client installation (check their `RequiredOnRemote` settings)
- **Cached mod list** - server may need restart after adding the field
- **SML version mismatch** - ensure server has SML 3.11.3+
- **Different mod** entirely - verify the Discord Chat Bridge specifically

---

**For questions or issues, please check:**
- [README.md](README.md) - General documentation
- [FAQ Section](README.md#frequently-asked-questions-faq) - Common questions
- [SETUP_GUIDE.md](SETUP_GUIDE.md) - Installation instructions
