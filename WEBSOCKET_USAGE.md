# WebSocket Support in SatisfactoryModLoader

## Overview

WebSocket support has been added to the SatisfactoryModLoader project to enable real-time bidirectional communication between the game server and external applications like Discord bots.

## Changes Made

1. **Added WebSockets module** to `Source/FactoryDedicatedServer/FactoryDedicatedServer.Build.cs`
2. **Added WebSockets module** to `Source/FactoryGame/FactoryGame.Build.cs`
3. **WebSockets are enabled** in `Config/DefaultEngine.ini` with `bEnableWebsockets=true`

## Usage in C++

### Creating a WebSocket Connection

```cpp
#include "IWebSocket.h"
#include "WebSocketsModule.h"

// Create a WebSocket connection
TSharedPtr<IWebSocket> WebSocket = FWebSocketsModule::Get().CreateWebSocket(TEXT("ws://localhost:8080"));

// Set up event handlers
WebSocket->OnConnected().AddLambda([]()
{
    UE_LOG(LogTemp, Log, TEXT("WebSocket Connected"));
});

WebSocket->OnConnectionError().AddLambda([](const FString& Error)
{
    UE_LOG(LogTemp, Error, TEXT("WebSocket Connection Error: %s"), *Error);
});

WebSocket->OnClosed().AddLambda([](int32 StatusCode, const FString& Reason, bool bWasClean)
{
    UE_LOG(LogTemp, Log, TEXT("WebSocket Closed: %d - %s"), StatusCode, *Reason);
});

WebSocket->OnMessage().AddLambda([](const FString& Message)
{
    UE_LOG(LogTemp, Log, TEXT("WebSocket Message Received: %s"), *Message);
});

WebSocket->OnRawMessage().AddLambda([](const void* Data, SIZE_T Size, SIZE_T BytesRemaining)
{
    UE_LOG(LogTemp, Log, TEXT("WebSocket Binary Message Received: %d bytes"), Size);
});

// Connect to the server
WebSocket->Connect();
```

### Sending Messages

```cpp
// Send text message
WebSocket->Send(TEXT("Hello from Satisfactory!"));

// Send JSON message
FString JsonMessage = TEXT("{\"type\":\"status\",\"data\":\"server_ready\"}");
WebSocket->Send(JsonMessage);

// Send binary data
TArray<uint8> BinaryData;
// ... populate BinaryData
WebSocket->Send(BinaryData.GetData(), BinaryData.Num(), true);
```

### Closing the Connection

```cpp
WebSocket->Close(1000, TEXT("Normal closure"));
```

## Discord Bot Integration

With WebSocket support enabled, you can now:

1. **Create a WebSocket server in your Discord bot** (using libraries like `ws` for Node.js or `websockets` for Python)
2. **Connect from the game server** to the Discord bot using the code above
3. **Send game events** (player joins, achievements, etc.) to Discord in real-time
4. **Receive commands** from Discord to control the game server

### Example Discord Bot (Node.js)

```javascript
const WebSocket = require('ws');

const wss = new WebSocket.Server({ port: 8080 });

wss.on('connection', (ws) => {
    console.log('Satisfactory server connected');
    
    ws.on('message', (message) => {
        console.log('Received:', message);
        const data = JSON.parse(message);
        
        // Handle game events
        if (data.type === 'player_join') {
            // Send notification to Discord channel
            discordChannel.send(`Player ${data.playerName} joined the server!`);
        }
    });
    
    ws.send(JSON.stringify({ type: 'status', message: 'Connected to Discord bot' }));
});
```

## Server API Integration

The existing HTTP Server API in `Source/FactoryDedicatedServer/Public/Networking/FGServerAPIManager.h` can be extended to:

1. Accept WebSocket upgrade requests
2. Maintain persistent WebSocket connections for real-time updates
3. Broadcast server events to connected WebSocket clients

## Notes

- WebSocket support is provided by Unreal Engine's built-in WebSockets module
- The module supports both secure (WSS) and unsecure (WS) connections
- For production use, consider using secure WebSocket connections (WSS) with proper certificates
- The existing SSL/TLS infrastructure in `Source/FactoryDedicatedServer/Public/Security/` can be leveraged for secure WebSocket connections

## Troubleshooting

If you encounter issues:

1. **Module not found**: Ensure your Unreal Engine installation includes the WebSockets module
2. **Connection failures**: Check firewall settings and ensure the WebSocket server is running
3. **SSL/TLS errors**: Verify certificates are properly configured for WSS connections

## References

- Unreal Engine WebSockets Documentation: https://docs.unrealengine.com/5.3/en-US/API/Runtime/WebSockets/
- WebSocket Protocol (RFC 6455): https://tools.ietf.org/html/rfc6455
- Discord.js Guide: https://discordjs.guide/
