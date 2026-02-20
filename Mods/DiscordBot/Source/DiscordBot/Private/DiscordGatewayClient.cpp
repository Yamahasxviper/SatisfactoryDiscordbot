// Copyright Epic Games, Inc. All Rights Reserved.

#include "DiscordGatewayClient.h"

#include "Dom/JsonObject.h"
#include "Dom/JsonValue.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonWriter.h"
#include "Serialization/JsonSerializer.h"

DEFINE_LOG_CATEGORY_STATIC(LogDiscordGateway, Log, All);

// ---------------------------------------------------------------------------
// UDiscordGatewayClient
// ---------------------------------------------------------------------------

UDiscordGatewayClient::UDiscordGatewayClient()
	: WebSocket(nullptr)
	, GatewayIntents(5633)
	, LastSequenceNumber(-1)
	, bIsReady(false)
	, HeartbeatIntervalSeconds(0.f)
	, HeartbeatElapsedSeconds(0.f)
	, bHeartbeatAckReceived(true)
{}

void UDiscordGatewayClient::BeginDestroy()
{
	Disconnect();
	Super::BeginDestroy();
}

// ---------------------------------------------------------------------------

void UDiscordGatewayClient::Connect(const FString& InBotToken,
	const FString& GatewayUrl, int32 Intents)
{
	if (WebSocket && WebSocket->IsConnected())
	{
		UE_LOG(LogDiscordGateway, Warning,
			TEXT("UDiscordGatewayClient::Connect() called while already connected. "
			     "Call Disconnect() first."));
		return;
	}

	BotToken       = InBotToken;
	GatewayIntents = Intents;
	LastSequenceNumber = -1;
	bIsReady           = false;
	bHeartbeatAckReceived = true;

	// Warn if the caller has accidentally included privileged intent bits.
	constexpr int32 PrivilegedIntentsMask =
		2      |  // GUILD_MEMBERS   – Server Members Intent
		256    |  // GUILD_PRESENCES – Presence Intent
		32768;    // MESSAGE_CONTENT – Message Content Intent

	if (Intents & PrivilegedIntentsMask)
	{
		UE_LOG(LogDiscordGateway, Warning,
			TEXT("UDiscordGatewayClient::Connect(): the Intents bitmask (0x%X) includes "
			     "one or more privileged intent bits (GUILD_MEMBERS=2, GUILD_PRESENCES=256, "
			     "MESSAGE_CONTENT=32768). These require explicit enablement in the Discord "
			     "Developer Portal and have been included as-is. Remove them if unintended."),
			Intents);
	}

	// Create (or reuse) the WebSocket object.
	if (!WebSocket)
	{
		WebSocket = NewObject<USMLWebSocket>(this);
		WebSocket->OnConnected.AddDynamic(this, &UDiscordGatewayClient::HandleWebSocketConnected);
		WebSocket->OnMessageReceived.AddDynamic(this, &UDiscordGatewayClient::HandleWebSocketMessage);
		WebSocket->OnConnectionError.AddDynamic(this, &UDiscordGatewayClient::HandleWebSocketError);
		WebSocket->OnClosed.AddDynamic(this, &UDiscordGatewayClient::HandleWebSocketClosed);
	}

	// Discord Gateway requires a valid JSON connection; disable auto-reconnect
	// at the transport level – the Gateway protocol handles reconnect/resume itself.
	WebSocket->bAutoReconnect = false;

	UE_LOG(LogDiscordGateway, Log,
		TEXT("UDiscordGatewayClient: connecting to Discord Gateway at %s (intents=0x%X)"),
		*GatewayUrl, Intents);

	WebSocket->Connect(GatewayUrl);
}

void UDiscordGatewayClient::Disconnect()
{
	StopHeartbeat();
	bIsReady = false;

	if (WebSocket && WebSocket->IsConnected())
	{
		WebSocket->Close(1000, TEXT("Normal closure"));
	}
}

// ---------------------------------------------------------------------------
// WebSocket event handlers
// ---------------------------------------------------------------------------

void UDiscordGatewayClient::HandleWebSocketConnected()
{
	UE_LOG(LogDiscordGateway, Log,
		TEXT("UDiscordGatewayClient: WebSocket transport connected. "
		     "Waiting for Discord Gateway HELLO..."));
	// Discord sends HELLO immediately; we wait for HandleWebSocketMessage.
}

