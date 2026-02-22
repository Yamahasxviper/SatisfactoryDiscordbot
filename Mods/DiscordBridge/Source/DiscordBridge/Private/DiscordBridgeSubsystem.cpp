// Copyright Coffee Stain Studios. All Rights Reserved.

#include "DiscordBridgeSubsystem.h"

#include "Dom/JsonObject.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonWriter.h"
#include "Serialization/JsonSerializer.h"
#include "HttpModule.h"
#include "Interfaces/IHttpRequest.h"
#include "Interfaces/IHttpResponse.h"
#include "FGChatManager.h"

// Discord Gateway endpoint (v10, JSON encoding)
static const FString DiscordGatewayUrl = TEXT("wss://gateway.discord.gg/?v=10&encoding=json");
// Discord REST API base URL
static const FString DiscordApiBase    = TEXT("https://discord.com/api/v10");

// ─────────────────────────────────────────────────────────────────────────────
// USubsystem lifetime
// ─────────────────────────────────────────────────────────────────────────────

bool UDiscordBridgeSubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
	// Only create this subsystem on dedicated servers.
	// This prevents the bot from running on client or listen-server builds,
	// meaning players do not need this mod (or SML) installed on their own machine.
	return IsRunningDedicatedServer();
}

void UDiscordBridgeSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	// Wire up the Discord→game relay once here so it is never double-bound
	// across reconnect cycles (Connect() may be called multiple times).
	OnDiscordMessageReceived.AddDynamic(this, &UDiscordBridgeSubsystem::RelayDiscordMessageToGame);

	// Poll every second until AFGChatManager is spawned (it is an AFGSubsystem
	// actor; it is not available during USubsystem::Initialize()).  Once found,
	// bind OnNewChatMessage to its OnChatMessageAdded delegate so player chat
	// messages are forwarded to Discord without requiring any funchook.
	ChatManagerBindTickerHandle = FTSTicker::GetCoreTicker().AddTicker(
		FTickerDelegate::CreateWeakLambda(this, [this](float) -> bool
		{
			UWorld* World = GetWorld();
			if (!World) return true; // keep trying

			AFGChatManager* CM = AFGChatManager::Get(World);
			if (!CM) return true; // keep trying

			// Record the current message count so we don't replay messages
			// that existed before we connected.
			TArray<FChatMessageStruct> Existing;
			CM->GetReceivedChatMessages(Existing);
			NumSeenChatMessages = Existing.Num();

			CM->OnChatMessageAdded.AddDynamic(this, &UDiscordBridgeSubsystem::OnNewChatMessage);
			BoundChatManager = CM;

			UE_LOG(LogTemp, Log, TEXT("DiscordBridge: Bound to AFGChatManager::OnChatMessageAdded."));
			return false; // stop polling
		}),
		1.0f);

	// Load (or auto-create) the JSON config file from Configs/DiscordBridge.cfg.
	Config = FDiscordBridgeConfig::LoadOrCreate();

	if (Config.BotToken.IsEmpty() || Config.ChannelId.IsEmpty())
	{
		UE_LOG(LogTemp, Warning,
		       TEXT("DiscordBridge: BotToken or ChannelId is not configured. "
		            "Edit Mods/DiscordBridge/Config/DefaultDiscordBridge.ini to enable the bridge."));
		return;
	}

	// Log active format strings so operators can verify they were loaded correctly.
	UE_LOG(LogTemp, Log,
	       TEXT("DiscordBridge: GameToDiscordFormat = \"%s\""), *Config.GameToDiscordFormat);
	UE_LOG(LogTemp, Log,
	       TEXT("DiscordBridge: DiscordToGameFormat = \"%s\""), *Config.DiscordToGameFormat);
	UE_LOG(LogTemp, Log,
	       TEXT("DiscordBridge: DiscordSenderFormat  = \"%s\""), *Config.DiscordSenderFormat);

	Connect();
}

void UDiscordBridgeSubsystem::Deinitialize()
{
	// Stop the ChatManager-bind polling ticker (no-op if it already stopped).
	FTSTicker::GetCoreTicker().RemoveTicker(ChatManagerBindTickerHandle);
	ChatManagerBindTickerHandle.Reset();

	// Unbind from the ChatManager delegate if we managed to bind earlier.
	if (IsValid(BoundChatManager))
	{
		BoundChatManager->OnChatMessageAdded.RemoveDynamic(this, &UDiscordBridgeSubsystem::OnNewChatMessage);
		BoundChatManager = nullptr;
	}

	Disconnect();
	Super::Deinitialize();
}

// ─────────────────────────────────────────────────────────────────────────────
// Connection management
// ─────────────────────────────────────────────────────────────────────────────

