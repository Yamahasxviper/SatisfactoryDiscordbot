// Copyright Epic Games, Inc. All Rights Reserved.

#include "DiscordBotSubsystem.h"
#include "DiscordBotConfig.h"

#include "FGChatManager.h"
#include "HttpModule.h"
#include "Interfaces/IHttpRequest.h"
#include "Interfaces/IHttpResponse.h"
#include "Json.h"
#include "Engine/World.h"
#include "TimerManager.h"

DEFINE_LOG_CATEGORY_STATIC(LogDiscordBot, Log, All);

// Discord REST API base URL
static const TCHAR* DiscordApiBase = TEXT("https://discord.com/api/v10");

ADiscordBotSubsystem::ADiscordBotSubsystem()
{
	ReplicationPolicy         = ESubsystemReplicationPolicy::SpawnOnServer;
	LastProcessedMessageCount = 0;
	bInjecting                = false;
}

void ADiscordBotSubsystem::BeginPlay()
{
	Super::BeginPlay();

	const UDiscordBotConfig* Config = UDiscordBotConfig::Get();

	if (Config->BotToken.IsEmpty() || Config->ChannelId.IsEmpty())
	{
		UE_LOG(LogDiscordBot, Warning,
			TEXT("DiscordBot: BotToken or ChannelId is not configured – bridge is disabled. "
			     "Set them in <GameDir>/Configs/DiscordBot.ini."));
		return;
	}

	// Bind to the chat manager so we can relay player messages to Discord.
	AFGChatManager* ChatManager = AFGChatManager::Get(GetWorld());
	if (ChatManager)
	{
		// Record how many messages are already in history so we do not forward them.
		TArray<FChatMessageStruct> Existing;
		ChatManager->GetReceivedChatMessages(Existing);
		LastProcessedMessageCount = Existing.Num();

		ChatManager->OnChatMessageAdded.AddDynamic(this, &ADiscordBotSubsystem::OnChatMessageAdded);
	}
	else
	{
		UE_LOG(LogDiscordBot, Warning,
			TEXT("DiscordBot: AFGChatManager not found – game-to-Discord relay disabled."));
	}

	// Start the Discord polling loop.
	const float Interval = FMath::Max(2.0f, Config->PollingIntervalSeconds);
	GetWorldTimerManager().SetTimer(
		PollingTimerHandle,
		this,
		&ADiscordBotSubsystem::PollDiscordMessages,
		Interval,
		/*bLoop=*/ true,
		/*FirstDelay=*/ Interval);

	UE_LOG(LogDiscordBot, Log,
		TEXT("DiscordBot: Bridge active (channel %s, poll every %.1f s)."),
		*Config->ChannelId, Interval);
}

void ADiscordBotSubsystem::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	GetWorldTimerManager().ClearTimer(PollingTimerHandle);

	AFGChatManager* ChatManager = AFGChatManager::Get(GetWorld());
	if (ChatManager)
	{
		ChatManager->OnChatMessageAdded.RemoveDynamic(this, &ADiscordBotSubsystem::OnChatMessageAdded);
	}

	Super::EndPlay(EndPlayReason);
}

// ---------------------------------------------------------------------------
// Game → Discord
// ---------------------------------------------------------------------------

void ADiscordBotSubsystem::OnChatMessageAdded()
{
	// Skip messages we injected from Discord to prevent echo loops.
	if (bInjecting)
	{
		return;
	}

	AFGChatManager* ChatManager = AFGChatManager::Get(GetWorld());
	if (!ChatManager)
	{
		return;
	}

	TArray<FChatMessageStruct> Messages;
	ChatManager->GetReceivedChatMessages(Messages);

	// Forward any newly appended player messages to Discord.
	for (int32 i = LastProcessedMessageCount; i < Messages.Num(); ++i)
	{
		const FChatMessageStruct& Msg = Messages[i];
		if (Msg.MessageType == EFGChatMessageType::CMT_PlayerMessage)
		{
			ForwardToDiscord(Msg.MessageSender.ToString(), Msg.MessageText.ToString());
		}
	}

	LastProcessedMessageCount = Messages.Num();
}