void UDiscordGatewayClient::HandleWebSocketMessage(const FString& Message)
{
	// Parse the top-level Gateway payload: { "op": N, "s": N|null, "t": "EVENT"|null, "d": {...} }
	TSharedPtr<FJsonObject> Root;
	TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Message);
	if (!FJsonSerializer::Deserialize(Reader, Root) || !Root.IsValid())
	{
		UE_LOG(LogDiscordGateway, Warning,
			TEXT("UDiscordGatewayClient: Failed to parse Gateway message JSON: %s"),
			*Message.Left(256));
		return;
	}

	// Opcode (required field)
	int32 Op = 0;
	if (!Root->TryGetNumberField(TEXT("op"), Op))
	{
		UE_LOG(LogDiscordGateway, Warning,
			TEXT("UDiscordGatewayClient: Gateway message missing 'op' field"));
		return;
	}

	// Sequence number (optional; only present on DISPATCH)
	int32 Seq = -1;
	{
		int32 SeqVal = 0;
		if (Root->TryGetNumberField(TEXT("s"), SeqVal))
		{
			Seq = SeqVal;
			if (Seq > LastSequenceNumber)
			{
				LastSequenceNumber = Seq;
			}
		}
	}

	// Event name (only present on DISPATCH, op=0)
	FString EventName;
	Root->TryGetStringField(TEXT("t"), EventName);

	// Data field – serialise back to JSON string for Blueprint consumers.
	FString RawDataJson;
	TSharedPtr<FJsonObject> DataObj;
	const TSharedPtr<FJsonValue>* DataValue = Root->Values.Find(TEXT("d"));
	if (DataValue && DataValue->IsValid())
	{
		if ((*DataValue)->Type == EJson::Object)
		{
			DataObj = (*DataValue)->AsObject();
			TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&RawDataJson);
			FJsonSerializer::Serialize(DataObj.ToSharedRef(), Writer);
		}
		else if ((*DataValue)->Type == EJson::Null)
		{
			RawDataJson = TEXT("null");
		}
		else
		{
			// Primitive (bool/number/string) – convert to a JSON string manually.
			const TSharedPtr<FJsonValue>& Val = *DataValue;
			switch (Val->Type)
			{
			case EJson::Boolean:
				RawDataJson = Val->AsBool() ? TEXT("true") : TEXT("false");
				break;
			case EJson::Number:
				RawDataJson = FString::SanitizeFloat(Val->AsNumber());
				break;
			case EJson::String:
				// Re-wrap in quotes so consumers receive valid JSON.
				RawDataJson = TEXT("\"") + Val->AsString() + TEXT("\"");
				break;
			default:
				break;
			}
		}
	}

	HandleGatewayPayload(Op, Seq, EventName, DataObj, RawDataJson);
}

void UDiscordGatewayClient::HandleWebSocketError(const FString& Error)
{
	UE_LOG(LogDiscordGateway, Error,
		TEXT("UDiscordGatewayClient: WebSocket error: %s"), *Error);
	StopHeartbeat();
	bIsReady = false;
	OnError.Broadcast(Error);
}

void UDiscordGatewayClient::HandleWebSocketClosed(int32 StatusCode, const FString& Reason)
{
	UE_LOG(LogDiscordGateway, Log,
		TEXT("UDiscordGatewayClient: WebSocket closed (code=%d, reason=%s)"),
		StatusCode, *Reason);
	StopHeartbeat();
	bIsReady = false;
}

// ---------------------------------------------------------------------------
// Gateway protocol
// ---------------------------------------------------------------------------

