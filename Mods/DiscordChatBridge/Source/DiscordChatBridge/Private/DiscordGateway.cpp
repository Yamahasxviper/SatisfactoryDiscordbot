// Copyright (c) 2024 Discord Chat Bridge Contributors

#include "DiscordGateway.h"
#include "WebSocketsModule.h"
#include "IWebSocket.h"
#include "Json.h"
#include "JsonUtilities.h"
#include "TimerManager.h"
#include "Engine/World.h"

UDiscordGateway::UDiscordGateway()
	: LastSequenceNumber(-1)
	, HeartbeatIntervalMs(0.0f)
	, ConnectionState(EGatewayConnectionState::Disconnected)
	, bReceivedHeartbeatAck(true)
{
}

UDiscordGateway::~UDiscordGateway()
{
	Disconnect();
}

void UDiscordGateway::Initialize(const FString& InBotToken)
{
	BotToken = InBotToken;
	UE_LOG(LogTemp, Log, TEXT("DiscordGateway: Initialized"));
}

void UDiscordGateway::Connect()
{
	if (ConnectionState != EGatewayConnectionState::Disconnected)
	{
		UE_LOG(LogTemp, Warning, TEXT("DiscordGateway: Already connecting or connected"));
		return;
	}

	if (BotToken.IsEmpty())
	{
		UE_LOG(LogTemp, Error, TEXT("DiscordGateway: Cannot connect - bot token is empty"));
		return;
	}

	UE_LOG(LogTemp, Log, TEXT("DiscordGateway: Connecting to Discord Gateway..."));
	ConnectionState = EGatewayConnectionState::Connecting;

	// Create WebSocket connection
	WebSocket = FWebSocketsModule::Get().CreateWebSocket(GATEWAY_URL, TEXT(""));

	// Bind event handlers
	WebSocket->OnConnected().AddUObject(this, &UDiscordGateway::OnWebSocketConnected);
	WebSocket->OnConnectionError().AddUObject(this, &UDiscordGateway::OnWebSocketConnectionError);
	WebSocket->OnClosed().AddUObject(this, &UDiscordGateway::OnWebSocketClosed);
	WebSocket->OnMessage().AddUObject(this, &UDiscordGateway::OnWebSocketMessage);

	// Connect
	WebSocket->Connect();
}

void UDiscordGateway::Disconnect()
{
	if (WebSocket.IsValid() && WebSocket->IsConnected())
	{
		UE_LOG(LogTemp, Log, TEXT("DiscordGateway: Disconnecting..."));
		WebSocket->Close();
	}

	StopHeartbeat();
	ConnectionState = EGatewayConnectionState::Disconnected;
	WebSocket.Reset();
}

void UDiscordGateway::UpdatePresence(const FString& ActivityName, int32 ActivityType)
{
	if (!IsConnected())
	{
		UE_LOG(LogTemp, Warning, TEXT("DiscordGateway: Cannot update presence - not connected"));
		return;
	}

	SendPresenceUpdate(ActivityName, ActivityType);
}

void UDiscordGateway::OnWebSocketConnected()
{
	UE_LOG(LogTemp, Log, TEXT("DiscordGateway: WebSocket connected, waiting for HELLO..."));
}

void UDiscordGateway::OnWebSocketConnectionError(const FString& Error)
{
	UE_LOG(LogTemp, Error, TEXT("DiscordGateway: Connection error: %s"), *Error);
	ConnectionState = EGatewayConnectionState::Disconnected;
	
	if (OnDisconnected.IsBound())
	{
		OnDisconnected.Execute(Error);
	}
}

void UDiscordGateway::OnWebSocketClosed(int32 StatusCode, const FString& Reason, bool bWasClean)
{
	UE_LOG(LogTemp, Warning, TEXT("DiscordGateway: Connection closed - Code: %d, Reason: %s, Clean: %d"), 
		StatusCode, *Reason, bWasClean);
	
	StopHeartbeat();
	ConnectionState = EGatewayConnectionState::Disconnected;

	if (OnDisconnected.IsBound())
	{
		OnDisconnected.Execute(Reason);
	}
}

