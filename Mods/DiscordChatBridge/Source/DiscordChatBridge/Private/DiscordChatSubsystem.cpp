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
	
	// Load configuration first to get log verbosity
	LoadConfiguration();
	
	if (BotConfig.LogVerbosity >= 3)
	{
		UE_LOG(LogTemp, Log, TEXT("DiscordChatSubsystem: Initializing"));
	}
	
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
		if (BotConfig.LogVerbosity >= 3)
		{
			UE_LOG(LogTemp, Log, TEXT("DiscordChatSubsystem: Bound to chat manager"));
		}
	}
	else
	{
		if (BotConfig.LogVerbosity >= 2)
		{
			UE_LOG(LogTemp, Warning, TEXT("DiscordChatSubsystem: Failed to get chat manager"));
		}
	}
	
	// Start polling Discord for messages
	if (DiscordAPI && DiscordAPI->IsInitialized())
	{
		DiscordAPI->StartPolling();
	}
}

void ADiscordChatSubsystem::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
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
	if (BotConfig.LogVerbosity >= 3)
	{
		UE_LOG(LogTemp, Log, TEXT("DiscordChatSubsystem: Loading configuration"));
	}
	
	// Try to load from config file
	FString ConfigSection = TEXT("/Script/DiscordChatBridge.DiscordChatSubsystem");
	
	if (GConfig)
	{
		FString BotToken;
		FString ChannelId;
		float PollInterval = 2.0f;
		bool bEnableGameToDiscord = true;
		bool bEnableDiscordToGame = true;
		FString GameMessageFormat;
		FString DiscordMessageFormat;
		float ColorR = 0.447f;
		float ColorG = 0.627f;
		float ColorB = 0.957f;
		int32 LogVerbosity = 3;
		
		// Load basic settings from Config/DefaultDiscordChatBridge.ini
		GConfig->GetString(*ConfigSection, TEXT("BotToken"), BotToken, GGameIni);
		GConfig->GetString(*ConfigSection, TEXT("ChannelId"), ChannelId, GGameIni);
		GConfig->GetFloat(*ConfigSection, TEXT("PollIntervalSeconds"), PollInterval, GGameIni);
		
		// Load new configuration settings
		GConfig->GetBool(*ConfigSection, TEXT("EnableGameToDiscord"), bEnableGameToDiscord, GGameIni);
		GConfig->GetBool(*ConfigSection, TEXT("EnableDiscordToGame"), bEnableDiscordToGame, GGameIni);
		GConfig->GetString(*ConfigSection, TEXT("GameMessageFormat"), GameMessageFormat, GGameIni);
		GConfig->GetString(*ConfigSection, TEXT("DiscordMessageFormat"), DiscordMessageFormat, GGameIni);
		GConfig->GetFloat(*ConfigSection, TEXT("DiscordMessageColorR"), ColorR, GGameIni);
		GConfig->GetFloat(*ConfigSection, TEXT("DiscordMessageColorG"), ColorG, GGameIni);
		GConfig->GetFloat(*ConfigSection, TEXT("DiscordMessageColorB"), ColorB, GGameIni);
		GConfig->GetInt(*ConfigSection, TEXT("LogVerbosity"), LogVerbosity, GGameIni);
		
		if (!BotToken.IsEmpty() && !ChannelId.IsEmpty())
		{
			BotConfig.BotToken = BotToken;
			BotConfig.ChannelId = ChannelId;
			BotConfig.PollIntervalSeconds = PollInterval;
			BotConfig.bEnableGameToDiscord = bEnableGameToDiscord;
			BotConfig.bEnableDiscordToGame = bEnableDiscordToGame;
			BotConfig.GameMessageFormat = GameMessageFormat.IsEmpty() ? TEXT("**[{username}]** {message}") : GameMessageFormat;
			BotConfig.DiscordMessageFormat = DiscordMessageFormat.IsEmpty() ? TEXT("[Discord] {username}") : DiscordMessageFormat;
			BotConfig.DiscordMessageColor = FLinearColor(ColorR, ColorG, ColorB);
			BotConfig.LogVerbosity = FMath::Clamp(LogVerbosity, 0, 4);
			
			if (BotConfig.LogVerbosity >= 3)
			{
				UE_LOG(LogTemp, Log, TEXT("DiscordChatSubsystem: Configuration loaded - Channel ID: %s, Poll Interval: %.1fs"), 
					*ChannelId, PollInterval);
			}
			
			if (BotConfig.LogVerbosity >= 4)
			{
				UE_LOG(LogTemp, Verbose, TEXT("DiscordChatSubsystem: GameToDiscord=%d, DiscordToGame=%d, LogVerbosity=%d"),
					bEnableGameToDiscord, bEnableDiscordToGame, LogVerbosity);
			}
		}
		else
		{
			if (BotConfig.LogVerbosity >= 2)
			{
				UE_LOG(LogTemp, Warning, TEXT("DiscordChatSubsystem: Configuration incomplete - please set BotToken and ChannelId in Config/DefaultDiscordChatBridge.ini"));
			}
		}
	}
}