void ADiscordBotSubsystem::ForwardToDiscord(const FString& SenderName, const FString& MessageText)
{
	const UDiscordBotConfig* Config = UDiscordBotConfig::Get();
	if (Config->BotToken.IsEmpty() || Config->ChannelId.IsEmpty())
	{
		return;
	}

	// Format: "[PlayerName]: message text"
	const FString Content =
		FString::Printf(TEXT("[%s]: %s"), *SenderName, *MessageText);

	const FString Url =
		FString::Printf(TEXT("%s/channels/%s/messages"), DiscordApiBase, *Config->ChannelId);

	TSharedPtr<FJsonObject> JsonObj = MakeShared<FJsonObject>();
	JsonObj->SetStringField(TEXT("content"), Content);

	FString BodyString;
	TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&BodyString);
	FJsonSerializer::Serialize(JsonObj.ToSharedRef(), Writer);

	TSharedRef<IHttpRequest, ESPMode::ThreadSafe> HttpReq = FHttpModule::Get().CreateRequest();
	HttpReq->SetURL(Url);
	HttpReq->SetVerb(TEXT("POST"));
	HttpReq->SetHeader(TEXT("Authorization"), GetAuthHeader());
	HttpReq->SetHeader(TEXT("Content-Type"), TEXT("application/json"));
	HttpReq->SetContentAsString(BodyString);
	HttpReq->OnProcessRequestComplete().BindLambda(
		[](FHttpRequestPtr, FHttpResponsePtr Response, bool bSuccess)
		{
			if (!bSuccess || !Response.IsValid())
			{
				UE_LOG(LogDiscordBot, Warning,
					TEXT("DiscordBot: Failed to POST message to Discord."));
			}
			else if (Response->GetResponseCode() < 200 || Response->GetResponseCode() >= 300)
			{
				UE_LOG(LogDiscordBot, Warning,
					TEXT("DiscordBot: Discord returned HTTP %d when posting message."),
					Response->GetResponseCode());
			}
		});
	HttpReq->ProcessRequest();
}

// ---------------------------------------------------------------------------
// Discord → Game
// ---------------------------------------------------------------------------

void ADiscordBotSubsystem::PollDiscordMessages()
{
	const UDiscordBotConfig* Config = UDiscordBotConfig::Get();
	if (Config->BotToken.IsEmpty() || Config->ChannelId.IsEmpty())
	{
		return;
	}

	FString Url =
		FString::Printf(TEXT("%s/channels/%s/messages"), DiscordApiBase, *Config->ChannelId);

	if (LastSeenMessageId.IsEmpty())
	{
		// First poll: fetch just the latest message to initialise our cursor.
		// We do not display anything – we only want messages posted AFTER startup.
		Url += TEXT("?limit=1");
	}
	else
	{
		// Subsequent polls: only retrieve messages newer than the last one we saw.
		// Discord returns them in ascending (oldest-first) order when using after=.
		Url += FString::Printf(TEXT("?limit=50&after=%s"), *LastSeenMessageId);
	}

	TSharedRef<IHttpRequest, ESPMode::ThreadSafe> HttpReq = FHttpModule::Get().CreateRequest();
	HttpReq->SetURL(Url);
	HttpReq->SetVerb(TEXT("GET"));
	HttpReq->SetHeader(TEXT("Authorization"), GetAuthHeader());
	HttpReq->OnProcessRequestComplete().BindUObject(this, &ADiscordBotSubsystem::HandlePollResponse);
	HttpReq->ProcessRequest();
}