void UDiscordBridgeSubsystem::Connect()
{
	if (WebSocketClient && WebSocketClient->IsConnected())
	{
		return; // Already connected.
	}

	WebSocketClient = USMLWebSocketClient::CreateWebSocketClient(this);

	// Configure auto-reconnect; Discord may close the connection at any time.
	WebSocketClient->bAutoReconnect              = true;
	WebSocketClient->ReconnectInitialDelaySeconds = 2.0f;
	WebSocketClient->MaxReconnectDelaySeconds     = 30.0f;
	WebSocketClient->MaxReconnectAttempts         = 0; // infinite

	// Bind WebSocket delegates.
	WebSocketClient->OnConnected.AddDynamic(this,  &UDiscordBridgeSubsystem::OnWebSocketConnected);
	WebSocketClient->OnMessage.AddDynamic(this,    &UDiscordBridgeSubsystem::OnWebSocketMessage);
	WebSocketClient->OnClosed.AddDynamic(this,     &UDiscordBridgeSubsystem::OnWebSocketClosed);
	WebSocketClient->OnError.AddDynamic(this,      &UDiscordBridgeSubsystem::OnWebSocketError);
	WebSocketClient->OnReconnecting.AddDynamic(this, &UDiscordBridgeSubsystem::OnWebSocketReconnecting);

	UE_LOG(LogTemp, Log, TEXT("DiscordBridge: Connecting to Discord Gateway…"));
	WebSocketClient->Connect(DiscordGatewayUrl, {}, {});
}

void UDiscordBridgeSubsystem::Disconnect()
{
	// Stop heartbeat ticker.
	FTSTicker::GetCoreTicker().RemoveTicker(HeartbeatTickerHandle);
	HeartbeatTickerHandle.Reset();

	// Signal offline status and post the server-offline notification while
	// the WebSocket is still open so Discord receives both before we close.
	if (bGatewayReady)
	{
		// Setting presence to "invisible" makes the bot appear offline to
		// users immediately, without waiting for Discord to detect the
		// WebSocket disconnection.
		SendUpdatePresence(TEXT("invisible"));

		if (!Config.ServerOfflineMessage.IsEmpty())
		{
			SendStatusMessageToDiscord(Config.ServerOfflineMessage);
		}
	}

	bGatewayReady        = false;
	bPendingHeartbeatAck = false;
	LastSequenceNumber   = -1;
	BotUserId.Empty();

	if (WebSocketClient)
	{
		WebSocketClient->Close(1000, TEXT("Client shutting down"));
		WebSocketClient = nullptr;
	}
}

// ─────────────────────────────────────────────────────────────────────────────
// WebSocket event handlers (game thread)
// ─────────────────────────────────────────────────────────────────────────────

void UDiscordBridgeSubsystem::OnWebSocketConnected()
{
	UE_LOG(LogTemp, Log, TEXT("DiscordBridge: WebSocket connection established. Awaiting Hello…"));
	// Discord will send op=10 (Hello) next; we send Identify in response.
}

void UDiscordBridgeSubsystem::OnWebSocketMessage(const FString& RawJson)
{
	HandleGatewayPayload(RawJson);
}

void UDiscordBridgeSubsystem::OnWebSocketClosed(int32 StatusCode, const FString& Reason)
{
	UE_LOG(LogTemp, Warning,
	       TEXT("DiscordBridge: Gateway connection closed (code=%d, reason='%s')."),
	       StatusCode, *Reason);

	// Detect Discord-specific close codes that indicate a permanent error.
	// For these codes reconnecting with the same credentials will never succeed,
	// so we signal the WebSocket client to stop auto-reconnecting.
	bool bTerminal = false;
	switch (StatusCode)
	{
	case 4004:
		UE_LOG(LogTemp, Error,
		       TEXT("DiscordBridge: Authentication failed (4004). "
		            "Verify BotToken in Mods/DiscordBridge/Config/DefaultDiscordBridge.ini. "
		            "Auto-reconnect disabled."));
		bTerminal = true;
		break;
	case 4010:
		UE_LOG(LogTemp, Error, TEXT("DiscordBridge: Invalid shard sent (4010). Auto-reconnect disabled."));
		bTerminal = true;
		break;
	case 4011:
		UE_LOG(LogTemp, Error, TEXT("DiscordBridge: Sharding required (4011). Auto-reconnect disabled."));
		bTerminal = true;
		break;
	case 4012:
		UE_LOG(LogTemp, Error, TEXT("DiscordBridge: Invalid Gateway API version (4012). Auto-reconnect disabled."));
		bTerminal = true;
		break;
	case 4013:
		UE_LOG(LogTemp, Error, TEXT("DiscordBridge: Invalid intent(s) (4013). Auto-reconnect disabled."));
		bTerminal = true;
		break;
	case 4014:
		UE_LOG(LogTemp, Error,
		       TEXT("DiscordBridge: Disallowed intent(s) (4014). "
		            "Enable all three Privileged Gateway Intents "
		            "(Presence, Server Members, Message Content) "
		            "in the Discord Developer Portal. Auto-reconnect disabled."));
		bTerminal = true;
		break;
	default:
		break;
	}

	if (bTerminal && WebSocketClient)
	{
		// Calling Close() sets bUserInitiatedClose on the background thread,
		// which causes the reconnect loop to exit without retrying.
		WebSocketClient->Close(1000, FString::Printf(TEXT("Terminal Discord close code %d"), StatusCode));
	}

	// Cancel heartbeat; it will be restarted on the next successful connection.
	FTSTicker::GetCoreTicker().RemoveTicker(HeartbeatTickerHandle);
	HeartbeatTickerHandle.Reset();
	bPendingHeartbeatAck = false;

	const bool bWasReady = bGatewayReady;
	bGatewayReady = false;

	if (bWasReady)
	{
		OnDiscordDisconnected.Broadcast(
			FString::Printf(TEXT("Connection closed (code %d): %s"), StatusCode, *Reason));
	}
}

