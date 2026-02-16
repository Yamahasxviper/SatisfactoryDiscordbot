// Copyright (c) 2024 Discord Chat Bridge Contributors

#include "DiscordChatSubsystem.h"
#include "ServerDefaultsConfigLoader.h"
#include "DiscordChatLogger.h"
#include "FGChatManager.h"
#include "Misc/ConfigCacheIni.h"
#include "Misc/Paths.h"
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
	
	UE_LOG(LogTemp, Log, TEXT("===================================================================="));
	UE_LOG(LogTemp, Log, TEXT("DiscordChatSubsystem: Initializing Discord Chat Bridge mod"));
	UE_LOG(LogTemp, Log, TEXT("===================================================================="));
	
	// Load configuration
	LoadConfiguration();
	
	// Initialize file logger
	FString LogDirectory;
	if (!BotConfig.LogFilePath.IsEmpty())
	{
		// Use configured path
		LogDirectory = BotConfig.LogFilePath;
	}
	else
	{
		// Use default path: Mods/DiscordChatBridge/Logs
		FString ModsPath = FPaths::ProjectModsDir();
		LogDirectory = FPaths::Combine(ModsPath, TEXT("DiscordChatBridge"), TEXT("Logs"));
	}
	
	// Convert to full path
	LogDirectory = FPaths::ConvertRelativePathToFull(LogDirectory);
	
	UE_LOG(LogTemp, Log, TEXT("DiscordChatSubsystem: Initializing file logger at: %s"), *LogDirectory);
	FDiscordChatLogger::Get().Initialize(LogDirectory);
	
	if (FDiscordChatLogger::Get().IsInitialized())
	{
		UE_LOG(LogTemp, Log, TEXT("DiscordChatSubsystem: ✓ File logger initialized successfully"));
		UE_LOG(LogTemp, Log, TEXT("DiscordChatSubsystem: Log file: %s"), *FDiscordChatLogger::Get().GetLogFilePath());
		
		// Log initialization message to file
		DISCORD_LOG_INFO(TEXT("===================================================================="));
		DISCORD_LOG_INFO(TEXT("Discord Chat Bridge - Initializing"));
		DISCORD_LOG_INFO(TEXT("===================================================================="));
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("DiscordChatSubsystem: ⚠️  File logger failed to initialize - logs will only go to console"));
	}
	
	// Create Discord API instance
	UE_LOG(LogTemp, Log, TEXT("DiscordChatSubsystem: Creating UDiscordAPI object..."));
	DiscordAPI = NewObject<UDiscordAPI>(this);
	if (DiscordAPI)
	{
		UE_LOG(LogTemp, Log, TEXT("DiscordChatSubsystem: UDiscordAPI object created successfully"));
		DiscordAPI->Initialize(BotConfig);
		DiscordAPI->OnMessageReceived.BindUObject(this, &ADiscordChatSubsystem::OnDiscordMessageReceived);
		
		// Log initialization status with clear visual indicators
		if (DiscordAPI->IsInitialized())
		{
			UE_LOG(LogTemp, Warning, TEXT("===================================================================="));
			UE_LOG(LogTemp, Warning, TEXT("✅ SUCCESS: Discord Chat Bridge is ACTIVE and READY"));
			UE_LOG(LogTemp, Warning, TEXT("   - Bot is configured and will connect to Discord"));
			UE_LOG(LogTemp, Warning, TEXT("   - Chat messages will be synchronized"));
			UE_LOG(LogTemp, Warning, TEXT("===================================================================="));
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("===================================================================="));
			UE_LOG(LogTemp, Warning, TEXT("⚠️  Discord Chat Bridge - No Configuration Found"));
			UE_LOG(LogTemp, Warning, TEXT("===================================================================="));
			UE_LOG(LogTemp, Warning, TEXT("   ✅ SERVER IS RUNNING NORMALLY"));
			UE_LOG(LogTemp, Warning, TEXT("   ℹ️  Discord features are disabled (configuration not provided)"));
			UE_LOG(LogTemp, Warning, TEXT("   ℹ️  This is OPTIONAL - server will work fine without it"));
			UE_LOG(LogTemp, Warning, TEXT("===================================================================="));
			UE_LOG(LogTemp, Warning, TEXT("   To enable Discord integration (optional):"));
			UE_LOG(LogTemp, Warning, TEXT("   1. Get Bot Token from: https://discord.com/developers/applications"));
			UE_LOG(LogTemp, Warning, TEXT("   2. Configure: Mods/DiscordChatBridge/config/DiscordChatBridge.ini"));
			UE_LOG(LogTemp, Warning, TEXT("   3. See QUICKSTART.md for 5-minute setup guide"));
			UE_LOG(LogTemp, Warning, TEXT("===================================================================="));
		}
	}
	else
	{
		DISCORD_LOG_ERROR(TEXT("===================================================================="));
		DISCORD_LOG_ERROR(TEXT("❌ CRITICAL ERROR: Failed to create UDiscordAPI object!"));
		DISCORD_LOG_ERROR(TEXT("   - This may indicate memory allocation failure or object system issues"));
		DISCORD_LOG_ERROR(TEXT("   - Discord chat bridge will NOT function"));
		DISCORD_LOG_ERROR(TEXT("===================================================================="));
	}
}