void ADiscordChatSubsystem::OnGameChatMessageAdded()
{
	if (!ChatManager || !DiscordAPI || !DiscordAPI->IsInitialized())
	{
		return;
	}
	
	// Check if game-to-Discord forwarding is enabled
	if (!BotConfig.bEnableGameToDiscord)
	{
		if (BotConfig.LogVerbosity >= 4)
		{
			UE_LOG(LogTemp, Verbose, TEXT("DiscordChatSubsystem: Game-to-Discord forwarding is disabled"));
		}
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
	
	// Apply message format
	FString FormattedMessage = BotConfig.GameMessageFormat;
	FormattedMessage = FormattedMessage.Replace(TEXT("{username}"), *Username);
	FormattedMessage = FormattedMessage.Replace(TEXT("{message}"), *MessageText);
	
	if (BotConfig.LogVerbosity >= 4)
	{
		UE_LOG(LogTemp, Verbose, TEXT("DiscordChatSubsystem: Forwarding message to Discord from %s: %s"), 
			*Username, *MessageText);
	}
	
	// Send the formatted message (username is empty since it's included in the format)
	DiscordAPI->SendMessage(TEXT(""), FormattedMessage);
}

void ADiscordChatSubsystem::OnDiscordMessageReceived(const FString& Username, const FString& Message)
{
	// Check if Discord-to-game forwarding is enabled
	if (!BotConfig.bEnableDiscordToGame)
	{
		if (BotConfig.LogVerbosity >= 4)
		{
			UE_LOG(LogTemp, Verbose, TEXT("DiscordChatSubsystem: Discord-to-game forwarding is disabled"));
		}
		return;
	}
	
	if (BotConfig.LogVerbosity >= 3)
	{
		UE_LOG(LogTemp, Log, TEXT("DiscordChatSubsystem: Received Discord message from %s: %s"), 
			*Username, *Message);
	}
	
	ForwardDiscordMessageToGame(Username, Message);
}

void ADiscordChatSubsystem::ForwardDiscordMessageToGame(const FString& Username, const FString& Message)
{
	if (!ChatManager)
	{
		return;
	}
	
	// Apply message format for the sender name
	FString FormattedSender = BotConfig.DiscordMessageFormat;
	FormattedSender = FormattedSender.Replace(TEXT("{username}"), *Username);
	
	// Create a chat message struct
	FChatMessageStruct ChatMessage;
	ChatMessage.MessageType = EFGChatMessageType::CMT_CustomMessage;
	ChatMessage.MessageSender = FText::FromString(FormattedSender);
	ChatMessage.MessageText = FText::FromString(Message);
	ChatMessage.MessageSenderColor = BotConfig.DiscordMessageColor;
	
	// Broadcast to all players
	ChatManager->BroadcastChatMessage(ChatMessage, nullptr);
	
	if (BotConfig.LogVerbosity >= 4)
	{
		UE_LOG(LogTemp, Verbose, TEXT("DiscordChatSubsystem: Forwarded Discord message to game"));
	}
}
