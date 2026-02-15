// Copyright (c) 2024 Discord Chat Bridge Contributors

#include "DiscordAPI.h"
#include "DiscordGateway.h"
#include "HttpModule.h"
#include "Interfaces/IHttpResponse.h"
#include "Dom/JsonObject.h"
#include "Dom/JsonValue.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonWriter.h"
#include "Serialization/JsonSerializer.h"
#include "JsonUtilities.h"
#include "TimerManager.h"
#include "Engine/World.h"

UDiscordAPI::UDiscordAPI()
	: Gateway(nullptr)
	, bIsInitialized(false)
	, bIsPolling(false)
	, bIsUpdatingActivity(false)
{
}

void UDiscordAPI::Initialize(const FDiscordBotConfig& Config)
{
	UE_LOG(LogTemp, Log, TEXT("DiscordAPI: Initialize called - validating configuration..."));
	BotConfig = Config;
	bIsInitialized = !BotConfig.BotToken.IsEmpty() && !BotConfig.ChannelId.IsEmpty();

	if (bIsInitialized)
	{
		UE_LOG(LogTemp, Log, TEXT("DiscordAPI: Configuration valid - Initialized with channel ID: %s"), *BotConfig.ChannelId);

		// Initialize Gateway if enabled
		if (BotConfig.bUseGatewayForPresence)
		{
			UE_LOG(LogTemp, Log, TEXT("DiscordAPI: Gateway presence enabled - attempting to create Gateway connection..."));
#if WITH_WEBSOCKETS_SUPPORT
			Gateway = NewObject<UDiscordGateway>(this);
			if (Gateway)
			{
				UE_LOG(LogTemp, Log, TEXT("DiscordAPI: UDiscordGateway object created successfully"));
				Gateway->Initialize(BotConfig.BotToken);
				Gateway->OnConnected.BindUObject(this, &UDiscordAPI::OnGatewayConnected);
				Gateway->OnDisconnected.BindUObject(this, &UDiscordAPI::OnGatewayDisconnected);
				Gateway->Connect();
				UE_LOG(LogTemp, Log, TEXT("DiscordAPI: Gateway connection initiated"));
			}
			else
			{
				UE_LOG(LogTemp, Error, TEXT("DiscordAPI: CRITICAL ERROR - Failed to create UDiscordGateway object!"));
				UE_LOG(LogTemp, Error, TEXT("DiscordAPI: Gateway features will not be available"));
				UE_LOG(LogTemp, Error, TEXT("DiscordAPI: Bot presence/status updates are disabled"));
			}
#else
			UE_LOG(LogTemp, Warning, TEXT("DiscordAPI: Gateway requested but WITH_WEBSOCKETS_SUPPORT=0"));
			UE_LOG(LogTemp, Warning, TEXT("DiscordAPI: WebSockets plugin was not found during compilation"));
			UE_LOG(LogTemp, Warning, TEXT("DiscordAPI: Gateway features are disabled - only REST API will work"));
			UE_LOG(LogTemp, Warning, TEXT("DiscordAPI: To enable Gateway, ensure WebSockets plugin is available in Engine or Project plugins"));
#endif
		}
		else
		{
			UE_LOG(LogTemp, Log, TEXT("DiscordAPI: Gateway presence disabled in configuration - using REST API only"));
		}
	}
	else
	{
		if (BotConfig.BotToken.IsEmpty())
		{
			UE_LOG(LogTemp, Warning, TEXT("DiscordAPI: Failed to initialize - BotToken is empty"));
		}
		if (BotConfig.ChannelId.IsEmpty())
		{
			UE_LOG(LogTemp, Warning, TEXT("DiscordAPI: Failed to initialize - ChannelId is empty"));
		}
		UE_LOG(LogTemp, Warning, TEXT("DiscordAPI: Please configure both BotToken and ChannelId in DiscordChatBridge.ini"));
	}
}