void ADiscordChatSubsystem::BeginPlay()
{
	Super::BeginPlay();
	
	UE_LOG(LogTemp, Log, TEXT("===================================================================="));
	UE_LOG(LogTemp, Log, TEXT("DiscordChatSubsystem: BeginPlay - Starting subsystem"));
	UE_LOG(LogTemp, Log, TEXT("===================================================================="));
	
	// Validate World pointer
	UWorld* World = GetWorld();
	if (!World)
	{
		DISCORD_LOG_ERROR(TEXT("===================================================================="));
		DISCORD_LOG_ERROR(TEXT("❌ CRITICAL ERROR: GetWorld() returned nullptr!"));
		DISCORD_LOG_ERROR(TEXT("   Cannot initialize without valid World pointer"));
		DISCORD_LOG_ERROR(TEXT("===================================================================="));
		return;
	}
	UE_LOG(LogTemp, Log, TEXT("DiscordChatSubsystem: ✓ World pointer validated"));
	
	// Early exit if Discord API is not initialized (missing configuration)
	if (!DiscordAPI || !DiscordAPI->IsInitialized())
	{
		UE_LOG(LogTemp, Log, TEXT("DiscordChatSubsystem: BeginPlay - Discord features disabled (not configured)"));
		UE_LOG(LogTemp, Log, TEXT("DiscordChatSubsystem: Server continues normally without Discord integration"));
		return;
	}
	
	// Get reference to chat manager
	UE_LOG(LogTemp, Log, TEXT("DiscordChatSubsystem: Attempting to get AFGChatManager reference..."));
	ChatManager = AFGChatManager::Get(World);
	
	if (ChatManager)
	{
		// Bind to chat message event
		ChatManager->OnChatMessageAdded.AddDynamic(this, &ADiscordChatSubsystem::OnGameChatMessageAdded);
		UE_LOG(LogTemp, Log, TEXT("DiscordChatSubsystem: ✓ Successfully bound to chat manager"));
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("DiscordChatSubsystem: Failed to get AFGChatManager - in-game chat integration will not work"));
		UE_LOG(LogTemp, Warning, TEXT("DiscordChatSubsystem: This may be normal if the chat manager hasn't been created yet"));
	}
	
	// Start polling Discord for messages
	UE_LOG(LogTemp, Log, TEXT("DiscordChatSubsystem: Starting Discord message polling..."));
	DiscordAPI->StartPolling();
	UE_LOG(LogTemp, Warning, TEXT("✅ Discord message polling STARTED - Bot is now active"));
	
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
	
	UE_LOG(LogTemp, Warning, TEXT("===================================================================="));
	UE_LOG(LogTemp, Warning, TEXT("✅ Discord Chat Bridge FULLY OPERATIONAL"));
	UE_LOG(LogTemp, Warning, TEXT("   - Two-way chat synchronization is active"));
	UE_LOG(LogTemp, Warning, TEXT("   - Messages will be bridged between Discord and game"));
	UE_LOG(LogTemp, Warning, TEXT("===================================================================="));
}

