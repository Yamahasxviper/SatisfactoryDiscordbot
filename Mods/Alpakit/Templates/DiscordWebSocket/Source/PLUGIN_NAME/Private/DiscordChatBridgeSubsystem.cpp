// Copyright Epic Games, Inc. All Rights Reserved.

#include "DiscordChatBridgeSubsystem.h"

#include "DiscordBotSettings.h"
#include "FGChatManager.h"
#include "Subsystem/SubsystemActorManager.h"
#include "Dom/JsonObject.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"
#include "TimerManager.h"

DEFINE_LOG_CATEGORY_STATIC(LogDiscordBridge, Log, All);

// ---------------------------------------------------------------------------
// Constructor
// ---------------------------------------------------------------------------

ADiscordChatBridgeSubsystem::ADiscordChatBridgeSubsystem()
	: WebSocket(nullptr)
	, bIdentified(false)
	, ReconnectDelaySec(5.0f)
	, bInjectingMessage(false)
{
	// Only run on the server (or listen-server host) so we don't create
	// multiple relay connections for each connected client.
	ReplicationPolicy = ESubsystemReplicationPolicy::SpawnOnServer;
}

// ---------------------------------------------------------------------------
// Static getter
// ---------------------------------------------------------------------------

ADiscordChatBridgeSubsystem* ADiscordChatBridgeSubsystem::Get(UObject* WorldContext)
{
	UWorld* World = GEngine->GetWorldFromContextObjectChecked(WorldContext);
	USubsystemActorManager* Manager = World->GetSubsystem<USubsystemActorManager>();
	if (!Manager) return nullptr;
	return Manager->GetSubsystemActor<ADiscordChatBridgeSubsystem>();
}

// ---------------------------------------------------------------------------
// Lifecycle
// ---------------------------------------------------------------------------

void ADiscordChatBridgeSubsystem::Init()
{
	Super::Init();
}

void ADiscordChatBridgeSubsystem::BeginPlay()
{
	Super::BeginPlay();

	const UDiscordBotSettings* Settings = GetDefault<UDiscordBotSettings>();

	if (Settings->RelayWebSocketUrl.IsEmpty())
	{
		UE_LOG(LogDiscordBridge, Warning,
			TEXT("DiscordBridge: RelayWebSocketUrl is not configured. "
			     "Set it in Config/DefaultPLUGIN_NAME.ini."));
		return;
	}

	// Bind to the game chat so we can forward player messages to Discord.
	if (AFGChatManager* ChatMgr = AFGChatManager::Get(this))
	{
		ChatMgr->OnChatMessageAdded.AddDynamic(
			this, &ADiscordChatBridgeSubsystem::OnChatMessageAdded);
	}

	Connect();
}

void ADiscordChatBridgeSubsystem::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	// Cancel any pending reconnect timer.
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(ReconnectTimerHandle);
	}

	// Cleanly close the WebSocket before the actor is destroyed.
	if (WebSocket && WebSocket->IsConnected())
	{
		WebSocket->Close(1001, TEXT("Server shutting down"));
	}
	WebSocket = nullptr;

	Super::EndPlay(EndPlayReason);
}

// ---------------------------------------------------------------------------
// Connection management
// ---------------------------------------------------------------------------

void ADiscordChatBridgeSubsystem::Connect()
{
	const UDiscordBotSettings* Settings = GetDefault<UDiscordBotSettings>();

	WebSocket = NewObject<USMLWebSocket>(this);
	WebSocket->OnConnected.AddDynamic(
		this, &ADiscordChatBridgeSubsystem::OnConnected);
	WebSocket->OnConnectionError.AddDynamic(
		this, &ADiscordChatBridgeSubsystem::OnConnectionError);
	WebSocket->OnClosed.AddDynamic(
		this, &ADiscordChatBridgeSubsystem::OnClosed);
	WebSocket->OnMessageReceived.AddDynamic(
		this, &ADiscordChatBridgeSubsystem::OnMessageReceived);

	UE_LOG(LogDiscordBridge, Log, TEXT("DiscordBridge: Connecting to relay at %s"),
		*Settings->RelayWebSocketUrl);

	WebSocket->Connect(Settings->RelayWebSocketUrl);
}

void ADiscordChatBridgeSubsystem::ScheduleReconnect()
{
	UWorld* World = GetWorld();
	if (!World) return;

	UE_LOG(LogDiscordBridge, Log,
		TEXT("DiscordBridge: Will reconnect in %.0f second(s)."), ReconnectDelaySec);

	FTimerDelegate Delegate;
	Delegate.BindUObject(this, &ADiscordChatBridgeSubsystem::Connect);
	World->GetTimerManager().SetTimer(
		ReconnectTimerHandle, Delegate, ReconnectDelaySec, /*bLoop=*/false);

	// Exponential back-off, capped at 60 seconds.
	ReconnectDelaySec = FMath::Min(ReconnectDelaySec * 2.0f, 60.0f);
}

// ---------------------------------------------------------------------------
// WebSocket event handlers
// ---------------------------------------------------------------------------