void UDiscordAPI::SendMessage(const FString& Username, const FString& Message)
{
	if (!bIsInitialized)
	{
		UE_LOG(LogTemp, Warning, TEXT("DiscordAPI: Cannot send message - API not initialized"));
		return;
	}

	// Create the HTTP request
	TSharedRef<IHttpRequest, ESPMode::ThreadSafe> HttpRequest = FHttpModule::Get().CreateRequest();
	
	FString Url = FString::Printf(TEXT("https://discord.com/api/v10/channels/%s/messages"), *BotConfig.ChannelId);
	HttpRequest->SetURL(Url);
	HttpRequest->SetVerb(TEXT("POST"));
	HttpRequest->SetHeader(TEXT("Authorization"), FString::Printf(TEXT("Bot %s"), *BotConfig.BotToken));
	HttpRequest->SetHeader(TEXT("Content-Type"), TEXT("application/json"));

	// Create JSON payload with configurable format
	TSharedPtr<FJsonObject> JsonObject = MakeShareable(new FJsonObject());
	FString FormattedMessage = BotConfig.GameNameFormat;
	FormattedMessage = FormattedMessage.Replace(TEXT("{source}"), *BotConfig.GameSourceLabel);
	FormattedMessage = FormattedMessage.Replace(TEXT("{username}"), *Username);
	FormattedMessage = FormattedMessage.Replace(TEXT("{message}"), *Message);
	JsonObject->SetStringField(TEXT("content"), FormattedMessage);

	FString JsonString;
	TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&JsonString);
	FJsonSerializer::Serialize(JsonObject.ToSharedRef(), Writer);

	HttpRequest->SetContentAsString(JsonString);
	HttpRequest->OnProcessRequestComplete().BindUObject(this, &UDiscordAPI::OnSendMessageResponse);

	if (HttpRequest->ProcessRequest())
	{
		UE_LOG(LogTemp, Verbose, TEXT("DiscordAPI: Sending message from %s"), *Username);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("DiscordAPI: Failed to process send message request"));
	}
}

void UDiscordAPI::OnSendMessageResponse(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
{
	if (bWasSuccessful && Response.IsValid())
	{
		int32 ResponseCode = Response->GetResponseCode();
		if (ResponseCode >= 200 && ResponseCode < 300)
		{
			UE_LOG(LogTemp, Verbose, TEXT("DiscordAPI: Message sent successfully"));
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("DiscordAPI: Failed to send message - Response code: %d, Body: %s"), 
				ResponseCode, *Response->GetContentAsString());
		}
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("DiscordAPI: Failed to send message - Request failed"));
	}
}

void UDiscordAPI::SendNotification(const FString& Message)
{
	if (!bIsInitialized)
	{
		UE_LOG(LogTemp, Warning, TEXT("DiscordAPI: Cannot send notification - API not initialized"));
		return;
	}

	// Determine which channel to use for notifications
	FString TargetChannelId = BotConfig.NotificationChannelId.IsEmpty() ? BotConfig.ChannelId : BotConfig.NotificationChannelId;

	// Create the HTTP request
	TSharedRef<IHttpRequest, ESPMode::ThreadSafe> HttpRequest = FHttpModule::Get().CreateRequest();
	
	FString Url = FString::Printf(TEXT("https://discord.com/api/v10/channels/%s/messages"), *TargetChannelId);
	HttpRequest->SetURL(Url);
	HttpRequest->SetVerb(TEXT("POST"));
	HttpRequest->SetHeader(TEXT("Authorization"), FString::Printf(TEXT("Bot %s"), *BotConfig.BotToken));
	HttpRequest->SetHeader(TEXT("Content-Type"), TEXT("application/json"));

	// Create JSON payload for notification
	TSharedPtr<FJsonObject> JsonObject = MakeShareable(new FJsonObject());
	JsonObject->SetStringField(TEXT("content"), Message);

	FString JsonString;
	TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&JsonString);
	FJsonSerializer::Serialize(JsonObject.ToSharedRef(), Writer);

	HttpRequest->SetContentAsString(JsonString);
	HttpRequest->OnProcessRequestComplete().BindUObject(this, &UDiscordAPI::OnSendNotificationResponse);

	if (HttpRequest->ProcessRequest())
	{
		UE_LOG(LogTemp, Log, TEXT("DiscordAPI: Sending notification to channel %s"), *TargetChannelId);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("DiscordAPI: Failed to process send notification request"));
	}
}

