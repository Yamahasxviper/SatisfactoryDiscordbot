// Copyright Epic Games, Inc. All Rights Reserved.

#include "DiscordGatewayWebSocket.h"

#include "Dom/JsonObject.h"
#include "Dom/JsonValue.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"
#include "Serialization/JsonWriter.h"
#include "Engine/Engine.h"

DEFINE_LOG_CATEGORY_STATIC(LogDiscordGateway, Log, All);

// ---------------------------------------------------------------------------
// Discord Gateway op-codes
// https://discord.com/developers/docs/topics/opcodes-and-status-codes
// ---------------------------------------------------------------------------

namespace EDiscordGatewayOpcode
{
	enum Type : int32
	{
		Dispatch            = 0,  ///< Server → Client: event dispatch
		Heartbeat           = 1,  ///< Client → Server: heartbeat
		Identify            = 2,  ///< Client → Server: identify
		PresenceUpdate      = 3,  ///< Client → Server: update presence
		VoiceStateUpdate    = 4,  ///< Client → Server: voice state update
		Resume              = 6,  ///< Client → Server: resume session
		Reconnect           = 7,  ///< Server → Client: please reconnect
		RequestGuildMembers = 8,  ///< Client → Server: request guild members
		InvalidSession      = 9,  ///< Server → Client: session invalidated
		Hello               = 10, ///< Server → Client: hello (with heartbeat_interval)
		HeartbeatAck        = 11, ///< Server → Client: heartbeat acknowledged
	};
}

// ---------------------------------------------------------------------------
// USMLWebSocketBlueprintLibrary  –  static factory
// ---------------------------------------------------------------------------

UDiscordGatewayWebSocket* UDiscordGatewayWebSocket::CreateDiscordGateway(
	UObject* WorldContextObject)
{
	return NewObject<UDiscordGatewayWebSocket>(WorldContextObject);
}

// ---------------------------------------------------------------------------
// Constructor / destructor
// ---------------------------------------------------------------------------

UDiscordGatewayWebSocket::UDiscordGatewayWebSocket()
	: WebSocket(nullptr)
	, GatewayIntents(0)
	, SequenceNumber(-1)
	, bIdentified(false)
{}

void UDiscordGatewayWebSocket::BeginDestroy()
{
	StopHeartbeat();
	if (WebSocket)
	{
		WebSocket->Close();
	}
	Super::BeginDestroy();
}

// ---------------------------------------------------------------------------
// Connection API
// ---------------------------------------------------------------------------

void UDiscordGatewayWebSocket::ConnectToDiscord(
	const FString& BotToken,
	int32          Intents,
	const FString& GatewayUrl)
{
	if (WebSocket && WebSocket->IsConnected())
	{
		UE_LOG(LogDiscordGateway, Warning,
			TEXT("ConnectToDiscord() called while already connected. Call Disconnect() first."));
		return;
	}

	// Normalise token – Discord expects the "Bot " prefix for bot accounts.
	Token = BotToken.StartsWith(TEXT("Bot "), ESearchCase::CaseSensitive)
		? BotToken
		: FString::Printf(TEXT("Bot %s"), *BotToken);

	GatewayIntents  = Intents;
	SequenceNumber  = -1;
	bIdentified     = false;

	// Log which privileged intents are requested so developers can verify
	// the corresponding toggles in the Discord Developer Portal.
	if (Intents & static_cast<int32>(EDiscordGatewayIntent::GuildPresences))
	{
		UE_LOG(LogDiscordGateway, Log,
			TEXT("Requesting privileged intent: Presence (GuildPresences). "
			     "Ensure it is enabled in the Discord Developer Portal."));
	}
	if (Intents & static_cast<int32>(EDiscordGatewayIntent::GuildMembers))
	{
		UE_LOG(LogDiscordGateway, Log,
			TEXT("Requesting privileged intent: Server Members (GuildMembers). "
			     "Ensure it is enabled in the Discord Developer Portal."));
	}
	if (Intents & static_cast<int32>(EDiscordGatewayIntent::MessageContent))
	{
		UE_LOG(LogDiscordGateway, Log,
			TEXT("Requesting privileged intent: Message Content. "
			     "Ensure it is enabled in the Discord Developer Portal."));
	}

	// Create the underlying WebSocket if we don't have one yet.
	if (!WebSocket)
	{
		WebSocket = NewObject<USMLWebSocket>(this);
		WebSocket->OnConnected.AddDynamic(this,
			&UDiscordGatewayWebSocket::HandleConnected);
		WebSocket->OnConnectionError.AddDynamic(this,
			&UDiscordGatewayWebSocket::HandleConnectionError);
		WebSocket->OnClosed.AddDynamic(this,
			&UDiscordGatewayWebSocket::HandleClosed);
		WebSocket->OnMessageReceived.AddDynamic(this,
			&UDiscordGatewayWebSocket::HandleMessage);
	}

	UE_LOG(LogDiscordGateway, Log,
		TEXT("Connecting to Discord Gateway: %s"), *GatewayUrl);
	WebSocket->Connect(GatewayUrl);
}