void ADiscordBotSubsystem::HandlePollResponse(
	FHttpRequestPtr /*Request*/,
	FHttpResponsePtr Response,
	bool bWasSuccessful)
{
	if (!bWasSuccessful || !Response.IsValid())
	{
		UE_LOG(LogDiscordBot, Warning,
			TEXT("DiscordBot: Discord poll request failed."));
		return;
	}

	if (Response->GetResponseCode() != 200)
	{
		UE_LOG(LogDiscordBot, Warning,
			TEXT("DiscordBot: Discord returned HTTP %d during poll."),
			Response->GetResponseCode());
		return;
	}

	TSharedPtr<FJsonValue> RootValue;
	TSharedRef<TJsonReader<>> Reader =
		TJsonReaderFactory<>::Create(Response->GetContentAsString());

	if (!FJsonSerializer::Deserialize(Reader, RootValue) || !RootValue.IsValid())
	{
		UE_LOG(LogDiscordBot, Warning,
			TEXT("DiscordBot: Failed to parse Discord poll response."));
		// On first poll, mark as initialised so we do not retry the same path.
		if (LastSeenMessageId.IsEmpty())
		{
			LastSeenMessageId = TEXT("0");
		}
		return;
	}

	const TArray<TSharedPtr<FJsonValue>>* MessageArrayPtr = nullptr;
	if (!RootValue->TryGetArray(MessageArrayPtr) || !MessageArrayPtr || MessageArrayPtr->IsEmpty())
	{
		// Empty array – no new messages.
		if (LastSeenMessageId.IsEmpty())
		{
			LastSeenMessageId = TEXT("0");
		}
		return;
	}

	const TArray<TSharedPtr<FJsonValue>>& MessageArray = *MessageArrayPtr;

	if (LastSeenMessageId.IsEmpty())
	{
		// First poll (limit=1, descending order): grab the newest ID as our cursor.
		const TSharedPtr<FJsonObject>* MsgObj = nullptr;
		if (MessageArray[0]->TryGetObject(MsgObj) && MsgObj)
		{
			(*MsgObj)->TryGetStringField(TEXT("id"), LastSeenMessageId);
		}
		if (LastSeenMessageId.IsEmpty())
		{
			LastSeenMessageId = TEXT("0");
		}
		// Do NOT display anything – we only show messages posted after startup.
		return;
	}

	// Subsequent polls: messages arrive in ascending order (after= query).
	// Display each non-bot message in-game.
	for (const TSharedPtr<FJsonValue>& MsgValue : MessageArray)
	{
		const TSharedPtr<FJsonObject>* MsgObj = nullptr;
		if (!MsgValue->TryGetObject(MsgObj) || !MsgObj)
		{
			continue;
		}

		FString MessageId;
		(*MsgObj)->TryGetStringField(TEXT("id"), MessageId);

		// Skip messages authored by a bot (includes our own bot's echoes).
		const TSharedPtr<FJsonObject>* AuthorObj = nullptr;
		bool bIsBot = false;
		if ((*MsgObj)->TryGetObjectField(TEXT("author"), AuthorObj) && AuthorObj)
		{
			(*AuthorObj)->TryGetBoolField(TEXT("bot"), bIsBot);
		}

		if (!bIsBot)
		{
			FString AuthorName;
			if (AuthorObj)
			{
				(*AuthorObj)->TryGetStringField(TEXT("username"), AuthorName);
			}

			FString Content;
			(*MsgObj)->TryGetStringField(TEXT("content"), Content);

			if (!Content.IsEmpty())
			{
				DisplayInGame(AuthorName, Content);
			}
		}

		// Always advance the cursor, even for bot messages.
		if (!MessageId.IsEmpty())
		{
			LastSeenMessageId = MessageId;
		}
	}
}

void ADiscordBotSubsystem::DisplayInGame(const FString& AuthorName, const FString& Content)
{
	AFGChatManager* ChatManager = AFGChatManager::Get(GetWorld());
	if (!ChatManager)
	{
		return;
	}

	const UDiscordBotConfig* Config = UDiscordBotConfig::Get();
	const FString DisplayName = Config->InGameDisplayName.IsEmpty()
		? TEXT("Discord")
		: Config->InGameDisplayName;

	FChatMessageStruct Msg;
	Msg.MessageType = EFGChatMessageType::CMT_CustomMessage;

	// Sender label: "DisplayName (AuthorName)" or just "DisplayName" if no name available.
	Msg.MessageSender = AuthorName.IsEmpty()
		? FText::FromString(DisplayName)
		: FText::FromString(FString::Printf(TEXT("%s (%s)"), *DisplayName, *AuthorName));

	Msg.MessageText        = FText::FromString(Content);
	// Use a soft indigo tint to visually distinguish Discord messages in chat.
	Msg.MessageSenderColor = FLinearColor(0.4f, 0.5f, 0.95f);

	// Guard against echoing this injected message back to Discord.
	bInjecting = true;
	ChatManager->BroadcastChatMessage(Msg);
	bInjecting = false;

	// Update our cursor so OnChatMessageAdded skips this message correctly.
	TArray<FChatMessageStruct> Messages;
	ChatManager->GetReceivedChatMessages(Messages);
	LastProcessedMessageCount = Messages.Num();
}

FString ADiscordBotSubsystem::GetAuthHeader() const
{
	return FString::Printf(TEXT("Bot %s"), *UDiscordBotConfig::Get()->BotToken);
}
