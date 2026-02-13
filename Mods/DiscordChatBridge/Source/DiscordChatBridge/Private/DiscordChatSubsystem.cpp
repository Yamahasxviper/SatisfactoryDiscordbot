// Copyright (c) 2024 Discord Chat Bridge Contributors

#include "DiscordChatSubsystem.h"
#include "FGChatManager.h"
#include "Misc/ConfigCacheIni.h"
#include "Engine/World.h"

ADiscordChatSubsystem::ADiscordChatSubsystem()
	: LastProcessedMessageIndex(0)
{
	ReplicationPolicy = ESubsystemReplicationPolicy::SpawnOnServer;
}

void ADiscordChatSubsystem::Init()
{
	Super::Init();
	
	UE_LOG(LogTemp, Log, TEXT("DiscordChatSubsystem: Initializing"));
	
	// Load configuration
	LoadConfiguration();
	
	// Create Discord API instance
	DiscordAPI = NewObject<UDiscordAPI>(this);
	if (DiscordAPI)
	{
		DiscordAPI->Initialize(BotConfig);
		DiscordAPI->OnMessageReceived.BindUObject(this, &ADiscordChatSubsystem::OnDiscordMessageReceived);
	}
}

void ADiscordChatSubsystem::BeginPlay()
{
	Super::BeginPlay();
	
	// Get reference to chat manager
	ChatManager = AFGChatManager::Get(GetWorld());
	
	if (ChatManager)
	{
		// Bind to chat message event
		ChatManager->OnChatMessageAdded.AddDynamic(this, &ADiscordChatSubsystem::OnGameChatMessageAdded);
		UE_LOG(LogTemp, Log, TEXT("DiscordChatSubsystem: Bound to chat manager"));
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("DiscordChatSubsystem: Failed to get chat manager"));
	}
	
	// Start polling Discord for messages
	if (DiscordAPI && DiscordAPI->IsInitialized())
	{
		DiscordAPI->StartPolling();
		
		// Send server start notification if enabled
		if (BotConfig.bEnableServerNotifications)
		{
			UE_LOG(LogTemp, Log, TEXT("DiscordChatSubsystem: Sending server start notification"));
			DiscordAPI->SendNotification(BotConfig.ServerStartMessage);
		}
	}
}

void ADiscordChatSubsystem::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	// Send server stop notification if enabled
	if (DiscordAPI && DiscordAPI->IsInitialized() && BotConfig.bEnableServerNotifications)
	{
		UE_LOG(LogTemp, Log, TEXT("DiscordChatSubsystem: Sending server stop notification"));
		DiscordAPI->SendNotification(BotConfig.ServerStopMessage);
		
		// Give the HTTP request a moment to complete before stopping
		// Note: In a real scenario, you might want to wait for the response,
		// but for simplicity we'll just add a small delay
		FPlatformProcess::Sleep(0.5f);
	}
	
	// Stop polling
	if (DiscordAPI)
	{
		DiscordAPI->StopPolling();
	}
	
	// Unbind from chat manager
	if (ChatManager)
	{
		ChatManager->OnChatMessageAdded.RemoveDynamic(this, &ADiscordChatSubsystem::OnGameChatMessageAdded);
	}
	
	Super::EndPlay(EndPlayReason);
}