void UDiscordGatewayWebSocket::Disconnect()
{
	StopHeartbeat();
	bIdentified    = false;
	SequenceNumber = -1;

	if (WebSocket)
	{
		WebSocket->Close(1000, TEXT("Client disconnecting"));
	}
}

// ---------------------------------------------------------------------------
// USMLWebSocket event handlers
// ---------------------------------------------------------------------------

void UDiscordGatewayWebSocket::HandleConnected()
{
	UE_LOG(LogDiscordGateway, Log,
		TEXT("TCP connection established. Waiting for Discord HELLO..."));
	// Discord sends HELLO immediately after the TCP handshake; we wait for it
	// in HandleMessage() before sending IDENTIFY.
}

void UDiscordGatewayWebSocket::HandleConnectionError(const FString& Error)
{
	UE_LOG(LogDiscordGateway, Error,
		TEXT("Discord Gateway connection error: %s"), *Error);
	StopHeartbeat();
	bIdentified = false;
	OnConnectionError.Broadcast(Error);
}

void UDiscordGatewayWebSocket::HandleClosed(int32 Code, const FString& Reason)
{
	UE_LOG(LogDiscordGateway, Log,
		TEXT("Discord Gateway closed – code=%d reason='%s'"), Code, *Reason);
	StopHeartbeat();
	bIdentified = false;
	OnClosed.Broadcast(Code, Reason);
}

void UDiscordGatewayWebSocket::HandleMessage(const FString& Message)
{
	HandleGatewayPayload(Message);
}

// ---------------------------------------------------------------------------
// Gateway protocol
// ---------------------------------------------------------------------------

void UDiscordGatewayWebSocket::HandleGatewayPayload(const FString& Json)
{
	// Parse the top-level Gateway payload object.
	TSharedPtr<FJsonObject> Root;
	TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Json);
	if (!FJsonSerializer::Deserialize(Reader, Root) || !Root.IsValid())
	{
		UE_LOG(LogDiscordGateway, Warning,
			TEXT("Failed to parse Gateway payload JSON: %s"), *Json.Left(256));
		return;
	}

	// "op" – operation code (always present).
	int32 Op = 0;
	if (!Root->TryGetNumberField(TEXT("op"), Op))
	{
		UE_LOG(LogDiscordGateway, Warning, TEXT("Gateway payload missing 'op' field"));
		return;
	}

	// "s" – sequence number (present for op=0 dispatches).
	int32 NewSeq = 0;
	if (Root->TryGetNumberField(TEXT("s"), NewSeq))
	{
		SequenceNumber = NewSeq;
	}

	switch (Op)
	{
	case EDiscordGatewayOpcode::Hello:
		{
			// Extract heartbeat_interval from the "d" object.
			const TSharedPtr<FJsonObject>* DObj = nullptr;
			int32 IntervalMs = 41250; // Discord's typical default
			if (Root->TryGetObjectField(TEXT("d"), DObj) && DObj && DObj->IsValid())
			{
				(*DObj)->TryGetNumberField(TEXT("heartbeat_interval"), IntervalMs);
			}
			HandleHello(IntervalMs);
			break;
		}

	case EDiscordGatewayOpcode::Dispatch:
		{
			FString EventName;
			Root->TryGetStringField(TEXT("t"), EventName);

			const TSharedPtr<FJsonObject>* DObj = nullptr;
			TSharedPtr<FJsonObject> DataObj;
			if (Root->TryGetObjectField(TEXT("d"), DObj) && DObj)
			{
				DataObj = *DObj;
			}

			HandleDispatch(EventName, DataObj);
			break;
		}

	case EDiscordGatewayOpcode::HeartbeatAck:
		HandleHeartbeatAck();
		break;

	case EDiscordGatewayOpcode::Heartbeat:
		// Server requests an immediate heartbeat.
		SendHeartbeat();
		break;

	case EDiscordGatewayOpcode::Reconnect:
		UE_LOG(LogDiscordGateway, Log,
			TEXT("Discord requested reconnect (op=7)."));
		Disconnect();
		break;

	case EDiscordGatewayOpcode::InvalidSession:
		UE_LOG(LogDiscordGateway, Warning,
			TEXT("Discord invalidated the session (op=9). Consider reconnecting."));
		break;

	default:
		UE_LOG(LogDiscordGateway, Verbose,
			TEXT("Unhandled Gateway op-code %d"), Op);
		break;
	}
}