void UDiscordBridgeSubsystem::OnWebSocketError(const FString& ErrorMessage)
{
	UE_LOG(LogTemp, Error, TEXT("DiscordBridge: WebSocket error: %s"), *ErrorMessage);

	FTSTicker::GetCoreTicker().RemoveTicker(HeartbeatTickerHandle);
	HeartbeatTickerHandle.Reset();
	bPendingHeartbeatAck = false;

	if (bGatewayReady)
	{
		bGatewayReady = false;
		OnDiscordDisconnected.Broadcast(FString::Printf(TEXT("WebSocket error: %s"), *ErrorMessage));
	}
}

void UDiscordBridgeSubsystem::OnWebSocketReconnecting(int32 AttemptNumber, float DelaySeconds)
{
	UE_LOG(LogTemp, Log,
	       TEXT("DiscordBridge: Reconnecting to Discord Gateway (attempt %d, delay %.1fs)…"),
	       AttemptNumber, DelaySeconds);

	// Reset Gateway state; we'll re-identify once the WebSocket reconnects.
	FTSTicker::GetCoreTicker().RemoveTicker(HeartbeatTickerHandle);
	HeartbeatTickerHandle.Reset();
	bPendingHeartbeatAck = false;
	bGatewayReady = false;
}

// ─────────────────────────────────────────────────────────────────────────────
// Discord Gateway protocol
// ─────────────────────────────────────────────────────────────────────────────

void UDiscordBridgeSubsystem::HandleGatewayPayload(const FString& RawJson)
{
	TSharedPtr<FJsonObject> Root;
	TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(RawJson);
	if (!FJsonSerializer::Deserialize(Reader, Root) || !Root.IsValid())
	{
		UE_LOG(LogTemp, Warning, TEXT("DiscordBridge: Failed to parse Gateway JSON: %s"), *RawJson);
		return;
	}

	const int32 OpCode = Root->GetIntegerField(TEXT("op"));

	switch (OpCode)
	{
	case EDiscordGatewayOpcode::Dispatch:
	{
		// Update the sequence number first; it is used in heartbeats.
		// TryGetNumberField only accepts double& in UE5's FJsonObject API.
		double Seq = -1.0;
		if (Root->TryGetNumberField(TEXT("s"), Seq))
		{
			LastSequenceNumber = static_cast<int32>(Seq);
		}

		FString EventType;
		Root->TryGetStringField(TEXT("t"), EventType);

		const TSharedPtr<FJsonObject>* DataPtr = nullptr;
		Root->TryGetObjectField(TEXT("d"), DataPtr);

		HandleDispatch(EventType, LastSequenceNumber,
		               DataPtr ? *DataPtr : MakeShared<FJsonObject>());
		break;
	}
	case EDiscordGatewayOpcode::Hello:
	{
		const TSharedPtr<FJsonObject>* DataPtr = nullptr;
		if (Root->TryGetObjectField(TEXT("d"), DataPtr) && DataPtr)
		{
			HandleHello(*DataPtr);
		}
		break;
	}
	case EDiscordGatewayOpcode::HeartbeatAck:
		HandleHeartbeatAck();
		break;

	case EDiscordGatewayOpcode::Heartbeat:
		// Server explicitly requested a heartbeat right now.
		SendHeartbeat();
		break;

	case EDiscordGatewayOpcode::Reconnect:
		HandleReconnect();
		break;

	case EDiscordGatewayOpcode::InvalidSession:
	{
		bool bResumable = false;
		Root->TryGetBoolField(TEXT("d"), bResumable);
		HandleInvalidSession(bResumable);
		break;
	}
	default:
		UE_LOG(LogTemp, VeryVerbose,
		       TEXT("DiscordBridge: Unhandled opcode %d"), OpCode);
		break;
	}
}