void ADiscordChatBridgeSubsystem::OnConnected()
{
	UE_LOG(LogDiscordBridge, Log,
		TEXT("DiscordBridge: Connected to relay. Sending identify frame."));

	// Reset back-off on successful connection.
	ReconnectDelaySec = 5.0f;

	// Send the identify frame so the relay knows which bot token and channel
	// to use for this session.
	const UDiscordBotSettings* Settings = GetDefault<UDiscordBotSettings>();

	TSharedRef<FJsonObject> Payload = MakeShared<FJsonObject>();
	Payload->SetStringField(TEXT("type"),       TEXT("identify"));
	Payload->SetStringField(TEXT("token"),      Settings->BotToken);
	Payload->SetStringField(TEXT("channel_id"), Settings->ChannelId);
	SendJson(Payload);

	bIdentified = true;
}

void ADiscordChatBridgeSubsystem::OnConnectionError(const FString& Error)
{
	UE_LOG(LogDiscordBridge, Error,
		TEXT("DiscordBridge: Connection error: %s"), *Error);

	bIdentified = false;
	ScheduleReconnect();
}

void ADiscordChatBridgeSubsystem::OnClosed(int32 StatusCode, const FString& Reason)
{
	UE_LOG(LogDiscordBridge, Log,
		TEXT("DiscordBridge: Connection closed (code=%d reason=%s). Reconnecting."),
		StatusCode, *Reason);

	bIdentified = false;
	ScheduleReconnect();
}

void ADiscordChatBridgeSubsystem::OnMessageReceived(const FString& Message)
{
	const UDiscordBotSettings* Settings = GetDefault<UDiscordBotSettings>();
	if (!Settings->bDiscordToGame) return;

	// Parse the JSON frame from the relay.
	TSharedPtr<FJsonObject> Json;
	TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Message);
	if (!FJsonSerializer::Deserialize(Reader, Json) || !Json.IsValid())
	{
		UE_LOG(LogDiscordBridge, Warning,
			TEXT("DiscordBridge: Could not parse relay message: %s"), *Message);
		return;
	}

	FString MessageType;
	if (!Json->TryGetStringField(TEXT("type"), MessageType))
	{
		return;
	}

	if (MessageType == TEXT("discord_message"))
	{
		FString Author, Text;
		Json->TryGetStringField(TEXT("author"), Author);
		Json->TryGetStringField(TEXT("text"),   Text);

		if (!Author.IsEmpty() && !Text.IsEmpty())
		{
			UE_LOG(LogDiscordBridge, Log,
				TEXT("DiscordBridge: Discord → Game: [%s] %s"), *Author, *Text);
			PostGameChatMessage(Author, Text);
		}
	}
	else if (MessageType == TEXT("identified"))
	{
		UE_LOG(LogDiscordBridge, Log,
			TEXT("DiscordBridge: Relay acknowledged identification."));
	}
}

// ---------------------------------------------------------------------------
// Chat integration
// ---------------------------------------------------------------------------

void ADiscordChatBridgeSubsystem::OnChatMessageAdded()
{
	const UDiscordBotSettings* Settings = GetDefault<UDiscordBotSettings>();
	if (!Settings->bGameToDiscord) return;
	if (!bIdentified) return;

	// Avoid echoing messages that we injected ourselves from Discord.
	if (bInjectingMessage) return;

	AFGChatManager* ChatMgr = AFGChatManager::Get(this);
	if (!ChatMgr) return;

	TArray<FChatMessageStruct> Messages;
	ChatMgr->GetReceivedChatMessages(Messages);
	if (Messages.IsEmpty()) return;

	const FChatMessageStruct& Latest = Messages.Last();

	// Only bridge player-authored chat messages, not system/ADA messages.
	if (Latest.MessageType != EFGChatMessageType::CMT_PlayerMessage) return;

	const FString SenderName  = Latest.MessageSender.ToString();
	const FString MessageText = Latest.MessageText.ToString();

	UE_LOG(LogDiscordBridge, Log,
		TEXT("DiscordBridge: Game → Discord: [%s] %s"), *SenderName, *MessageText);

	TSharedRef<FJsonObject> Payload = MakeShared<FJsonObject>();
	Payload->SetStringField(TEXT("type"),   TEXT("game_message"));
	Payload->SetStringField(TEXT("player"), SenderName);
	Payload->SetStringField(TEXT("text"),   MessageText);
	SendJson(Payload);
}

void ADiscordChatBridgeSubsystem::SendJson(const TSharedRef<FJsonObject>& Json)
{
	if (!WebSocket || !WebSocket->IsConnected()) return;

	FString Output;
	TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&Output);
	FJsonSerializer::Serialize(Json, Writer);
	WebSocket->Send(Output);
}

void ADiscordChatBridgeSubsystem::PostGameChatMessage(
	const FString& Sender, const FString& Text)
{
	AFGChatManager* ChatMgr = AFGChatManager::Get(this);
	if (!ChatMgr) return;

	const UDiscordBotSettings* Settings = GetDefault<UDiscordBotSettings>();
	const FString DisplayName = Settings->DiscordPrefix.IsEmpty()
		? Sender
		: FString::Printf(TEXT("%s %s"), *Settings->DiscordPrefix, *Sender);

	FChatMessageStruct Msg;
	Msg.MessageType        = EFGChatMessageType::CMT_PlayerMessage;
	Msg.MessageSender      = FText::FromString(DisplayName);
	Msg.MessageText        = FText::FromString(Text);
	// Use a Discord-esque blue so players can distinguish Discord messages.
	Msg.MessageSenderColor = FLinearColor(0.4f, 0.6f, 1.0f);

	bInjectingMessage = true;
	ChatMgr->BroadcastChatMessage(Msg);
	bInjectingMessage = false;
}
