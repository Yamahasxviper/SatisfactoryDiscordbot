// Copyright (c) 2024 Discord Chat Bridge Contributors

#include "DiscordChatSubsystem.h"
#include "ServerDefaultsConfigLoader.h"
#include "FGChatManager.h"
#include "Misc/ConfigCacheIni.h"
#include "Engine/World.h"
#include "GameFramework/GameStateBase.h"
#include "GameFramework/PlayerState.h"
#include "HAL/PlatformProcess.h"

ADiscordChatSubsystem::ADiscordChatSubsystem()
	: LastProcessedMessageIndex(0)
{
	ReplicationPolicy = ESubsystemReplicationPolicy::SpawnOnServer;
}

void ADiscordChatSubsystem::Init()
{
	Super::Init();
	
	UE_LOG(LogTemp, Log, TEXT("DiscordChatSubsystem: Initializing Discord Chat Bridge mod"));
	
	// Load configuration
	LoadConfiguration();
	
	// Create Discord API instance
	UE_LOG(LogTemp, Log, TEXT("DiscordChatSubsystem: Creating UDiscordAPI object..."));
	DiscordAPI = NewObject<UDiscordAPI>(this);
	if (DiscordAPI)
	{
		UE_LOG(LogTemp, Log, TEXT("DiscordChatSubsystem: UDiscordAPI object created successfully"));
		DiscordAPI->Initialize(BotConfig);
		DiscordAPI->OnMessageReceived.BindUObject(this, &ADiscordChatSubsystem::OnDiscordMessageReceived);
		
		// Log initialization status
		if (DiscordAPI->IsInitialized())
		{
			UE_LOG(LogTemp, Log, TEXT("DiscordChatSubsystem: Successfully initialized with valid configuration"));
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("DiscordChatSubsystem: Initialized but not configured - mod will not be active"));
			UE_LOG(LogTemp, Warning, TEXT("DiscordChatSubsystem: To use this mod, configure BotToken and ChannelId in DiscordChatBridge.ini"));
		}
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("DiscordChatSubsystem: CRITICAL ERROR - Failed to create UDiscordAPI object!"));
		UE_LOG(LogTemp, Error, TEXT("DiscordChatSubsystem: This may indicate memory allocation failure or object system issues"));
		UE_LOG(LogTemp, Error, TEXT("DiscordChatSubsystem: Discord chat bridge will NOT function"));
	}
}

