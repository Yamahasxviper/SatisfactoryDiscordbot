// Copyright Coffee Stain Studios. All Rights Reserved.

#include "DiscordBridgeSubsystem.h"
#include "DiscordBridgeConfig.h"

#include "Dom/JsonObject.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonWriter.h"
#include "Serialization/JsonSerializer.h"
#include "HttpModule.h"
#include "Interfaces/IHttpRequest.h"
#include "Interfaces/IHttpResponse.h"

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

	const UDiscordBridgeConfig* Config = GetDefault<UDiscordBridgeConfig>();

	if (Config->BotToken.IsEmpty() || Config->ChannelId.IsEmpty())
	{
		UE_LOG(LogTemp, Warning,
		       TEXT("DiscordBridge: BotToken or ChannelId is not configured. "
		            "Edit Saved/Config/<Platform>/DiscordBridge.ini to enable the bridge."));
		return;
	}

	Connect();
}

void UDiscordBridgeSubsystem::Deinitialize()
{
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

	// Post the server-offline notification message while still connected.
	if (bGatewayReady)
	{
		const UDiscordBridgeConfig* Config = GetDefault<UDiscordBridgeConfig>();
		if (!Config->ServerOfflineMessage.IsEmpty())
		{
			SendStatusMessageToDiscord(Config->ServerOfflineMessage);
		}
	}

	bGatewayReady      = false;
	LastSequenceNumber = -1;
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

	// Cancel heartbeat; it will be restarted on the next successful connection.
	FTSTicker::GetCoreTicker().RemoveTicker(HeartbeatTickerHandle);
	HeartbeatTickerHandle.Reset();

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

	// Start the heartbeat ticker. FTSTicker is engine-level and fires on the
	// game thread regardless of whether a UWorld exists yet, which avoids the
	// race condition where Hello arrives before the dedicated-server world is
	// fully loaded and GetWorld() still returns null.
	FTSTicker::GetCoreTicker().RemoveTicker(HeartbeatTickerHandle);
	HeartbeatTickerHandle = FTSTicker::GetCoreTicker().AddTicker(
		FTickerDelegate::CreateUObject(this, &UDiscordBridgeSubsystem::HeartbeatTick),
		HeartbeatIntervalSeconds);

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
}

