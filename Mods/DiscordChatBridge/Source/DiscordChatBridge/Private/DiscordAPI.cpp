// Copyright (c) 2024 Discord Chat Bridge Contributors

#include "DiscordAPI.h"
#include "HttpModule.h"
#include "Interfaces/IHttpResponse.h"
#include "Json.h"
#include "JsonUtilities.h"
#include "TimerManager.h"
#include "Engine/World.h"

UDiscordAPI::UDiscordAPI()
	: bIsInitialized(false)
	, bIsPolling(false)
{
}

void UDiscordAPI::Initialize(const FDiscordBotConfig& Config)
{
	BotConfig = Config;
	bIsInitialized = !BotConfig.BotToken.IsEmpty() && !BotConfig.ChannelId.IsEmpty();

	if (bIsInitialized)
	{
		UE_LOG(LogTemp, Log, TEXT("DiscordAPI: Initialized with channel ID: %s"), *BotConfig.ChannelId);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("DiscordAPI: Failed to initialize - missing bot token or channel ID"));
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

		FString MessageId = MessageObj->GetStringField(TEXT("id"));
		FString Content = MessageObj->GetStringField(TEXT("content"));
		
		// Check if message is from a bot (ignore bot messages to prevent loops)
		TSharedPtr<FJsonObject> AuthorObj = MessageObj->GetObjectField(TEXT("author"));
		if (AuthorObj.IsValid())
		{
			bool bIsBot = AuthorObj->GetBoolField(TEXT("bot"));
			if (bIsBot)
			{
				// Update last message ID but don't process bot messages
				LastMessageId = MessageId;
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
}