void ADiscordChatSubsystem::LoadConfiguration()
{
	UE_LOG(LogTemp, Log, TEXT("DiscordChatSubsystem: Loading configuration"));
	
	// Try to load from config file
	FString ConfigSection = TEXT("/Script/DiscordChatBridge.DiscordChatSubsystem");
	
	if (GConfig)
	{
		FString BotToken;
		FString ChannelId;
		float PollInterval = 2.0f;
		FString DiscordNameFormat;
		FString GameNameFormat;
		FString DiscordSourceLabel;
		FString GameSourceLabel;
		bool bEnableServerNotifications = false;
		FString NotificationChannelId;
		FString ServerStartMessage;
		FString ServerStopMessage;
		
		// Load settings from Config/DefaultDiscordChatBridge.ini
		GConfig->GetString(*ConfigSection, TEXT("BotToken"), BotToken, GGameIni);
		GConfig->GetString(*ConfigSection, TEXT("ChannelId"), ChannelId, GGameIni);
		GConfig->GetFloat(*ConfigSection, TEXT("PollIntervalSeconds"), PollInterval, GGameIni);
		GConfig->GetString(*ConfigSection, TEXT("DiscordNameFormat"), DiscordNameFormat, GGameIni);
		GConfig->GetString(*ConfigSection, TEXT("GameNameFormat"), GameNameFormat, GGameIni);
		GConfig->GetString(*ConfigSection, TEXT("DiscordSourceLabel"), DiscordSourceLabel, GGameIni);
		GConfig->GetString(*ConfigSection, TEXT("GameSourceLabel"), GameSourceLabel, GGameIni);
		GConfig->GetBool(*ConfigSection, TEXT("EnableServerNotifications"), bEnableServerNotifications, GGameIni);
		GConfig->GetString(*ConfigSection, TEXT("NotificationChannelId"), NotificationChannelId, GGameIni);
		GConfig->GetString(*ConfigSection, TEXT("ServerStartMessage"), ServerStartMessage, GGameIni);
		GConfig->GetString(*ConfigSection, TEXT("ServerStopMessage"), ServerStopMessage, GGameIni);
		
		if (!BotToken.IsEmpty() && !ChannelId.IsEmpty())
		{
			BotConfig.BotToken = BotToken;
			BotConfig.ChannelId = ChannelId;
			BotConfig.PollIntervalSeconds = PollInterval;
			
			// Use custom formats if provided, otherwise use defaults
			if (!DiscordNameFormat.IsEmpty())
			{
				BotConfig.DiscordNameFormat = DiscordNameFormat;
			}
			if (!GameNameFormat.IsEmpty())
			{
				BotConfig.GameNameFormat = GameNameFormat;
			}
			if (!DiscordSourceLabel.IsEmpty())
			{
				BotConfig.DiscordSourceLabel = DiscordSourceLabel;
			}
			if (!GameSourceLabel.IsEmpty())
			{
				BotConfig.GameSourceLabel = GameSourceLabel;
			}
			
			// Load server notification settings
			BotConfig.bEnableServerNotifications = bEnableServerNotifications;
			if (!NotificationChannelId.IsEmpty())
			{
				BotConfig.NotificationChannelId = NotificationChannelId;
			}
			if (!ServerStartMessage.IsEmpty())
			{
				BotConfig.ServerStartMessage = ServerStartMessage;
			}
			if (!ServerStopMessage.IsEmpty())
			{
				BotConfig.ServerStopMessage = ServerStopMessage;
			}
			
			UE_LOG(LogTemp, Log, TEXT("DiscordChatSubsystem: Configuration loaded - Channel ID: %s, Poll Interval: %.1fs, Notifications: %s"), 
				*ChannelId, PollInterval, bEnableServerNotifications ? TEXT("Enabled") : TEXT("Disabled"));
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("DiscordChatSubsystem: Configuration incomplete - please set BotToken and ChannelId in Config/DefaultDiscordChatBridge.ini"));
		}
	}
}

void ADiscordChatSubsystem::OnGameChatMessageAdded()
{
	if (!ChatManager || !DiscordAPI || !DiscordAPI->IsInitialized())
	{
		return;
	}
	
	// Get all received messages
	TArray<FChatMessageStruct> Messages;
	ChatManager->GetReceivedChatMessages(Messages);
	
	// Process new messages
	for (int32 i = LastProcessedMessageIndex; i < Messages.Num(); i++)
	{
		const FChatMessageStruct& Message = Messages[i];
		
		// Only forward player messages to Discord
		if (Message.MessageType == EFGChatMessageType::CMT_PlayerMessage)
		{
			ForwardGameMessageToDiscord(Message);
		}
	}
	
	// Update the index
	LastProcessedMessageIndex = Messages.Num();
}

void ADiscordChatSubsystem::ForwardGameMessageToDiscord(const FChatMessageStruct& Message)
{
	if (!DiscordAPI || !DiscordAPI->IsInitialized())
	{
		return;
	}
	
	FString Username = Message.MessageSender.ToString();
	FString MessageText = Message.MessageText.ToString();
	
	if (Username.IsEmpty())
	{
		Username = TEXT("Unknown Player");
	}
	
	UE_LOG(LogTemp, Verbose, TEXT("DiscordChatSubsystem: Forwarding message to Discord from %s: %s"), 
		*Username, *MessageText);
	
	DiscordAPI->SendMessage(Username, MessageText);
}

void ADiscordChatSubsystem::OnDiscordMessageReceived(const FString& Username, const FString& Message)
{
	UE_LOG(LogTemp, Log, TEXT("DiscordChatSubsystem: Received Discord message from %s: %s"), 
		*Username, *Message);
	
	ForwardDiscordMessageToGame(Username, Message);
}

void ADiscordChatSubsystem::ForwardDiscordMessageToGame(const FString& Username, const FString& Message)
{
	if (!ChatManager)
	{
		return;
	}
	
	// Format the sender name using the configured format
	FString FormattedSender = BotConfig.DiscordNameFormat;
	FormattedSender = FormattedSender.Replace(TEXT("{source}"), *BotConfig.DiscordSourceLabel);
	FormattedSender = FormattedSender.Replace(TEXT("{username}"), *Username);
	
	// Create a chat message struct
	FChatMessageStruct ChatMessage;
	ChatMessage.MessageType = EFGChatMessageType::CMT_CustomMessage;
	ChatMessage.MessageSender = FText::FromString(FormattedSender);
	ChatMessage.MessageText = FText::FromString(Message);
	ChatMessage.MessageSenderColor = FLinearColor(0.447f, 0.627f, 0.957f); // Discord blurple color
	
	// Broadcast to all players
	ChatManager->BroadcastChatMessage(ChatMessage, nullptr);
	
	UE_LOG(LogTemp, Verbose, TEXT("DiscordChatSubsystem: Forwarded Discord message to game"));
}