void UDiscordBridgeSubsystem::HandleReconnect()
{
	UE_LOG(LogTemp, Log, TEXT("DiscordBridge: Server requested reconnect."));
	// Close and let USMLWebSocketClient's auto-reconnect handle the rest.
	if (WebSocketClient)
	{
		WebSocketClient->Close(1000, TEXT("Reconnect requested by server"));
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
	const UDiscordBridgeConfig* Config = GetDefault<UDiscordBridgeConfig>();
	if (!Config->ServerOnlineMessage.IsEmpty())
	{
		SendStatusMessageToDiscord(Config->ServerOnlineMessage);
	}

	OnDiscordConnected.Broadcast();
}

void UDiscordBridgeSubsystem::HandleMessageCreate(const TSharedPtr<FJsonObject>& DataObj)
{
	// Only process messages from the configured channel.
	const UDiscordBridgeConfig* Config = GetDefault<UDiscordBridgeConfig>();

	FString ChannelId;
	if (!DataObj->TryGetStringField(TEXT("channel_id"), ChannelId) ||
	    ChannelId != Config->ChannelId)
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
	if (Config->bIgnoreBotMessages)
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

	// Prefer the global_name (display name) introduced in Discord username 2.0,
	// fall back to the classic username.
	FString Username;
	if (!Author->TryGetStringField(TEXT("global_name"), Username) || Username.IsEmpty())
	{
		Author->TryGetStringField(TEXT("username"), Username);
	}

	FString Content;
	DataObj->TryGetStringField(TEXT("content"), Content);

	if (Content.IsEmpty())
	{
		return; // Embeds-only or attachment-only messages; skip.
	}

	UE_LOG(LogTemp, Log,
	       TEXT("DiscordBridge: [%s] %s"), *Username, *Content);

	OnDiscordMessageReceived.Broadcast(Username, Content);
}

// ─────────────────────────────────────────────────────────────────────────────
// Sending Gateway payloads
// ─────────────────────────────────────────────────────────────────────────────

void UDiscordBridgeSubsystem::SendIdentify()
{
	const UDiscordBridgeConfig* Config = GetDefault<UDiscordBridgeConfig>();

	// Build the connection properties object.
	TSharedPtr<FJsonObject> Props = MakeShared<FJsonObject>();
	Props->SetStringField(TEXT("os"),      TEXT("windows"));
	Props->SetStringField(TEXT("browser"), TEXT("satisfactory_discord_bridge"));
	Props->SetStringField(TEXT("device"),  TEXT("satisfactory_discord_bridge"));

	// Build the Identify data object.
	TSharedPtr<FJsonObject> Data = MakeShared<FJsonObject>();
	Data->SetStringField(TEXT("token"),   Config->BotToken);
	Data->SetNumberField(TEXT("intents"), EDiscordGatewayIntent::All);
	Data->SetObjectField(TEXT("properties"), Props);

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
	const UDiscordBridgeConfig* Config = GetDefault<UDiscordBridgeConfig>();

	if (Config->BotToken.IsEmpty() || Config->ChannelId.IsEmpty())
	{
		return;
	}

	TSharedPtr<FJsonObject> Body = MakeShared<FJsonObject>();
	Body->SetStringField(TEXT("content"), Message);

	FString BodyString;
	TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&BodyString);
	FJsonSerializer::Serialize(Body.ToSharedRef(), Writer);

	const FString Url = FString::Printf(
		TEXT("%s/channels/%s/messages"), *DiscordApiBase, *Config->ChannelId);

	TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request =
		FHttpModule::Get().CreateRequest();

	Request->SetURL(Url);
	Request->SetVerb(TEXT("POST"));
	Request->SetHeader(TEXT("Content-Type"),  TEXT("application/json"));
	Request->SetHeader(TEXT("Authorization"),
	                   FString::Printf(TEXT("Bot %s"), *Config->BotToken));
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
// Game → Discord (REST API)
// ─────────────────────────────────────────────────────────────────────────────

void UDiscordBridgeSubsystem::SendGameMessageToDiscord(const FString& PlayerName,
                                                        const FString& Message)
{
	const UDiscordBridgeConfig* Config = GetDefault<UDiscordBridgeConfig>();

	if (Config->BotToken.IsEmpty() || Config->ChannelId.IsEmpty())
	{
		UE_LOG(LogTemp, Warning,
		       TEXT("DiscordBridge: Cannot send message – BotToken or ChannelId not configured."));
		return;
	}

	// Apply the configurable format string.
	FString FormattedContent = Config->GameToDiscordFormat;
	FormattedContent = FormattedContent.Replace(TEXT("{PlayerName}"), *PlayerName);
	FormattedContent = FormattedContent.Replace(TEXT("{Message}"),    *Message);

	// Build the JSON body: {"content": "…"}
	TSharedPtr<FJsonObject> Body = MakeShared<FJsonObject>();
	Body->SetStringField(TEXT("content"), FormattedContent);

	FString BodyString;
	TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&BodyString);
	FJsonSerializer::Serialize(Body.ToSharedRef(), Writer);

	// POST to the Discord REST API.
	const FString Url = FString::Printf(
		TEXT("%s/channels/%s/messages"), *DiscordApiBase, *Config->ChannelId);

	TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request =
		FHttpModule::Get().CreateRequest();

	Request->SetURL(Url);
	Request->SetVerb(TEXT("POST"));
	Request->SetHeader(TEXT("Content-Type"),  TEXT("application/json"));
	Request->SetHeader(TEXT("Authorization"),
	                   FString::Printf(TEXT("Bot %s"), *Config->BotToken));
	Request->SetContentAsString(BodyString);

	Request->OnProcessRequestComplete().BindWeakLambda(
		this,
		[PlayerName](FHttpRequestPtr /*Req*/, FHttpResponsePtr Resp, bool bConnected)
		{
			if (!bConnected || !Resp.IsValid())
			{
				UE_LOG(LogTemp, Warning,
				       TEXT("DiscordBridge: HTTP request failed for player '%s'."),
				       *PlayerName);
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