void UDiscordGatewayClient::HandleGatewayPayload(int32 Op, int32 Seq,
	const FString& EventName, const TSharedPtr<FJsonObject>& DataObj,
	const FString& RawDataJson)
{
	switch (Op)
	{
	// ------------------------------------------------------------------
	// Opcode 0 – DISPATCH: a Gateway event (MESSAGE_CREATE, READY, etc.)
	// ------------------------------------------------------------------
	case 0:
		{
			if (EventName == TEXT("READY"))
			{
				// Extract session_id so we can resume if needed.
				if (DataObj.IsValid())
				{
					DataObj->TryGetStringField(TEXT("session_id"), SessionId);
				}
				bIsReady = true;
				UE_LOG(LogDiscordGateway, Log,
					TEXT("UDiscordGatewayClient: READY (session_id=%s)"), *SessionId);
				OnReady.Broadcast();
			}

			// Always forward all DISPATCH events (including READY) to Blueprint.
			OnDispatch.Broadcast(EventName, RawDataJson);
		}
		break;

	// ------------------------------------------------------------------
	// Opcode 1 – HEARTBEAT: server requests an immediate heartbeat
	// ------------------------------------------------------------------
	case 1:
		UE_LOG(LogDiscordGateway, Verbose,
			TEXT("UDiscordGatewayClient: Server requested immediate heartbeat"));
		SendHeartbeat();
		break;

	// ------------------------------------------------------------------
	// Opcode 7 – RECONNECT: Discord wants us to reconnect
	// ------------------------------------------------------------------
	case 7:
		UE_LOG(LogDiscordGateway, Log,
			TEXT("UDiscordGatewayClient: Discord requested reconnect (opcode 7)"));
		StopHeartbeat();
		bIsReady = false;
		OnReconnect.Broadcast();
		if (WebSocket)
		{
			WebSocket->Close(1000, TEXT("Reconnect requested by server"));
		}
		break;

	// ------------------------------------------------------------------
	// Opcode 9 – INVALID_SESSION
	// ------------------------------------------------------------------
	case 9:
		{
			// The "d" field is a boolean: true = session is resumable.
			bool bResumable = false;
			if (DataObj == nullptr)
			{
				// d was a primitive bool
				// RawDataJson contains "true" or "false"
				bResumable = (RawDataJson == TEXT("true"));
			}

			UE_LOG(LogDiscordGateway, Warning,
				TEXT("UDiscordGatewayClient: Invalid session (resumable=%s). "
				     "Re-identifying after a short delay..."),
				bResumable ? TEXT("true") : TEXT("false"));

			// Per Discord docs: wait 1–5 seconds then re-identify.
			// For simplicity we schedule a re-identify immediately via a one-shot ticker.
			FTSTicker::GetCoreTicker().AddTicker(
				FTickerDelegate::CreateWeakLambda(this,
					[this](float) -> bool
					{
						if (WebSocket && WebSocket->IsConnected())
						{
							SendIdentify();
						}
						return false; // one-shot
					}),
				1.5f /* seconds before firing */);
		}
		break;

	// ------------------------------------------------------------------
	// Opcode 10 – HELLO: sent immediately after the TCP connection opens
	// ------------------------------------------------------------------
	case 10:
		{
			float IntervalMs = 41250.f; // safe fallback per Discord docs
			if (DataObj.IsValid())
			{
				double Val = 41250.0;
				if (DataObj->TryGetNumberField(TEXT("heartbeat_interval"), Val))
				{
					IntervalMs = static_cast<float>(Val);
				}
			}
			const float IntervalSec = IntervalMs / 1000.f;

			UE_LOG(LogDiscordGateway, Log,
				TEXT("UDiscordGatewayClient: HELLO received (heartbeat_interval=%.0f ms). "
				     "Starting heartbeat and sending IDENTIFY."),
				IntervalMs);

			StartHeartbeat(IntervalSec);
			SendIdentify();
		}
		break;

	// ------------------------------------------------------------------
	// Opcode 11 – HEARTBEAT_ACK
	// ------------------------------------------------------------------
	case 11:
		UE_LOG(LogDiscordGateway, Verbose,
			TEXT("UDiscordGatewayClient: Heartbeat acknowledged"));
		bHeartbeatAckReceived = true;
		break;

	default:
		UE_LOG(LogDiscordGateway, Verbose,
			TEXT("UDiscordGatewayClient: Received unknown opcode %d – ignoring"), Op);
		break;
	}
}

// ---------------------------------------------------------------------------