void UDiscordAPI::OnSendNotificationResponse(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
{
	if (bWasSuccessful && Response.IsValid())
	{
		int32 ResponseCode = Response->GetResponseCode();
		if (ResponseCode >= 200 && ResponseCode < 300)
		{
			UE_LOG(LogTemp, Log, TEXT("DiscordAPI: Notification sent successfully"));
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("DiscordAPI: Failed to send notification - Response code: %d, Body: %s"), 
				ResponseCode, *Response->GetContentAsString());
		}
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("DiscordAPI: Failed to send notification - Request failed"));
	}
}

void UDiscordAPI::StartPolling()
{
	if (!bIsInitialized)
	{
		UE_LOG(LogTemp, Warning, TEXT("DiscordAPI: Cannot start polling - API not initialized"));
		return;
	}

	if (bIsPolling)
	{
		UE_LOG(LogTemp, Warning, TEXT("DiscordAPI: Already polling"));
		return;
	}

	bIsPolling = true;
	UE_LOG(LogTemp, Log, TEXT("DiscordAPI: Started polling for messages"));

	// Get the world for timer manager
	UWorld* World = GetWorld();
	if (World)
	{
		World->GetTimerManager().SetTimer(
			PollTimerHandle,
			this,
			&UDiscordAPI::PollMessages,
			BotConfig.PollIntervalSeconds,
			true
		);
	}
}

void UDiscordAPI::StopPolling()
{
	if (!bIsPolling)
	{
		return;
	}

	bIsPolling = false;
	UE_LOG(LogTemp, Log, TEXT("DiscordAPI: Stopped polling for messages"));

	UWorld* World = GetWorld();
	if (World)
	{
		World->GetTimerManager().ClearTimer(PollTimerHandle);
	}
}

void UDiscordAPI::PollMessages()
{
	if (!bIsInitialized || !bIsPolling)
	{
		return;
	}

	// Create the HTTP request
	TSharedRef<IHttpRequest, ESPMode::ThreadSafe> HttpRequest = FHttpModule::Get().CreateRequest();
	
	FString Url = FString::Printf(TEXT("https://discord.com/api/v10/channels/%s/messages?limit=10"), *BotConfig.ChannelId);
	if (!LastMessageId.IsEmpty())
	{
		Url += FString::Printf(TEXT("&after=%s"), *LastMessageId);
	}
	
	HttpRequest->SetURL(Url);
	HttpRequest->SetVerb(TEXT("GET"));
	HttpRequest->SetHeader(TEXT("Authorization"), FString::Printf(TEXT("Bot %s"), *BotConfig.BotToken));

	HttpRequest->OnProcessRequestComplete().BindUObject(this, &UDiscordAPI::OnPollMessagesResponse);

	if (!HttpRequest->ProcessRequest())
	{
		UE_LOG(LogTemp, Error, TEXT("DiscordAPI: Failed to process poll messages request"));
	}
}