void UDiscordGateway::OnWebSocketMessage(const FString& Message)
{
	// Parse JSON message
	TSharedPtr<FJsonObject> JsonObject;
	TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Message);

	if (!FJsonSerializer::Deserialize(Reader, JsonObject) || !JsonObject.IsValid())
	{
		UE_LOG(LogTemp, Error, TEXT("DiscordGateway: Failed to parse message: %s"), *Message);
		return;
	}

	HandleGatewayMessage(JsonObject);
}

void UDiscordGateway::HandleGatewayMessage(const TSharedPtr<FJsonObject>& JsonObject)
{
	// Get opcode
	int32 Opcode = JsonObject->GetIntegerField(TEXT("op"));
	
	// Get sequence number if present
	if (JsonObject->HasField(TEXT("s")) && !JsonObject->TryGetField(TEXT("s"))->IsNull())
	{
		LastSequenceNumber = JsonObject->GetIntegerField(TEXT("s"));
	}

	// Get event name and data
	FString EventName;
	JsonObject->TryGetStringField(TEXT("t"), EventName);

	UE_LOG(LogTemp, Verbose, TEXT("DiscordGateway: Received opcode %d, event: %s"), Opcode, *EventName);

	// Handle based on opcode
	switch (static_cast<EDiscordGatewayOpcode>(Opcode))
	{
	case EDiscordGatewayOpcode::Hello:
		{
			TSharedPtr<FJsonObject> Data = JsonObject->GetObjectField(TEXT("d"));
			if (Data.IsValid())
			{
				HandleHello(Data);
			}
			else
			{
				UE_LOG(LogTemp, Warning, TEXT("DiscordGateway: Missing data field in HELLO message"));
			}
		}
		break;

	case EDiscordGatewayOpcode::HeartbeatAck:
		HandleHeartbeatAck();
		break;

	case EDiscordGatewayOpcode::Dispatch:
		if (EventName == TEXT("READY"))
		{
			TSharedPtr<FJsonObject> Data = JsonObject->GetObjectField(TEXT("d"));
			if (Data.IsValid())
			{
				HandleReady(Data);
			}
			else
			{
				UE_LOG(LogTemp, Warning, TEXT("DiscordGateway: Missing data field in READY message"));
			}
		}
		break;

	case EDiscordGatewayOpcode::InvalidSession:
		{
			bool CanResume = JsonObject->GetBoolField(TEXT("d"));
			HandleInvalidSession(CanResume);
		}
		break;

	case EDiscordGatewayOpcode::Reconnect:
		HandleReconnect();
		break;

	default:
		UE_LOG(LogTemp, Verbose, TEXT("DiscordGateway: Unhandled opcode: %d"), Opcode);
		break;
	}
}

void UDiscordGateway::HandleHello(const TSharedPtr<FJsonObject>& Data)
{
	HeartbeatIntervalMs = Data->GetNumberField(TEXT("heartbeat_interval"));
	UE_LOG(LogTemp, Log, TEXT("DiscordGateway: Received HELLO, heartbeat interval: %.0f ms"), HeartbeatIntervalMs);

	// Start heartbeat
	StartHeartbeat();

	// Send IDENTIFY
	ConnectionState = EGatewayConnectionState::Identifying;
	SendIdentify();
}

void UDiscordGateway::HandleReady(const TSharedPtr<FJsonObject>& Data)
{
	SessionId = Data->GetStringField(TEXT("session_id"));
	UE_LOG(LogTemp, Log, TEXT("DiscordGateway: READY received, Session ID: %s"), *SessionId);

	ConnectionState = EGatewayConnectionState::Connected;

	if (OnConnected.IsBound())
	{
		OnConnected.Execute();
	}
}