void UDiscordGatewayWebSocket::HandleHello(int32 HeartbeatIntervalMs)
{
	UE_LOG(LogDiscordGateway, Log,
		TEXT("Received HELLO – heartbeat_interval=%d ms"), HeartbeatIntervalMs);

	StartHeartbeat(HeartbeatIntervalMs);
	SendIdentify();
}

void UDiscordGatewayWebSocket::HandleDispatch(
	const FString&                    EventName,
	const TSharedPtr<FJsonObject>& Data)
{
	// Serialise the data object back to a JSON string for the raw event delegate.
	FString DataJson;
	if (Data.IsValid())
	{
		TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&DataJson);
		FJsonSerializer::Serialize(Data.ToSharedRef(), Writer);
	}

	// Fire the catch-all delegate first.
	OnDiscordEvent.Broadcast(EventName, DataJson);

	// ---- Route well-known events ----

	if (EventName == TEXT("READY"))
	{
		FString SessionId;
		FString UserId;
		if (Data.IsValid())
		{
			Data->TryGetStringField(TEXT("session_id"), SessionId);
			const TSharedPtr<FJsonObject>* UserObj = nullptr;
			if (Data->TryGetObjectField(TEXT("user"), UserObj) && UserObj)
			{
				(*UserObj)->TryGetStringField(TEXT("id"), UserId);
			}
		}
		bIdentified = true;
		UE_LOG(LogDiscordGateway, Log,
			TEXT("Discord READY – session_id=%s user_id=%s"), *SessionId, *UserId);
		OnReady.Broadcast(SessionId, UserId);
	}
	else if (EventName == TEXT("PRESENCE_UPDATE"))
	{
		// Requires GuildPresences (Presence Intent).
		FString UserId;
		FString Status;
		if (Data.IsValid())
		{
			const TSharedPtr<FJsonObject>* UserObj = nullptr;
			if (Data->TryGetObjectField(TEXT("user"), UserObj) && UserObj)
			{
				(*UserObj)->TryGetStringField(TEXT("id"), UserId);
			}
			Data->TryGetStringField(TEXT("status"), Status);
		}
		OnPresenceUpdate.Broadcast(UserId, Status);
	}
	else if (EventName == TEXT("GUILD_MEMBER_ADD"))
	{
		// Requires GuildMembers (Server Members Intent).
		FString GuildId;
		FString UserId;
		if (Data.IsValid())
		{
			Data->TryGetStringField(TEXT("guild_id"), GuildId);
			const TSharedPtr<FJsonObject>* UserObj = nullptr;
			if (Data->TryGetObjectField(TEXT("user"), UserObj) && UserObj)
			{
				(*UserObj)->TryGetStringField(TEXT("id"), UserId);
			}
		}
		OnGuildMemberAdd.Broadcast(GuildId, UserId);
	}
	else if (EventName == TEXT("MESSAGE_CREATE"))
	{
		// Requires GuildMessages intent; content requires MessageContent intent.
		FString ChannelId;
		FString AuthorId;
		FString Content;
		if (Data.IsValid())
		{
			Data->TryGetStringField(TEXT("channel_id"), ChannelId);
			Data->TryGetStringField(TEXT("content"),    Content);
			const TSharedPtr<FJsonObject>* AuthorObj = nullptr;
			if (Data->TryGetObjectField(TEXT("author"), AuthorObj) && AuthorObj)
			{
				(*AuthorObj)->TryGetStringField(TEXT("id"), AuthorId);
			}
		}
		OnMessageCreate.Broadcast(ChannelId, AuthorId, Content);
	}
}

void UDiscordGatewayWebSocket::HandleHeartbeatAck()
{
	UE_LOG(LogDiscordGateway, Verbose, TEXT("Heartbeat acknowledged (op=11)"));
}

// ---------------------------------------------------------------------------
// Send helpers
// ---------------------------------------------------------------------------