void UDiscordBridgeSubsystem::HandleHello(const TSharedPtr<FJsonObject>& DataObj)
{
	// Discord sends the heartbeat interval in milliseconds.
	double HeartbeatMs = 41250.0; // sensible default
	DataObj->TryGetNumberField(TEXT("heartbeat_interval"), HeartbeatMs);
	HeartbeatIntervalSeconds = static_cast<float>(HeartbeatMs) / 1000.0f;

	UE_LOG(LogTemp, Log,
	       TEXT("DiscordBridge: Hello received. Heartbeat interval: %.2f s"),
	       HeartbeatIntervalSeconds);

	// Start heartbeating with a random jitter so that all bots don't hammer the
	// Gateway simultaneously on mass-reconnects (Discord "thundering herd" concern).
	// Strategy: one-shot ticker after a random [0, interval] delay fires the first
	// heartbeat and then installs the regular repeating ticker.
	// HeartbeatTickerHandle tracks whichever ticker is active so Disconnect() can
	// always cancel it with a single RemoveTicker() call.
	FTSTicker::GetCoreTicker().RemoveTicker(HeartbeatTickerHandle);
	bPendingHeartbeatAck = false;

	const float JitterSeconds = FMath::FRandRange(0.0f, HeartbeatIntervalSeconds);
	HeartbeatTickerHandle = FTSTicker::GetCoreTicker().AddTicker(
		FTickerDelegate::CreateWeakLambda(this, [this](float) -> bool
		{
			SendHeartbeat();
			// Replace the one-shot handle with the regular repeating ticker.
			HeartbeatTickerHandle = FTSTicker::GetCoreTicker().AddTicker(
				FTickerDelegate::CreateUObject(this, &UDiscordBridgeSubsystem::HeartbeatTick),
				HeartbeatIntervalSeconds);
			return false; // one-shot – do not repeat
		}),
		JitterSeconds);

	// Send Identify so Discord authenticates us.
	SendIdentify();
}

void UDiscordBridgeSubsystem::HandleDispatch(const FString& EventType, int32 Sequence,
                                              const TSharedPtr<FJsonObject>& DataObj)
{
	if (EventType == TEXT("READY"))
	{
		HandleReady(DataObj);
	}
	else if (EventType == TEXT("MESSAGE_CREATE"))
	{
		HandleMessageCreate(DataObj);
	}
	// Other events (PRESENCE_UPDATE, GUILD_MEMBER_ADD, …) are received because of
	// the intents we request but are not processed by this bridge.
}

void UDiscordBridgeSubsystem::HandleHeartbeatAck()
{
	UE_LOG(LogTemp, VeryVerbose, TEXT("DiscordBridge: Heartbeat acknowledged."));
	bPendingHeartbeatAck = false;
}

void UDiscordBridgeSubsystem::HandleReconnect()
{
	UE_LOG(LogTemp, Log, TEXT("DiscordBridge: Server requested reconnect."));

	// Reset Gateway state; we'll re-identify once the WebSocket reconnects.
	FTSTicker::GetCoreTicker().RemoveTicker(HeartbeatTickerHandle);
	HeartbeatTickerHandle.Reset();
	bPendingHeartbeatAck = false;
	bGatewayReady        = false;
	LastSequenceNumber   = -1;
	BotUserId.Empty();

	// Restart the WebSocket connection by calling Connect() on the existing
	// client.  Do NOT call Close() here: Close() → EnqueueClose() sets
	// bUserInitiatedClose = true inside FSMLWebSocketRunnable, which exits
	// the reconnect loop permanently and leaves the bot offline.
	// Connect() stops the current thread and starts a fresh runnable with
	// auto-reconnect still enabled.
	if (WebSocketClient)
	{
		WebSocketClient->Connect(DiscordGatewayUrl, {}, {});
	}
}

void UDiscordBridgeSubsystem::HandleInvalidSession(bool bResumable)
{
	UE_LOG(LogTemp, Warning,
	       TEXT("DiscordBridge: Invalid session (resumable=%s). Re-identifying in 2s…"),
	       bResumable ? TEXT("true") : TEXT("false"));

	// Per Discord spec, wait 1–5 seconds before re-identifying.
	// Use a one-shot FTSTicker so the game thread is never blocked.
	FTSTicker::GetCoreTicker().AddTicker(
		FTickerDelegate::CreateWeakLambda(this, [this](float) -> bool
		{
			SendIdentify();
			return false; // one-shot – do not repeat
		}),
		2.0f);
}

void UDiscordBridgeSubsystem::HandleReady(const TSharedPtr<FJsonObject>& DataObj)
{
	// Extract the bot user ID so we can filter out self-sent messages.
	const TSharedPtr<FJsonObject>* UserPtr = nullptr;
	if (DataObj->TryGetObjectField(TEXT("user"), UserPtr) && UserPtr)
	{
		(*UserPtr)->TryGetStringField(TEXT("id"), BotUserId);
	}

	bGatewayReady = true;

	UE_LOG(LogTemp, Log,
	       TEXT("DiscordBridge: Gateway ready. Bot user ID: %s"), *BotUserId);

	// Set bot presence to Online so Discord shows it as available.
	SendUpdatePresence(TEXT("online"));

	// Post the server-online notification message, if configured.
	if (!Config.ServerOnlineMessage.IsEmpty())
	{
		SendStatusMessageToDiscord(Config.ServerOnlineMessage);
	}

	OnDiscordConnected.Broadcast();
}