void UDiscordGateway::HandleHeartbeatAck()
{
	bReceivedHeartbeatAck = true;
	UE_LOG(LogTemp, Verbose, TEXT("DiscordGateway: Heartbeat ACK received"));
}

void UDiscordGateway::HandleInvalidSession(bool CanResume)
{
	UE_LOG(LogTemp, Warning, TEXT("DiscordGateway: Invalid session, can resume: %d"), CanResume);

	if (CanResume && !SessionId.IsEmpty())
	{
		ConnectionState = EGatewayConnectionState::Reconnecting;
		SendResume();
	}
	else
	{
		// Must re-identify
		ConnectionState = EGatewayConnectionState::Identifying;
		SessionId.Empty();
		LastSequenceNumber = -1;
		SendIdentify();
	}
}

void UDiscordGateway::HandleReconnect()
{
	UE_LOG(LogTemp, Warning, TEXT("DiscordGateway: Server requested reconnect"));
	
	// Close current connection and reconnect
	Disconnect();
	
	// Schedule reconnect
	UWorld* World = GetWorld();
	if (World)
	{
		FTimerHandle ReconnectTimer;
		World->GetTimerManager().SetTimer(ReconnectTimer, [this]()
		{
			Connect();
		}, 1.0f, false);
	}
}

void UDiscordGateway::SendIdentify()
{
	UE_LOG(LogTemp, Log, TEXT("DiscordGateway: Sending IDENTIFY..."));

	TSharedPtr<FJsonObject> Payload = MakeShareable(new FJsonObject());
	Payload->SetNumberField(TEXT("op"), static_cast<int32>(EDiscordGatewayOpcode::Identify));

	TSharedPtr<FJsonObject> Data = MakeShareable(new FJsonObject());
	Data->SetStringField(TEXT("token"), BotToken);

	// Properties
	TSharedPtr<FJsonObject> Properties = MakeShareable(new FJsonObject());
	Properties->SetStringField(TEXT("$os"), TEXT("linux"));
	Properties->SetStringField(TEXT("$browser"), TEXT("satisfactory-discord-bridge"));
	Properties->SetStringField(TEXT("$device"), TEXT("satisfactory-discord-bridge"));
	Data->SetObjectField(TEXT("properties"), Properties);

	// Intents (we only need GUILD_MESSAGES for now)
	Data->SetNumberField(TEXT("intents"), 512); // GUILD_MESSAGES = 1 << 9

	Payload->SetObjectField(TEXT("d"), Data);

	SendPayload(Payload);
}

void UDiscordGateway::SendHeartbeat()
{
	TSharedPtr<FJsonObject> Payload = MakeShareable(new FJsonObject());
	Payload->SetNumberField(TEXT("op"), static_cast<int32>(EDiscordGatewayOpcode::Heartbeat));

	if (LastSequenceNumber >= 0)
	{
		Payload->SetNumberField(TEXT("d"), LastSequenceNumber);
	}
	else
	{
		Payload->SetField(TEXT("d"), MakeShareable(new FJsonValueNull()));
	}

	UE_LOG(LogTemp, Verbose, TEXT("DiscordGateway: Sending heartbeat (seq: %d)"), LastSequenceNumber);
	SendPayload(Payload);

	// Check if we received ACK for previous heartbeat
	if (!bReceivedHeartbeatAck)
	{
		UE_LOG(LogTemp, Warning, TEXT("DiscordGateway: Did not receive heartbeat ACK, reconnecting..."));
		Disconnect();
		Connect();
	}

	bReceivedHeartbeatAck = false;
}