void UDiscordGatewayClient::SendIdentify()
{
	// Build the IDENTIFY payload.
	// Non-privileged intents only: Presence (256), Server Members (2), and
	// Message Content (32768) are intentionally excluded from the default value.
	TSharedRef<FJsonObject> Root = MakeShared<FJsonObject>();
	Root->SetNumberField(TEXT("op"), 2); // IDENTIFY

	TSharedRef<FJsonObject> Data = MakeShared<FJsonObject>();
	Data->SetStringField(TEXT("token"), BotToken);
	Data->SetNumberField(TEXT("intents"), GatewayIntents);

	TSharedRef<FJsonObject> Props = MakeShared<FJsonObject>();
	Props->SetStringField(TEXT("os"),      TEXT("linux"));
	Props->SetStringField(TEXT("browser"), TEXT("SatisfactoryDiscordBot"));
	Props->SetStringField(TEXT("device"),  TEXT("SatisfactoryDiscordBot"));
	Data->SetObjectField(TEXT("properties"), Props);

	Root->SetObjectField(TEXT("d"), Data);

	UE_LOG(LogDiscordGateway, Log,
		TEXT("UDiscordGatewayClient: Sending IDENTIFY (intents=0x%X)"), GatewayIntents);

	SendJson(Root);
}

void UDiscordGatewayClient::SendHeartbeat()
{
	TSharedRef<FJsonObject> Root = MakeShared<FJsonObject>();
	Root->SetNumberField(TEXT("op"), 1); // HEARTBEAT

	// "d" must be the last sequence number, or null if none received yet.
	if (LastSequenceNumber >= 0)
	{
		Root->SetNumberField(TEXT("d"), LastSequenceNumber);
	}
	else
	{
		Root->SetField(TEXT("d"), MakeShared<FJsonValueNull>());
	}

	if (!bHeartbeatAckReceived)
	{
		// Discord docs: if a heartbeat ACK is not received before the next
		// heartbeat, the connection should be treated as a zombie and closed.
		UE_LOG(LogDiscordGateway, Warning,
			TEXT("UDiscordGatewayClient: No heartbeat ACK received before next heartbeat. "
			     "Closing zombie connection."));
		if (WebSocket)
		{
			WebSocket->Close(1001, TEXT("Heartbeat timeout"));
		}
		return;
	}

	bHeartbeatAckReceived = false;
	UE_LOG(LogDiscordGateway, Verbose, TEXT("UDiscordGatewayClient: Sending heartbeat (seq=%d)"),
		LastSequenceNumber);
	SendJson(Root);
}

void UDiscordGatewayClient::SendJson(const TSharedRef<FJsonObject>& Payload)
{
	if (!WebSocket || !WebSocket->IsConnected())
	{
		UE_LOG(LogDiscordGateway, Warning,
			TEXT("UDiscordGatewayClient::SendJson() called while not connected"));
		return;
	}

	FString JsonString;
	TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&JsonString);
	if (!FJsonSerializer::Serialize(Payload, Writer))
	{
		UE_LOG(LogDiscordGateway, Error,
			TEXT("UDiscordGatewayClient::SendJson(): JSON serialization failed"));
		return;
	}

	WebSocket->Send(JsonString);
}

// ---------------------------------------------------------------------------
// Heartbeat ticker
// ---------------------------------------------------------------------------

void UDiscordGatewayClient::StartHeartbeat(float IntervalSeconds)
{
	StopHeartbeat();

	HeartbeatIntervalSeconds = IntervalSeconds;
	HeartbeatElapsedSeconds  = 0.f;
	bHeartbeatAckReceived    = true;

	// Use FTSTicker so the heartbeat works without needing a UWorld reference.
	HeartbeatTickerHandle = FTSTicker::GetCoreTicker().AddTicker(
		FTickerDelegate::CreateWeakLambda(this,
			[this](float DeltaTime) -> bool
			{
				HeartbeatElapsedSeconds += DeltaTime;
				if (HeartbeatElapsedSeconds >= HeartbeatIntervalSeconds)
				{
					HeartbeatElapsedSeconds = 0.f;
					SendHeartbeat();
				}
				return true; // keep ticking
			}),
		0.f /* tick every frame; interval is tracked manually */);
}

void UDiscordGatewayClient::StopHeartbeat()
{
	if (HeartbeatTickerHandle.IsValid())
	{
		FTSTicker::GetCoreTicker().RemoveTicker(HeartbeatTickerHandle);
		HeartbeatTickerHandle.Reset();
	}
	HeartbeatElapsedSeconds = 0.f;
}

// ===========================================================================
// UDiscordGatewayClientBlueprintLibrary
// ===========================================================================

UDiscordGatewayClient* UDiscordGatewayClientBlueprintLibrary::CreateDiscordGatewayClient(
	UObject* WorldContextObject)
{
	return NewObject<UDiscordGatewayClient>(WorldContextObject);
}