void ADiscordChatSubsystem::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	(void)EndPlayReason;  // Suppress unused parameter warning
	
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
	
	// Shutdown file logger
	UE_LOG(LogTemp, Log, TEXT("DiscordChatSubsystem: Shutting down file logger"));
	DISCORD_LOG_INFO(TEXT("Discord Chat Bridge - Shutting down"));
	FDiscordChatLogger::Get().Shutdown();
	
	Super::EndPlay(EndPlayReason);
}

void ADiscordChatSubsystem::LoadConfiguration()
{
	UE_LOG(LogTemp, Log, TEXT("DiscordChatSubsystem: === BEGIN CONFIGURATION LOADING ==="));
	
	// Try to load from config INI format first (new system)
	UE_LOG(LogTemp, Log, TEXT("DiscordChatSubsystem: Attempting to load from config/DiscordChatBridge.ini..."));
	bool bLoadedFromIni = FServerDefaultsConfigLoader::LoadFromServerDefaults(BotConfig);
	if (bLoadedFromIni)
	{
		UE_LOG(LogTemp, Log, TEXT("DiscordChatSubsystem: SUCCESS - Configuration loaded from config/DiscordChatBridge.ini"));
		
		// Validate loaded configuration
		if (BotConfig.BotToken.IsEmpty())
		{
			UE_LOG(LogTemp, Error, TEXT("DiscordChatSubsystem: ERROR - BotToken is EMPTY in config file!"));
		}
		else
		{
			UE_LOG(LogTemp, Log, TEXT("DiscordChatSubsystem: ✓ BotToken is configured"));
		}
		
		if (BotConfig.ChannelId.IsEmpty())
		{
			UE_LOG(LogTemp, Error, TEXT("DiscordChatSubsystem: ERROR - ChannelId is EMPTY in config file!"));
		}
		else
		{
			UE_LOG(LogTemp, Log, TEXT("DiscordChatSubsystem: ✓ ChannelId: %s"), *BotConfig.ChannelId);
		}
		
		UE_LOG(LogTemp, Log, TEXT("DiscordChatSubsystem: - Poll Interval: %.1fs"), BotConfig.PollIntervalSeconds);
		UE_LOG(LogTemp, Log, TEXT("DiscordChatSubsystem: - Server Notifications: %s"), BotConfig.bEnableServerNotifications ? TEXT("Enabled") : TEXT("Disabled"));
		UE_LOG(LogTemp, Log, TEXT("DiscordChatSubsystem: - Bot Activity Updates: %s"), BotConfig.bEnableBotActivity ? TEXT("Enabled") : TEXT("Disabled"));
		UE_LOG(LogTemp, Log, TEXT("DiscordChatSubsystem: - Gateway Presence: %s"), BotConfig.bUseGatewayForPresence ? TEXT("Enabled") : TEXT("Disabled"));
		UE_LOG(LogTemp, Log, TEXT("DiscordChatSubsystem: === END CONFIGURATION LOADING ==="));
		return;
	}
	
	UE_LOG(LogTemp, Log, TEXT("DiscordChatSubsystem: config/DiscordChatBridge.ini not found - falling back to Unreal Engine config system"));
	
	// Fallback to INI format (legacy system)
	FString ConfigSection = TEXT("/Script/DiscordChatBridge.DiscordChatSubsystem");
	
	if (!GConfig)
	{
		DISCORD_LOG_ERROR(TEXT("DiscordChatSubsystem: CRITICAL ERROR - GConfig is nullptr!"));
		DISCORD_LOG_ERROR(TEXT("DiscordChatSubsystem: Cannot load configuration from INI files"));
		DISCORD_LOG_ERROR(TEXT("DiscordChatSubsystem: This indicates a serious engine initialization problem"));
		DISCORD_LOG_ERROR(TEXT("DiscordChatSubsystem: === END CONFIGURATION LOADING (FAILED) ==="));
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
	FString LogFilePath;
	
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
	GConfig->GetString(*ConfigSection, TEXT("LogFilePath"), LogFilePath, GGameIni);
	
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
		
		// Load log file path
		if (!LogFilePath.IsEmpty())
		{
			BotConfig.LogFilePath = LogFilePath;
		}
		
		UE_LOG(LogTemp, Log, TEXT("DiscordChatSubsystem: SUCCESS - Configuration loaded from INI"));
		UE_LOG(LogTemp, Log, TEXT("DiscordChatSubsystem: ✓ BotToken is configured"));
		UE_LOG(LogTemp, Log, TEXT("DiscordChatSubsystem: ✓ ChannelId: %s"), *ChannelId);
		UE_LOG(LogTemp, Log, TEXT("DiscordChatSubsystem: - Poll Interval: %.1fs"), PollInterval);
		UE_LOG(LogTemp, Log, TEXT("DiscordChatSubsystem: - Server Notifications: %s"), bEnableServerNotifications ? TEXT("Enabled") : TEXT("Disabled"));
		UE_LOG(LogTemp, Log, TEXT("DiscordChatSubsystem: - Bot Activity Updates: %s"), bEnableBotActivity ? TEXT("Enabled") : TEXT("Disabled"));
		UE_LOG(LogTemp, Log, TEXT("DiscordChatSubsystem: - Gateway Presence: %s"), bUseGatewayForPresence ? TEXT("Enabled") : TEXT("Disabled"));
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("===================================================================="));
		UE_LOG(LogTemp, Warning, TEXT("⚠️  Discord Configuration Incomplete"));
		UE_LOG(LogTemp, Warning, TEXT("===================================================================="));
		UE_LOG(LogTemp, Warning, TEXT("   ✅ SERVER IS RUNNING - No issues detected"));
		UE_LOG(LogTemp, Warning, TEXT("   ℹ️  Discord integration is OPTIONAL and currently disabled"));
		UE_LOG(LogTemp, Warning, TEXT("===================================================================="));
		if (BotToken.IsEmpty())
		{
			UE_LOG(LogTemp, Warning, TEXT("   Missing: BotToken"));
			UE_LOG(LogTemp, Warning, TEXT("      → Get from: https://discord.com/developers/applications"));
		}
		else
		{
			UE_LOG(LogTemp, Log, TEXT("   ✓ BotToken is configured"));
		}
		if (ChannelId.IsEmpty())
		{
			UE_LOG(LogTemp, Warning, TEXT("   Missing: ChannelId"));
			UE_LOG(LogTemp, Warning, TEXT("      → Enable Developer Mode in Discord, right-click channel, Copy ID"));
		}
		else
		{
			UE_LOG(LogTemp, Log, TEXT("   ✓ ChannelId is configured"));
		}
		UE_LOG(LogTemp, Warning, TEXT("===================================================================="));
		UE_LOG(LogTemp, Warning, TEXT("   To enable Discord features (optional):"));
		UE_LOG(LogTemp, Warning, TEXT("   1. Mods/DiscordChatBridge/config/DiscordChatBridge.ini (recommended)"));
		UE_LOG(LogTemp, Warning, TEXT("   2. Config/DefaultDiscordChatBridge.ini (legacy)"));
		UE_LOG(LogTemp, Warning, TEXT("   3. See Mods/DiscordChatBridge/help/QUICKSTART.md for setup"));
		UE_LOG(LogTemp, Warning, TEXT("===================================================================="));
		
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