void UDiscordGateway::SendPresenceUpdate(const FString& ActivityName, int32 ActivityType)
{
	UE_LOG(LogTemp, Log, TEXT("DiscordGateway: Updating presence - Activity: %s, Type: %d"), *ActivityName, ActivityType);

	TSharedPtr<FJsonObject> Payload = MakeShareable(new FJsonObject());
	Payload->SetNumberField(TEXT("op"), static_cast<int32>(EDiscordGatewayOpcode::PresenceUpdate));

	TSharedPtr<FJsonObject> Data = MakeShareable(new FJsonObject());
	Data->SetField(TEXT("since"), MakeShareable(new FJsonValueNull()));
	Data->SetStringField(TEXT("status"), TEXT("online"));
	Data->SetBoolField(TEXT("afk"), false);

	// Activities array
	TArray<TSharedPtr<FJsonValue>> Activities;
	TSharedPtr<FJsonObject> Activity = MakeShareable(new FJsonObject());
	Activity->SetStringField(TEXT("name"), ActivityName);
	Activity->SetNumberField(TEXT("type"), ActivityType); // 0 = Playing, 1 = Streaming, 2 = Listening, 3 = Watching, 5 = Competing
	Activities.Add(MakeShareable(new FJsonValueObject(Activity)));

	Data->SetArrayField(TEXT("activities"), Activities);

	Payload->SetObjectField(TEXT("d"), Data);

	SendPayload(Payload);
}

void UDiscordGateway::SendResume()
{
	UE_LOG(LogTemp, Log, TEXT("DiscordGateway: Sending RESUME..."));

	TSharedPtr<FJsonObject> Payload = MakeShareable(new FJsonObject());
	Payload->SetNumberField(TEXT("op"), static_cast<int32>(EDiscordGatewayOpcode::Resume));

	TSharedPtr<FJsonObject> Data = MakeShareable(new FJsonObject());
	Data->SetStringField(TEXT("token"), BotToken);
	Data->SetStringField(TEXT("session_id"), SessionId);
	Data->SetNumberField(TEXT("seq"), LastSequenceNumber);

	Payload->SetObjectField(TEXT("d"), Data);

	SendPayload(Payload);
}

void UDiscordGateway::SendPayload(const TSharedPtr<FJsonObject>& Payload)
{
	if (!WebSocket.IsValid() || !WebSocket->IsConnected())
	{
		UE_LOG(LogTemp, Warning, TEXT("DiscordGateway: Cannot send payload - not connected"));
		return;
	}

	FString JsonString;
	TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&JsonString);
	
	if (FJsonSerializer::Serialize(Payload.ToSharedRef(), Writer))
	{
		WebSocket->Send(JsonString);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("DiscordGateway: Failed to serialize payload"));
	}
}

void UDiscordGateway::StartHeartbeat()
{
	if (HeartbeatIntervalMs <= 0)
	{
		UE_LOG(LogTemp, Error, TEXT("DiscordGateway: Invalid heartbeat interval"));
		return;
	}

	StopHeartbeat();

	UWorld* World = GetWorld();
	if (World)
	{
		float IntervalSeconds = HeartbeatIntervalMs / 1000.0f;
		
		// Add jitter to initial heartbeat (Discord recommends this)
		float InitialDelay = FMath::RandRange(0.0f, IntervalSeconds);

		World->GetTimerManager().SetTimer(
			HeartbeatTimerHandle,
			this,
			&UDiscordGateway::OnHeartbeatTimer,
			IntervalSeconds,
			true,
			InitialDelay
		);

		UE_LOG(LogTemp, Log, TEXT("DiscordGateway: Heartbeat started (interval: %.1f seconds, initial delay: %.1f seconds)"), 
			IntervalSeconds, InitialDelay);
	}
}

void UDiscordGateway::StopHeartbeat()
{
	UWorld* World = GetWorld();
	if (World && HeartbeatTimerHandle.IsValid())
	{
		World->GetTimerManager().ClearTimer(HeartbeatTimerHandle);
		HeartbeatTimerHandle.Invalidate();
		UE_LOG(LogTemp, Verbose, TEXT("DiscordGateway: Heartbeat stopped"));
	}
}

void UDiscordGateway::OnHeartbeatTimer()
{
	SendHeartbeat();
}