void UDiscordBridgeSubsystem::HandleMessageCreate(const TSharedPtr<FJsonObject>& DataObj)
{
	// Only process messages from the configured channel.
	FString ChannelId;
	if (!DataObj->TryGetStringField(TEXT("channel_id"), ChannelId) ||
	    ChannelId != Config.ChannelId)
	{
		return;
	}

	// Extract the author object.
	const TSharedPtr<FJsonObject>* AuthorPtr = nullptr;
	if (!DataObj->TryGetObjectField(TEXT("author"), AuthorPtr) || !AuthorPtr)
	{
		return;
	}
	const TSharedPtr<FJsonObject>& Author = *AuthorPtr;

	// Optionally ignore bot messages (including our own) to prevent echo loops.
	if (Config.bIgnoreBotMessages)
	{
		bool bIsBot = false;
		Author->TryGetBoolField(TEXT("bot"), bIsBot);
		if (bIsBot)
		{
			return;
		}
	}
	// Always ignore this bot's own messages regardless of the config flag.
	FString AuthorId;
	Author->TryGetStringField(TEXT("id"), AuthorId);
	if (!BotUserId.IsEmpty() && AuthorId == BotUserId)
	{
		return;
	}

	// Display name priority: server nickname > global display name > username.
	// The member object is included in MESSAGE_CREATE events for guild messages
	// when the GUILD_MEMBERS intent is enabled.
	FString Username;
	const TSharedPtr<FJsonObject>* MemberPtr = nullptr;
	if (DataObj->TryGetObjectField(TEXT("member"), MemberPtr) && MemberPtr)
	{
		(*MemberPtr)->TryGetStringField(TEXT("nick"), Username);
	}
	if (Username.IsEmpty())
	{
		if (!Author->TryGetStringField(TEXT("global_name"), Username) || Username.IsEmpty())
		{
			Author->TryGetStringField(TEXT("username"), Username);
		}
	}
	// Final safety fallback: every Discord user has a username, but guard against
	// unexpected API responses that omit all name fields.
	if (Username.IsEmpty())
	{
		UE_LOG(LogTemp, Warning,
		       TEXT("DiscordBridge: Could not extract display name from Discord message author; using 'Discord User'."));
		Username = TEXT("Discord User");
	}

	FString Content;
	DataObj->TryGetStringField(TEXT("content"), Content);
	Content = Content.TrimStartAndEnd();

	if (Content.IsEmpty())
	{
		return; // Embeds-only, sticker-only, or whitespace-only messages; skip.
	}

	UE_LOG(LogTemp, Log,
	       TEXT("DiscordBridge: Discord message received from '%s': %s"), *Username, *Content);

	OnDiscordMessageReceived.Broadcast(Username, Content);
}

// ─────────────────────────────────────────────────────────────────────────────
// Sending Gateway payloads
// ─────────────────────────────────────────────────────────────────────────────

void UDiscordBridgeSubsystem::SendIdentify()
{
	// Build the connection properties object.
	// The "os" property is informational; Discord uses it to identify the client
	// platform.  Use the actual compile-time platform so it is accurate for both
	// the Windows and Linux dedicated-server Alpakit targets.
#if PLATFORM_WINDOWS
	static const FString DiscordOs = TEXT("windows");
#elif PLATFORM_LINUX
	static const FString DiscordOs = TEXT("linux");
#else
	static const FString DiscordOs = TEXT("unknown");
#endif
	TSharedPtr<FJsonObject> Props = MakeShared<FJsonObject>();
	Props->SetStringField(TEXT("os"),      DiscordOs);
	Props->SetStringField(TEXT("browser"), TEXT("satisfactory_discord_bridge"));
	Props->SetStringField(TEXT("device"),  TEXT("satisfactory_discord_bridge"));

	// Set the initial presence so the bot appears online immediately upon
	// authentication, before a separate UpdatePresence op is sent.
	TSharedPtr<FJsonObject> InitialPresence = MakeShared<FJsonObject>();
	InitialPresence->SetField(TEXT("since"), MakeShared<FJsonValueNull>());
	InitialPresence->SetArrayField(TEXT("activities"), TArray<TSharedPtr<FJsonValue>>());
	InitialPresence->SetStringField(TEXT("status"), TEXT("online"));
	InitialPresence->SetBoolField(TEXT("afk"), false);

	// Build the Identify data object.
	TSharedPtr<FJsonObject> Data = MakeShared<FJsonObject>();
	Data->SetStringField(TEXT("token"),   Config.BotToken);
	Data->SetNumberField(TEXT("intents"), EDiscordGatewayIntent::All);
	Data->SetObjectField(TEXT("properties"), Props);
	Data->SetObjectField(TEXT("presence"), InitialPresence);

	// Wrap in the Gateway payload envelope.
	TSharedPtr<FJsonObject> Payload = MakeShared<FJsonObject>();
	Payload->SetNumberField(TEXT("op"), EDiscordGatewayOpcode::Identify);
	Payload->SetObjectField(TEXT("d"),  Data);

	SendGatewayPayload(Payload);

	UE_LOG(LogTemp, Log,
	       TEXT("DiscordBridge: Identify sent (intents=%d)."),
	       EDiscordGatewayIntent::All);
}

