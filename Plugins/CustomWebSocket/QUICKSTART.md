# Custom WebSocket Plugin - Quick Start

Get started with the Custom WebSocket plugin in 5 minutes!

## Installation

### Option 1: Copy Plugin to Your Project

1. Copy the `CustomWebSocket` folder to your project's `Plugins` directory:
   ```
   YourProject/
   └── Plugins/
       └── CustomWebSocket/
   ```

2. Regenerate project files (right-click .uproject → Generate Visual Studio project files)

3. Build your project

### Option 2: Use in Satisfactory Mod Project

The plugin is already included! Just add it to your mod's dependencies:

**In your mod's .uplugin:**
```json
"Plugins": [
    {
        "Name": "CustomWebSocket",
        "Enabled": true
    }
]
```

**In your mod's Build.cs:**
```csharp
PublicDependencyModuleNames.AddRange(new string[] {
    "CustomWebSocket"
});
```

## First WebSocket Connection

### 1. Create a Simple Test

```cpp
#include "CustomWebSocket.h"

void UMyClass::TestWebSocket()
{
    // Create WebSocket
    TSharedPtr<FCustomWebSocket> WS = MakeShared<FCustomWebSocket>();

    // Handle connection
    WS->OnConnected.BindLambda([WS](bool bSuccess) {
        if (bSuccess) {
            UE_LOG(LogTemp, Log, TEXT("✅ Connected!"));
            WS->SendText(TEXT("Hello WebSocket!"));
        }
    });

    // Handle messages
    WS->OnMessage.BindLambda([](const FString& Msg, bool bIsText) {
        UE_LOG(LogTemp, Log, TEXT("📨 Received: %s"), *Msg);
    });

    // Connect to echo server
    WS->Connect(TEXT("wss://echo.websocket.org"));
}
```

### 2. Add Tick Function

WebSocket needs to be ticked for async operations:

```cpp
// In your Actor's Tick
void AMyActor::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    if (WebSocket.IsValid())
    {
        WebSocket->Tick(DeltaTime);
    }
}
```

### 3. Test It!

Run your project and you should see:
```
LogTemp: ✅ Connected!
LogTemp: 📨 Received: Hello WebSocket!
```

## Common Use Cases

### Discord Bot

```cpp
TSharedPtr<FCustomWebSocket> DiscordWS = MakeShared<FCustomWebSocket>();

DiscordWS->OnConnected.BindLambda([DiscordWS](bool bSuccess) {
    if (bSuccess) {
        // Send IDENTIFY with bot token
        FString IdentifyPayload = TEXT("{\"op\":2,\"d\":{\"token\":\"YOUR_TOKEN\",\"intents\":33026}}");
        DiscordWS->SendText(IdentifyPayload);
    }
});

DiscordWS->Connect(TEXT("wss://gateway.discord.gg/?v=10&encoding=json"));
```

### Chat Application

```cpp
TSharedPtr<FCustomWebSocket> ChatWS = MakeShared<FCustomWebSocket>();

ChatWS->OnMessage.BindLambda([](const FString& Message, bool bIsText) {
    // Parse and display chat message
    UE_LOG(LogTemp, Log, TEXT("Chat: %s"), *Message);
});

ChatWS->Connect(TEXT("wss://your-chat-server.com/chat"));
```

### Real-Time Data

```cpp
TSharedPtr<FCustomWebSocket> DataWS = MakeShared<FCustomWebSocket>();

DataWS->OnMessage.BindLambda([](const FString& Data, bool bIsText) {
    // Parse JSON data
    TSharedPtr<FJsonObject> JsonObject;
    TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Data);
    
    if (FJsonSerializer::Deserialize(Reader, JsonObject)) {
        // Use the data
    }
});

DataWS->Connect(TEXT("wss://api.example.com/stream"));
```

## Best Practices

### 1. Always Check Connection Status

```cpp
if (WebSocket->IsConnected()) {
    WebSocket->SendText(TEXT("Message"));
} else {
    UE_LOG(LogTemp, Warning, TEXT("Not connected!"));
}
```

### 2. Handle Errors

```cpp
WebSocket->OnError.BindLambda([](const FString& Error) {
    UE_LOG(LogTemp, Error, TEXT("WebSocket Error: %s"), *Error);
    // Maybe try to reconnect?
});
```

### 3. Clean Disconnect

```cpp
void AMyActor::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    if (WebSocket.IsValid()) {
        WebSocket->Disconnect(1000, TEXT("Normal closure"));
        WebSocket.Reset();
    }
    Super::EndPlay(EndPlayReason);
}
```

### 4. Use Weak Pointers for Callbacks

```cpp
TWeakObjectPtr<UMyClass> WeakThis = this;

WebSocket->OnMessage.BindLambda([WeakThis](const FString& Msg, bool bIsText) {
    if (WeakThis.IsValid()) {
        WeakThis->HandleMessage(Msg);
    }
});
```

## Troubleshooting

### "WebSocket not connecting"

1. Check URL format: `ws://` or `wss://`
2. Verify server is running
3. Check firewall settings
4. Enable logging: `LogTemp: Log, All`

### "No messages received"

1. Make sure you're calling `Tick()` regularly
2. Check `OnError` delegate
3. Verify connection with `IsConnected()`

### "TLS/SSL error"

1. Use `wss://` for secure connections
2. Ensure OpenSSL module is available
3. Check server certificate validity

## Next Steps

- Read the [full README](README.md) for complete API documentation
- Check [Examples](Examples/) folder for working samples
- See [Discord integration example](../../Mods/DiscordBot/) for production usage

## Support

- Issues: [GitHub Issues](https://github.com/Yamahasxviper/SatisfactoryDiscordbot/issues)
- Documentation: [GitHub Wiki](https://github.com/Yamahasxviper/SatisfactoryDiscordbot/wiki)

Happy coding! 🚀