void UDiscordAPI::OnPollMessagesResponse(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
{
	if (!bWasSuccessful || !Response.IsValid())
	{
		UE_LOG(LogTemp, Warning, TEXT("DiscordAPI: Failed to poll messages - Request failed"));
		return;
	}

	int32 ResponseCode = Response->GetResponseCode();
	if (ResponseCode < 200 || ResponseCode >= 300)
	{
		UE_LOG(LogTemp, Warning, TEXT("DiscordAPI: Failed to poll messages - Response code: %d"), ResponseCode);
		return;
	}

	// Parse JSON response
	FString ResponseString = Response->GetContentAsString();
	TArray<TSharedPtr<FJsonValue>> JsonArray;
	TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(ResponseString);
	
	if (!FJsonSerializer::Deserialize(Reader, JsonArray))
	{
		UE_LOG(LogTemp, Warning, TEXT("DiscordAPI: Failed to parse messages JSON"));
		return;
	}

	// Process messages in reverse order (oldest first)
	for (int32 i = JsonArray.Num() - 1; i >= 0; i--)
	{
		TSharedPtr<FJsonObject> MessageObj = JsonArray[i]->AsObject();
		if (!MessageObj.IsValid())
		{
			continue;
		}

		// Validate required fields
		if (!MessageObj->HasField(TEXT("id")))
		{
			UE_LOG(LogTemp, Warning, TEXT("DiscordAPI: Message missing 'id' field, skipping"));
			continue;
		}
		
		FString MessageId = MessageObj->GetStringField(TEXT("id"));
		
		if (!MessageObj->HasField(TEXT("content")))
		{
			UE_LOG(LogTemp, Warning, TEXT("DiscordAPI: Message missing 'content' field, skipping"));
			continue;
		}
		
		FString Content = MessageObj->GetStringField(TEXT("content"));
		
		// Check if message is from a bot (ignore bot messages to prevent loops)
		if (!MessageObj->HasField(TEXT("author")))
		{
			UE_LOG(LogTemp, Warning, TEXT("DiscordAPI: Message missing 'author' field, skipping"));
			continue;
		}
		
		TSharedPtr<FJsonObject> AuthorObj = MessageObj->GetObjectField(TEXT("author"));
		if (!AuthorObj.IsValid())
		{
			UE_LOG(LogTemp, Warning, TEXT("DiscordAPI: Invalid author object, skipping message"));
			continue;
		}
		
		// Check if 'bot' field exists
		if (!AuthorObj->HasField(TEXT("bot")))
		{
			UE_LOG(LogTemp, Warning, TEXT("DiscordAPI: Author missing 'bot' field, assuming not a bot"));
		}
		else
		{
			bool bIsBot = AuthorObj->GetBoolField(TEXT("bot"));
			if (bIsBot)
			{
				// Update last message ID but don't process bot messages
				LastMessageId = MessageId;
				continue;
			}
		}

		// Check if 'username' field exists
		if (!AuthorObj->HasField(TEXT("username")))
		{
			UE_LOG(LogTemp, Warning, TEXT("DiscordAPI: Author missing 'username' field, skipping message"));
			continue;
		}
		
		FString Username = AuthorObj->GetStringField(TEXT("username"));
		
		// Update the last message ID
		LastMessageId = MessageId;

		// Call the delegate
		if (!Content.IsEmpty() && OnMessageReceived.IsBound())
		{
			OnMessageReceived.Execute(Username, Content);
		}
	}
}

void UDiscordAPI::StartActivityUpdates()
{
	if (!bIsInitialized)
	{
		UE_LOG(LogTemp, Warning, TEXT("DiscordAPI: Cannot start activity updates - API not initialized"));
		return;
	}

	if (!BotConfig.bEnableBotActivity)
	{
		UE_LOG(LogTemp, Log, TEXT("DiscordAPI: Bot activity is disabled in config"));
		return;
	}

	if (bIsUpdatingActivity)
	{
		UE_LOG(LogTemp, Warning, TEXT("DiscordAPI: Already updating activity"));
		return;
	}

	bIsUpdatingActivity = true;
	UE_LOG(LogTemp, Log, TEXT("DiscordAPI: Started bot activity updates"));
}

void UDiscordAPI::StopActivityUpdates()
{
	if (!bIsUpdatingActivity)
	{
		return;
	}

	bIsUpdatingActivity = false;
	UE_LOG(LogTemp, Log, TEXT("DiscordAPI: Stopped bot activity updates"));

	UWorld* World = GetWorld();
	if (World)
	{
		World->GetTimerManager().ClearTimer(ActivityUpdateTimerHandle);
	}

	// Set bot to offline before disconnecting Gateway
	if (Gateway && Gateway->IsConnected())
	{
		UE_LOG(LogTemp, Log, TEXT("DiscordAPI: Setting bot status to offline before disconnecting"));
		Gateway->SetOfflineStatus();
		
		// Give the offline status message a moment to be sent before disconnecting
		// Use a small delay to ensure the message is transmitted
		if (World)
		{
			FTimerHandle DisconnectTimer;
			World->GetTimerManager().SetTimer(DisconnectTimer, [this]()
			{
				if (Gateway)
				{
					Gateway->Disconnect();
				}
			}, 0.5f, false);
		}
		else
		{
			Gateway->Disconnect();
		}
	}
	else if (Gateway)
	{
		Gateway->Disconnect();
	}
}

void UDiscordAPI::UpdateBotActivity(int32 PlayerCount)
{
	if (!bIsInitialized || !BotConfig.bEnableBotActivity)
	{
		return;
	}

	// Format the activity message
	FString ActivityMessage = BotConfig.BotActivityFormat;
	ActivityMessage = ActivityMessage.Replace(TEXT("{playercount}"), *FString::FromInt(PlayerCount));

	// Use Gateway for presence if enabled and connected
	if (BotConfig.bUseGatewayForPresence && Gateway && Gateway->IsConnected())
	{
		// Update bot presence via Gateway using configurable format and activity type
		FString PresenceText = BotConfig.GatewayPresenceFormat;
		PresenceText = PresenceText.Replace(TEXT("{playercount}"), *FString::FromInt(PlayerCount));
		
		Gateway->UpdatePresence(PresenceText, BotConfig.GatewayActivityType);
		UE_LOG(LogTemp, Log, TEXT("DiscordAPI: Updated bot presence via Gateway: Type=%d, Text=%s"), 
			BotConfig.GatewayActivityType, *PresenceText);
		return;
	}

	// Fallback to REST API (post to channel)
	// Determine which channel to use for activity status
	FString TargetChannelId = BotConfig.BotActivityChannelId.IsEmpty() ? BotConfig.ChannelId : BotConfig.BotActivityChannelId;

	// Create the HTTP request to post player count status
	TSharedRef<IHttpRequest, ESPMode::ThreadSafe> HttpRequest = FHttpModule::Get().CreateRequest();
	
	FString Url = FString::Printf(TEXT("https://discord.com/api/v10/channels/%s/messages"), *TargetChannelId);
	HttpRequest->SetURL(Url);
	HttpRequest->SetVerb(TEXT("POST"));
	HttpRequest->SetHeader(TEXT("Authorization"), FString::Printf(TEXT("Bot %s"), *BotConfig.BotToken));
	HttpRequest->SetHeader(TEXT("Content-Type"), TEXT("application/json"));

	// Create JSON payload
	TSharedPtr<FJsonObject> JsonObject = MakeShareable(new FJsonObject());
	JsonObject->SetStringField(TEXT("content"), ActivityMessage);

	FString JsonString;
	TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&JsonString);
	FJsonSerializer::Serialize(JsonObject.ToSharedRef(), Writer);

	HttpRequest->SetContentAsString(JsonString);
	HttpRequest->OnProcessRequestComplete().BindUObject(this, &UDiscordAPI::OnUpdateActivityResponse);

	if (HttpRequest->ProcessRequest())
	{
		UE_LOG(LogTemp, Log, TEXT("DiscordAPI: Posting player count status: %s"), *ActivityMessage);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("DiscordAPI: Failed to process activity status request"));
	}
}

void UDiscordAPI::OnUpdateActivityResponse(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
{
	if (bWasSuccessful && Response.IsValid())
	{
		int32 ResponseCode = Response->GetResponseCode();
		if (ResponseCode >= 200 && ResponseCode < 300)
		{
			UE_LOG(LogTemp, Log, TEXT("DiscordAPI: Bot activity updated successfully"));
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("DiscordAPI: Failed to update bot activity - Response code: %d, Body: %s"), 
				ResponseCode, *Response->GetContentAsString());
		}
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("DiscordAPI: Failed to update bot activity - Request failed"));
	}
}

void UDiscordAPI::OnGatewayConnected()
{
	UE_LOG(LogTemp, Log, TEXT("DiscordAPI: Gateway connected successfully"));
}

void UDiscordAPI::OnGatewayDisconnected(const FString& Reason)
{
	UE_LOG(LogTemp, Warning, TEXT("DiscordAPI: Gateway disconnected: %s"), *Reason);
	
	// Optionally implement auto-reconnect logic here
	// For now, just log the disconnection
}