void UDiscordBridgeSubsystem::SendHeartbeat()
{
	// Zombie-connection detection (per Discord Gateway documentation):
	// If the previous heartbeat was never acknowledged, the connection is a
	// zombie – packets are being sent locally but not reaching Discord.
	// Discord has already marked the bot offline.  Close the WebSocket with a
	// non-1000 code so USMLWebSocketClient's auto-reconnect triggers a fresh
	// connection and re-identification.
	if (bPendingHeartbeatAck)
	{
		UE_LOG(LogTemp, Warning,
		       TEXT("DiscordBridge: Heartbeat not acknowledged – zombie connection detected. "
		            "Closing to trigger reconnect."));

		// Cancel the heartbeat ticker before closing so we don't send more
		// heartbeats on a dead socket.
		FTSTicker::GetCoreTicker().RemoveTicker(HeartbeatTickerHandle);
		HeartbeatTickerHandle.Reset();
		bPendingHeartbeatAck = false;

		if (WebSocketClient)
		{
			// Non-1000 close code signals USMLWebSocketClient that this was
			// not a user-initiated close, so it will attempt to reconnect.
			WebSocketClient->Close(1001, TEXT("Zombie connection – no heartbeat ack"));
		}
		return;
	}

	TSharedPtr<FJsonObject> Payload = MakeShared<FJsonObject>();
	Payload->SetNumberField(TEXT("op"), EDiscordGatewayOpcode::Heartbeat);

	// The heartbeat data field must be the last received sequence number, or
	// a JSON null if no dispatch has been received yet.
	if (LastSequenceNumber >= 0)
	{
		Payload->SetNumberField(TEXT("d"), LastSequenceNumber);
	}
	else
	{
		Payload->SetField(TEXT("d"), MakeShared<FJsonValueNull>());
	}

	SendGatewayPayload(Payload);
	bPendingHeartbeatAck = true;
}

void UDiscordBridgeSubsystem::SendUpdatePresence(const FString& Status)
{
	// Build the presence data object (Discord Gateway op=3).
	TSharedPtr<FJsonObject> Data = MakeShared<FJsonObject>();
	Data->SetField(TEXT("since"),      MakeShared<FJsonValueNull>());
	Data->SetArrayField(TEXT("activities"), TArray<TSharedPtr<FJsonValue>>());
	Data->SetStringField(TEXT("status"), Status);
	Data->SetBoolField(TEXT("afk"),    false);

	TSharedPtr<FJsonObject> Payload = MakeShared<FJsonObject>();
	Payload->SetNumberField(TEXT("op"), EDiscordGatewayOpcode::UpdatePresence);
	Payload->SetObjectField(TEXT("d"),  Data);

	SendGatewayPayload(Payload);

	UE_LOG(LogTemp, Log, TEXT("DiscordBridge: Presence updated to '%s'."), *Status);
}

void UDiscordBridgeSubsystem::SendStatusMessageToDiscord(const FString& Message)
{
	if (Config.BotToken.IsEmpty() || Config.ChannelId.IsEmpty())
	{
		return;
	}

	TSharedPtr<FJsonObject> Body = MakeShared<FJsonObject>();
	Body->SetStringField(TEXT("content"), Message);

	FString BodyString;
	TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&BodyString);
	FJsonSerializer::Serialize(Body.ToSharedRef(), Writer);

	const FString Url = FString::Printf(
		TEXT("%s/channels/%s/messages"), *DiscordApiBase, *Config.ChannelId);

	TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request =
		FHttpModule::Get().CreateRequest();

	Request->SetURL(Url);
	Request->SetVerb(TEXT("POST"));
	Request->SetHeader(TEXT("Content-Type"),  TEXT("application/json"));
	Request->SetHeader(TEXT("Authorization"),
	                   FString::Printf(TEXT("Bot %s"), *Config.BotToken));
	Request->SetContentAsString(BodyString);

	Request->OnProcessRequestComplete().BindWeakLambda(
		this,
		[Message](FHttpRequestPtr /*Req*/, FHttpResponsePtr Resp, bool bConnected)
		{
			if (!bConnected || !Resp.IsValid())
			{
				UE_LOG(LogTemp, Warning,
				       TEXT("DiscordBridge: HTTP request failed for status message '%s'."),
				       *Message);
				return;
			}
			if (Resp->GetResponseCode() < 200 || Resp->GetResponseCode() >= 300)
			{
				UE_LOG(LogTemp, Warning,
				       TEXT("DiscordBridge: Discord REST API returned %d: %s"),
				       Resp->GetResponseCode(), *Resp->GetContentAsString());
			}
		});

	Request->ProcessRequest();
}