void UDiscordGatewayWebSocket::SendIdentify()
{
	if (!WebSocket || !WebSocket->IsConnected())
	{
		UE_LOG(LogDiscordGateway, Warning,
			TEXT("SendIdentify() called but WebSocket is not connected"));
		return;
	}

	// Build the IDENTIFY payload (op=2).
	// https://discord.com/developers/docs/topics/gateway#identifying
	TSharedPtr<FJsonObject> PropsObj = MakeShared<FJsonObject>();
	PropsObj->SetStringField(TEXT("os"),      TEXT("windows"));
	PropsObj->SetStringField(TEXT("browser"), TEXT("satisfactory_mod"));
	PropsObj->SetStringField(TEXT("device"),  TEXT("satisfactory_mod"));

	TSharedPtr<FJsonObject> DataObj = MakeShared<FJsonObject>();
	DataObj->SetStringField(TEXT("token"),   Token);
	DataObj->SetNumberField(TEXT("intents"), static_cast<double>(GatewayIntents));
	DataObj->SetObjectField(TEXT("properties"), PropsObj);

	TSharedPtr<FJsonObject> Payload = MakeShared<FJsonObject>();
	Payload->SetNumberField(TEXT("op"), static_cast<double>(EDiscordGatewayOpcode::Identify));
	Payload->SetObjectField(TEXT("d"), DataObj);

	FString JsonStr;
	TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&JsonStr);
	FJsonSerializer::Serialize(Payload.ToSharedRef(), Writer);

	UE_LOG(LogDiscordGateway, Log,
		TEXT("Sending IDENTIFY with intents=%d"), GatewayIntents);
	WebSocket->Send(JsonStr);
}

void UDiscordGatewayWebSocket::SendHeartbeat()
{
	if (!WebSocket || !WebSocket->IsConnected())
	{
		return;
	}

	// op=1, d=<sequence number or null>
	TSharedPtr<FJsonObject> Payload = MakeShared<FJsonObject>();
	Payload->SetNumberField(TEXT("op"), static_cast<double>(EDiscordGatewayOpcode::Heartbeat));
	if (SequenceNumber >= 0)
	{
		Payload->SetNumberField(TEXT("d"), static_cast<double>(SequenceNumber));
	}
	else
	{
		Payload->SetField(TEXT("d"), MakeShared<FJsonValueNull>());
	}

	FString JsonStr;
	TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&JsonStr);
	FJsonSerializer::Serialize(Payload.ToSharedRef(), Writer);

	UE_LOG(LogDiscordGateway, Verbose,
		TEXT("Sending HEARTBEAT (seq=%d)"), SequenceNumber);
	WebSocket->Send(JsonStr);
}

// ---------------------------------------------------------------------------
// Heartbeat timer
// ---------------------------------------------------------------------------

void UDiscordGatewayWebSocket::StartHeartbeat(int32 IntervalMs)
{
	StopHeartbeat();

	UWorld* World = GetWorld();
	if (!World)
	{
		// If there is no world context (e.g. during testing or editor use),
		// fall back to GEngine's first world.
		if (GEngine)
		{
			for (const FWorldContext& Ctx : GEngine->GetWorldContexts())
			{
				if (Ctx.World())
				{
					World = Ctx.World();
					break;
				}
			}
		}
	}

	if (!World)
	{
		UE_LOG(LogDiscordGateway, Warning,
			TEXT("StartHeartbeat: no UWorld available – heartbeat timer not started. "
			     "Ensure the outer WorldContextObject is valid."));
		return;
	}

	const float IntervalSec = static_cast<float>(IntervalMs) / 1000.0f;
	World->GetTimerManager().SetTimer(
		HeartbeatTimerHandle,
		this, &UDiscordGatewayWebSocket::SendHeartbeat,
		IntervalSec,
		/* bLoop = */ true,
		/* FirstDelay = */ IntervalSec);

	UE_LOG(LogDiscordGateway, Log,
		TEXT("Heartbeat timer started – interval=%.2f s"), IntervalSec);
}

void UDiscordGatewayWebSocket::StopHeartbeat()
{
	if (!HeartbeatTimerHandle.IsValid())
	{
		return;
	}

	UWorld* World = GetWorld();
	if (!World && GEngine)
	{
		for (const FWorldContext& Ctx : GEngine->GetWorldContexts())
		{
			if (Ctx.World())
			{
				World = Ctx.World();
				break;
			}
		}
	}

	if (World)
	{
		World->GetTimerManager().ClearTimer(HeartbeatTimerHandle);
	}
	HeartbeatTimerHandle.Invalidate();
}