void ADiscordChatSubsystem::BeginPlay()
{
	Super::BeginPlay();
	
	UE_LOG(LogTemp, Log, TEXT("DiscordChatSubsystem: BeginPlay called - starting subsystem initialization"));
	
	// Validate World pointer
	UWorld* World = GetWorld();
	if (!World)
	{
		UE_LOG(LogTemp, Error, TEXT("DiscordChatSubsystem: CRITICAL ERROR - GetWorld() returned nullptr!"));
		UE_LOG(LogTemp, Error, TEXT("DiscordChatSubsystem: Cannot initialize without valid World pointer"));
		return;
	}
	UE_LOG(LogTemp, Log, TEXT("DiscordChatSubsystem: World pointer validated successfully"));
	
	// Early exit if Discord API is not initialized (missing configuration)
	if (!DiscordAPI || !DiscordAPI->IsInitialized())
	{
		UE_LOG(LogTemp, Warning, TEXT("DiscordChatSubsystem: BeginPlay called but mod is not configured - skipping initialization"));
		return;
	}
	
	// Get reference to chat manager
	UE_LOG(LogTemp, Log, TEXT("DiscordChatSubsystem: Attempting to get AFGChatManager reference..."));
	ChatManager = AFGChatManager::Get(World);
	
	if (ChatManager)
	{
		// Bind to chat message event
		ChatManager->OnChatMessageAdded.AddDynamic(this, &ADiscordChatSubsystem::OnGameChatMessageAdded);
		UE_LOG(LogTemp, Log, TEXT("DiscordChatSubsystem: Successfully bound to chat manager"));
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("DiscordChatSubsystem: Failed to get AFGChatManager - in-game chat integration will not work"));
		UE_LOG(LogTemp, Warning, TEXT("DiscordChatSubsystem: This may be normal if the chat manager hasn't been created yet"));
	}
	
	// Start polling Discord for messages
	UE_LOG(LogTemp, Log, TEXT("DiscordChatSubsystem: Starting Discord message polling..."));
	DiscordAPI->StartPolling();
	
	// Send server start notification if enabled
	if (BotConfig.bEnableServerNotifications)
	{
		UE_LOG(LogTemp, Log, TEXT("DiscordChatSubsystem: Sending server start notification"));
		DiscordAPI->SendNotification(BotConfig.ServerStartMessage);
	}
	
	// Start bot activity updates if enabled
	if (BotConfig.bEnableBotActivity)
	{
		UE_LOG(LogTemp, Log, TEXT("DiscordChatSubsystem: Bot activity updates enabled - setting up timer"));
		DiscordAPI->StartActivityUpdates();
		
		// Set up timer to periodically update activity
		World->GetTimerManager().SetTimer(
			ActivityTimerHandle,
			this,
			&ADiscordChatSubsystem::UpdateBotActivity,
			BotConfig.ActivityUpdateIntervalSeconds,
			true,
			0.0f  // Start immediately
		);
		UE_LOG(LogTemp, Log, TEXT("DiscordChatSubsystem: Activity update timer started (interval: %f seconds)"), BotConfig.ActivityUpdateIntervalSeconds);
	}
	
	UE_LOG(LogTemp, Log, TEXT("DiscordChatSubsystem: BeginPlay initialization complete"));
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
		DiscordAPI->StopActivityUpdates();
	}
	
	// Clear activity timer
	UWorld* World = GetWorld();
	if (World)
	{
		World->GetTimerManager().ClearTimer(ActivityTimerHandle);
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
	UE_LOG(LogTemp, Log, TEXT("DiscordChatSubsystem: === BEGIN CONFIGURATION LOADING ==="));
	
	// Try to load from config TXT format first (new system)
	UE_LOG(LogTemp, Log, TEXT("DiscordChatSubsystem: Attempting to load from config/DiscordChatBridge.txt..."));
	bool bLoadedFromTxt = FServerDefaultsConfigLoader::LoadFromServerDefaults(BotConfig);
	if (bLoadedFromTxt)
	{
		UE_LOG(LogTemp, Log, TEXT("DiscordChatSubsystem: SUCCESS - Configuration loaded from config/DiscordChatBridge.txt"));
		UE_LOG(LogTemp, Log, TEXT("DiscordChatSubsystem: Channel ID: %s, Poll Interval: %.1fs, Notifications: %s, Bot Activity: %s, Gateway: %s"), 
			*BotConfig.ChannelId, BotConfig.PollIntervalSeconds, 
			BotConfig.bEnableServerNotifications ? TEXT("Enabled") : TEXT("Disabled"), 
			BotConfig.bEnableBotActivity ? TEXT("Enabled") : TEXT("Disabled"), 
			BotConfig.bUseGatewayForPresence ? TEXT("Enabled") : TEXT("Disabled"));
		UE_LOG(LogTemp, Log, TEXT("DiscordChatSubsystem: === END CONFIGURATION LOADING ==="));
		return;
	}
	
	UE_LOG(LogTemp, Log, TEXT("DiscordChatSubsystem: TXT config not found - falling back to INI config system"));
	
	// Fallback to INI format (legacy system)
	FString ConfigSection = TEXT("/Script/DiscordChatBridge.DiscordChatSubsystem");
	
	if (!GConfig)
	{
		UE_LOG(LogTemp, Error, TEXT("DiscordChatSubsystem: CRITICAL ERROR - GConfig is nullptr!"));
		UE_LOG(LogTemp, Error, TEXT("DiscordChatSubsystem: Cannot load configuration from INI files"));
		UE_LOG(LogTemp, Error, TEXT("DiscordChatSubsystem: This indicates a serious engine initialization problem"));
		UE_LOG(LogTemp, Error, TEXT("DiscordChatSubsystem: === END CONFIGURATION LOADING (FAILED) ==="));
		return;
	}
	
	UE_LOG(LogTemp, Log, TEXT("DiscordChatSubsystem: GConfig validated - loading INI settings from section: %s"), *ConfigSection);
	
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
	bool bEnableBotActivity = false;
	FString BotActivityFormat;
	float ActivityUpdateIntervalSeconds = 60.0f;
	FString BotActivityChannelId;
	bool bUseGatewayForPresence = false;
	FString GatewayPresenceFormat;
	int32 GatewayActivityType = 0;
	
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
	GConfig->GetBool(*ConfigSection, TEXT("EnableBotActivity"), bEnableBotActivity, GGameIni);
	GConfig->GetString(*ConfigSection, TEXT("BotActivityFormat"), BotActivityFormat, GGameIni);
	GConfig->GetFloat(*ConfigSection, TEXT("ActivityUpdateIntervalSeconds"), ActivityUpdateIntervalSeconds, GGameIni);
	GConfig->GetString(*ConfigSection, TEXT("BotActivityChannelId"), BotActivityChannelId, GGameIni);
	GConfig->GetBool(*ConfigSection, TEXT("UseGatewayForPresence"), bUseGatewayForPresence, GGameIni);
	GConfig->GetString(*ConfigSection, TEXT("GatewayPresenceFormat"), GatewayPresenceFormat, GGameIni);
	GConfig->GetInt(*ConfigSection, TEXT("GatewayActivityType"), GatewayActivityType, GGameIni);
	
	UE_LOG(LogTemp, Log, TEXT("DiscordChatSubsystem: INI values read - BotToken: %s, ChannelId: %s"), 
		BotToken.IsEmpty() ? TEXT("EMPTY") : TEXT("SET"), 
		ChannelId.IsEmpty() ? TEXT("EMPTY") : TEXT("SET"));
	
	if (!BotToken.IsEmpty() && !ChannelId.IsEmpty())
	{
		UE_LOG(LogTemp, Log, TEXT("DiscordChatSubsystem: Required configuration values found - applying settings..."));
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
		
		// Load bot activity settings
		BotConfig.bEnableBotActivity = bEnableBotActivity;
		BotConfig.bUseGatewayForPresence = bUseGatewayForPresence;
		if (!BotActivityFormat.IsEmpty())
		{
			BotConfig.BotActivityFormat = BotActivityFormat;
		}
		BotConfig.ActivityUpdateIntervalSeconds = ActivityUpdateIntervalSeconds;
		if (!BotActivityChannelId.IsEmpty())
		{
			BotConfig.BotActivityChannelId = BotActivityChannelId;
		}
		if (!GatewayPresenceFormat.IsEmpty())
		{
			BotConfig.GatewayPresenceFormat = GatewayPresenceFormat;
		}
		BotConfig.GatewayActivityType = GatewayActivityType;
		
		UE_LOG(LogTemp, Log, TEXT("DiscordChatSubsystem: SUCCESS - Configuration loaded from INI - Channel ID: %s, Poll Interval: %.1fs, Notifications: %s, Bot Activity: %s, Gateway: %s"), 
			*ChannelId, PollInterval, bEnableServerNotifications ? TEXT("Enabled") : TEXT("Disabled"), bEnableBotActivity ? TEXT("Enabled") : TEXT("Disabled"), bUseGatewayForPresence ? TEXT("Enabled") : TEXT("Disabled"));
	}
	else
	{
		if (BotToken.IsEmpty())
		{
			UE_LOG(LogTemp, Warning, TEXT("DiscordChatSubsystem: BotToken not found in INI configuration"));
		}
		if (ChannelId.IsEmpty())
		{
			UE_LOG(LogTemp, Warning, TEXT("DiscordChatSubsystem: ChannelId not found in INI configuration"));
		}
		UE_LOG(LogTemp, Warning, TEXT("DiscordChatSubsystem: Configuration incomplete - BotToken and ChannelId must be set"));
		UE_LOG(LogTemp, Warning, TEXT("DiscordChatSubsystem: Expected location: Config/DefaultDiscordChatBridge.ini"));
		UE_LOG(LogTemp, Warning, TEXT("DiscordChatSubsystem: Expected section: %s"), *ConfigSection);
		UE_LOG(LogTemp, Warning, TEXT("DiscordChatSubsystem: The mod will load but remain inactive until configured"));
		UE_LOG(LogTemp, Warning, TEXT("DiscordChatSubsystem: See help/QUICKSTART.md for setup instructions"));
		
		// Ensure all feature flags are disabled when configuration is incomplete
		BotConfig.bEnableServerNotifications = false;
		BotConfig.bEnableBotActivity = false;
		BotConfig.bUseGatewayForPresence = false;
	}
	
	UE_LOG(LogTemp, Log, TEXT("DiscordChatSubsystem: === END CONFIGURATION LOADING ==="));
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

void ADiscordChatSubsystem::UpdateBotActivity()
{
	if (!DiscordAPI || !DiscordAPI->IsInitialized())
	{
		return;
	}

	int32 PlayerCount = GetPlayerCount();
	DiscordAPI->UpdateBotActivity(PlayerCount);
}

int32 ADiscordChatSubsystem::GetPlayerCount() const
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return 0;
	}

	AGameStateBase* GameState = World->GetGameState();
	if (!GameState)
	{
		return 0;
	}

	// Get the number of player states, which represents connected players
	return GameState->PlayerArray.Num();
}