void UDiscordBridgeSubsystem::SendGatewayPayload(const TSharedPtr<FJsonObject>& Payload)
{
	if (!WebSocketClient || !WebSocketClient->IsConnected())
	{
		return;
	}

	FString JsonString;
	TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&JsonString);
	FJsonSerializer::Serialize(Payload.ToSharedRef(), Writer);

	WebSocketClient->SendText(JsonString);
}

// ─────────────────────────────────────────────────────────────────────────────
// Heartbeat timer
// ─────────────────────────────────────────────────────────────────────────────

bool UDiscordBridgeSubsystem::HeartbeatTick(float /*DeltaTime*/)
{
	SendHeartbeat();
	return true; // keep ticking
}

// ─────────────────────────────────────────────────────────────────────────────
// Chat-manager delegate handler (Game → Discord)
// ─────────────────────────────────────────────────────────────────────────────

void UDiscordBridgeSubsystem::OnNewChatMessage()
{
	if (!IsValid(BoundChatManager)) return;

	TArray<FChatMessageStruct> Messages;
	BoundChatManager->GetReceivedChatMessages(Messages);

	if (Messages.Num() == 0) return;

	// AFGChatManager keeps a rolling history bounded by mMaxNumMessagesInHistory
	// (default 50).  Once the buffer is full, each new message evicts the oldest
	// one, so Messages.Num() stays constant.  In that case NumSeenChatMessages
	// equals Messages.Num() even though a new message was just added, and a
	// naive "> NumSeenChatMessages" guard would silently drop every message after
	// the 50th.  Detect this by falling back to the last element (the newest
	// message) whenever the count has not grown.
	//
	// In the normal (non-full) case we process all entries from NumSeenChatMessages
	// onward; in the rolling-buffer case we process only the last entry.
	// Forward only CMT_PlayerMessage entries to Discord.  Messages relayed FROM
	// Discord are also CMT_PlayerMessage, so PendingRelayedMessages is used to
	// identify and skip them before they are forwarded back to Discord.
	const int32 StartIdx = (Messages.Num() > NumSeenChatMessages)
	                       ? NumSeenChatMessages
	                       : (Messages.Num() - 1);

	for (int32 i = StartIdx; i < Messages.Num(); ++i)
	{
		const FChatMessageStruct& Msg = Messages[i];
		if (Msg.MessageType == EFGChatMessageType::CMT_PlayerMessage)
		{
			const FString PlayerName  = Msg.MessageSender.ToString().TrimStartAndEnd();
			const FString MessageText = Msg.MessageText.ToString().TrimStartAndEnd();

			// Skip messages we relayed from Discord to prevent echo loops.
			// RelayDiscordMessageToGame() pre-registers the FormattedMessage text
			// in PendingRelayedMessages; Remove() returns > 0 when found.
			if (PendingRelayedMessages.Remove(MessageText) > 0)
			{
				UE_LOG(LogTemp, VeryVerbose,
				       TEXT("DiscordBridge: Suppressing echo of Discord-relayed message: '%s'"),
				       *MessageText);
				continue;
			}

			UE_LOG(LogTemp, Log,
			       TEXT("DiscordBridge: Player message detected. Sender: '%s', Text: '%s'"),
			       *PlayerName, *MessageText);

			if (MessageText.IsEmpty())
			{
				UE_LOG(LogTemp, Warning,
				       TEXT("DiscordBridge: Skipping player message with empty text from '%s'."),
				       *PlayerName);
				continue;
			}

			SendGameMessageToDiscord(PlayerName, MessageText);
		}
	}
	NumSeenChatMessages = Messages.Num();
}

// ─────────────────────────────────────────────────────────────────────────────

void UDiscordBridgeSubsystem::SendGameMessageToDiscord(const FString& PlayerName,
                                                        const FString& Message)
{
	if (Config.BotToken.IsEmpty() || Config.ChannelId.IsEmpty())
	{
		UE_LOG(LogTemp, Warning,
		       TEXT("DiscordBridge: Cannot send message – BotToken or ChannelId not configured."));
		return;
	}

	// Apply the configurable format string.
	// Fall back to a plain "Name: Message" string when the format is empty so
	// the message is never silently discarded due to a misconfigured INI.
	const FString EffectivePlayerName = PlayerName.IsEmpty() ? TEXT("Unknown") : PlayerName;

	FString FormattedContent = Config.GameToDiscordFormat;
	FormattedContent = FormattedContent.Replace(TEXT("{PlayerName}"), *EffectivePlayerName);
	FormattedContent = FormattedContent.Replace(TEXT("{Message}"),    *Message);

	if (FormattedContent.IsEmpty())
	{
		UE_LOG(LogTemp, Warning,
		       TEXT("DiscordBridge: GameToDiscordFormat produced an empty string for player '%s'. "
		            "Check the GameToDiscordFormat setting in DefaultDiscordBridge.ini."),
		       *EffectivePlayerName);
		return;
	}

	UE_LOG(LogTemp, Log,
	       TEXT("DiscordBridge: Sending to Discord: %s"), *FormattedContent);

	// Build the JSON body: {"content": "…"}
	TSharedPtr<FJsonObject> Body = MakeShared<FJsonObject>();
	Body->SetStringField(TEXT("content"), FormattedContent);

	FString BodyString;
	TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&BodyString);
	FJsonSerializer::Serialize(Body.ToSharedRef(), Writer);

	// POST to the Discord REST API.
	const FString Url = FString::Printf(
		TEXT("%s/channels/%s/messages"), *DiscordApiBase, *Config.ChannelId);

	TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request =
		FHttpModule::Get().CreateRequest();

	Request->SetURL(Url);
	Request->SetVerb(TEXT("POST"));
	Request->SetHeader(TEXT("Content-Type"),  TEXT("application/json"));
	Request->SetHeader(TEXT("Authorization"),
	                   FString::Printf(TEXT("Bot %s"), *Config.BotToken));
	Request->SetContentAsString(BodyString);

	Request->OnProcessRequestComplete().BindWeakLambda(
		this,
		[EffectivePlayerName](FHttpRequestPtr /*Req*/, FHttpResponsePtr Resp, bool bConnected)
		{
			if (!bConnected || !Resp.IsValid())
			{
				UE_LOG(LogTemp, Warning,
				       TEXT("DiscordBridge: HTTP request failed for player '%s'."),
				       *EffectivePlayerName);
				return;
			}
			if (Resp->GetResponseCode() < 200 || Resp->GetResponseCode() >= 300)
			{
				UE_LOG(LogTemp, Warning,
				       TEXT("DiscordBridge: Discord REST API returned %d: %s"),
				       Resp->GetResponseCode(), *Resp->GetContentAsString());
			}
		});

	Request->ProcessRequest();
}

// ─────────────────────────────────────────────────────────────────────────────
// Discord → Game chat relay
// ─────────────────────────────────────────────────────────────────────────────

void UDiscordBridgeSubsystem::RelayDiscordMessageToGame(const FString& Username,
                                                         const FString& Message)
{
	UWorld* World = GetWorld();
	if (!World)
	{
		UE_LOG(LogTemp, Warning,
		       TEXT("DiscordBridge: No world available – cannot relay Discord message to game chat."));
		return;
	}

	AFGChatManager* ChatManager = AFGChatManager::Get(World);
	if (!ChatManager)
	{
		UE_LOG(LogTemp, Warning,
		       TEXT("DiscordBridge: ChatManager not found – cannot relay Discord message to game chat."));
		return;
	}

	// Apply the configurable format string (DiscordToGameFormat) to the message body.
	// Use a fallback if the format is empty so the message is never silently
	// dropped due to a misconfigured INI.
	FString FormattedMessage = Config.DiscordToGameFormat;
	FormattedMessage = FormattedMessage.Replace(TEXT("{Username}"), *Username);
	FormattedMessage = FormattedMessage.Replace(TEXT("{Message}"),  *Message);

	if (FormattedMessage.IsEmpty())
	{
		// Format produced an empty result – fall back to the raw message so the
		// content is always visible rather than silently dropped.
		FormattedMessage = Message;
	}

	// Build the sender label that will appear in the chat name column using
	// the configurable DiscordSenderFormat. Falls back to "[Discord] {Username}"
	// if the format produces an empty string.
	FString SenderLabel = Config.DiscordSenderFormat;
	SenderLabel = SenderLabel.Replace(TEXT("{Username}"), *Username);
	if (SenderLabel.IsEmpty())
	{
		SenderLabel = FString::Printf(TEXT("[Discord] %s"), *Username);
	}

	FChatMessageStruct ChatMsg;
	// Use CMT_PlayerMessage so the game's chat widget renders both the sender
	// name (MessageSender) and the message body (MessageText) visibly.
	// CMT_SystemMessage can suppress or override MessageText in some engine
	// builds, causing only the sender label to appear.
	ChatMsg.MessageType   = EFGChatMessageType::CMT_PlayerMessage;
	ChatMsg.MessageSender = FText::FromString(SenderLabel);
	ChatMsg.MessageText   = FText::FromString(FormattedMessage);

	// Register this message before broadcasting so that OnNewChatMessage can
	// recognise and skip it, preventing an echo back to Discord.
	PendingRelayedMessages.Add(FormattedMessage.TrimStartAndEnd());

	ChatManager->BroadcastChatMessage(ChatMsg);

	UE_LOG(LogTemp, Log,
	       TEXT("DiscordBridge: Relayed to game chat – sender: '%s', text: '%s'"),
	       *SenderLabel, *FormattedMessage);
}
