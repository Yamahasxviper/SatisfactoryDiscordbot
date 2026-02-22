// Copyright Coffee Stain Studios. All Rights Reserved.

#include "DiscordBridgeSubsystem.h"

#include "Dom/JsonObject.h"
#include "Dom/JsonValue.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonWriter.h"
#include "Serialization/JsonSerializer.h"
#include "HttpModule.h"
#include "Interfaces/IHttpRequest.h"
#include "Interfaces/IHttpResponse.h"
#include "FGChatManager.h"

// Discord REST API base URL (v10)
static const FString DiscordApiBase = TEXT("https://discord.com/api/v10");

// ─────────────────────────────────────────────────────────────────────────────
// USubsystem lifetime
// ─────────────────────────────────────────────────────────────────────────────

bool UDiscordBridgeSubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
	// Run on dedicated servers only; clients never need the bridge running locally.
	return IsRunningDedicatedServer();
}

void UDiscordBridgeSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	// Wire up the Discord→game relay once so it survives reconnect cycles.
	OnDiscordMessageReceived.AddDynamic(this, &UDiscordBridgeSubsystem::RelayDiscordMessageToGame);

	// Poll for AFGChatManager once per second until it becomes available, then
	// bind OnNewChatMessage so in-game chat is forwarded to Discord.
	ChatManagerBindTickerHandle = FTSTicker::GetCoreTicker().AddTicker(
		FTickerDelegate::CreateWeakLambda(this, [this](float) -> bool
		{
			UWorld* World = GetWorld();
			if (!World) return true;

			AFGChatManager* CM = AFGChatManager::Get(World);
			if (!CM) return true;

			TArray<FChatMessageStruct> Existing;
			CM->GetReceivedChatMessages(Existing);
			NumSeenChatMessages = Existing.Num();

			CM->OnChatMessageAdded.AddDynamic(this, &UDiscordBridgeSubsystem::OnNewChatMessage);
			BoundChatManager = CM;

			UE_LOG(LogTemp, Log, TEXT("DiscordBridge: Bound to AFGChatManager::OnChatMessageAdded."));
			return false; // stop polling
		}),
		1.0f);

	Config = FDiscordBridgeConfig::LoadOrCreate();

	if (Config.BotToken.IsEmpty() || Config.ChannelId.IsEmpty())
	{
		UE_LOG(LogTemp, Warning,
		       TEXT("DiscordBridge: BotToken or ChannelId is not configured. "
		            "Edit Saved/Config/DiscordBridge.ini to enable the bridge."));
		return;
	}

	Connect();
}

void UDiscordBridgeSubsystem::Deinitialize()
{
	FTSTicker::GetCoreTicker().RemoveTicker(ChatManagerBindTickerHandle);
	ChatManagerBindTickerHandle.Reset();

	if (IsValid(BoundChatManager))
	{
		BoundChatManager->OnChatMessageAdded.RemoveDynamic(this, &UDiscordBridgeSubsystem::OnNewChatMessage);
		BoundChatManager = nullptr;
	}

	Disconnect();
	Super::Deinitialize();
}

// ─────────────────────────────────────────────────────────────────────────────
// Connection management (REST-poll lifecycle)
// ─────────────────────────────────────────────────────────────────────────────

void UDiscordBridgeSubsystem::Connect()
{
	if (bPollingActive)
	{
		return;
	}

	UE_LOG(LogTemp, Log,
	       TEXT("DiscordBridge: Starting Discord REST poll (interval %.1f s)."),
	       Config.PollIntervalSeconds);

	// Step 1 – fetch own user ID so we can suppress echo of the bot's own posts.
	// Step 2 (baseline) starts inside the FetchBotUserId callback.
	FetchBotUserId();
}

void UDiscordBridgeSubsystem::Disconnect()
{
	FTSTicker::GetCoreTicker().RemoveTicker(PollTickerHandle);
	PollTickerHandle.Reset();

	const bool bWasActive = bPollingActive;
	bPollingActive        = false;
	bBaselineEstablished  = false;
	LastMessageId.Empty();

	if (bWasActive)
	{
		// Post server-offline message before we stop.
		if (!Config.ServerOfflineMessage.IsEmpty())
		{
			SendStatusMessageToDiscord(Config.ServerOfflineMessage);
		}
		OnDiscordDisconnected.Broadcast(TEXT("Polling stopped."));
	}
}

// ─────────────────────────────────────────────────────────────────────────────
// Step 1 – fetch bot user ID
// ─────────────────────────────────────────────────────────────────────────────

void UDiscordBridgeSubsystem::FetchBotUserId()
{
	const FString Url = FString::Printf(TEXT("%s/users/@me"), *DiscordApiBase);
	TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request = MakeApiRequest(TEXT("GET"), Url);

	Request->OnProcessRequestComplete().BindWeakLambda(
		this,
		[this](FHttpRequestPtr /*Req*/, FHttpResponsePtr Resp, bool bConnected)
		{
			if (bConnected && Resp.IsValid() && Resp->GetResponseCode() == 200)
			{
				TSharedPtr<FJsonObject> UserObj;
				TSharedRef<TJsonReader<>> Reader =
					TJsonReaderFactory<>::Create(Resp->GetContentAsString());
				if (FJsonSerializer::Deserialize(Reader, UserObj) && UserObj.IsValid())
				{
					UserObj->TryGetStringField(TEXT("id"), BotUserId);
				}
				UE_LOG(LogTemp, Log,
				       TEXT("DiscordBridge: Bot user ID: %s"), *BotUserId);
			}
			else
			{
				const int32 Code = (Resp.IsValid()) ? Resp->GetResponseCode() : 0;
				UE_LOG(LogTemp, Warning,
				       TEXT("DiscordBridge: Could not fetch bot user ID (HTTP %d). "
				            "Self-message filtering will fall back to the bot flag."), Code);
			}

			// Proceed to step 2 regardless – a missing user ID is not fatal.
			FetchBaselineMessageId();
		});

	Request->ProcessRequest();
}

// ─────────────────────────────────────────────────────────────────────────────
// Step 2 – establish the baseline message ID
// ─────────────────────────────────────────────────────────────────────────────

void UDiscordBridgeSubsystem::FetchBaselineMessageId()
{
	// Fetch the single most-recent message so we know where history ends.
	// Any message with a snowflake older than or equal to this ID already existed
	// before the server started and must NOT be relayed to in-game chat.
	const FString Url = FString::Printf(
		TEXT("%s/channels/%s/messages?limit=1"), *DiscordApiBase, *Config.ChannelId);

	TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request = MakeApiRequest(TEXT("GET"), Url);

	Request->OnProcessRequestComplete().BindWeakLambda(
		this,
		[this](FHttpRequestPtr /*Req*/, FHttpResponsePtr Resp, bool bConnected)
		{
			if (bConnected && Resp.IsValid() && Resp->GetResponseCode() == 200)
			{
				TArray<TSharedPtr<FJsonValue>> Messages;
				TSharedRef<TJsonReader<>> Reader =
					TJsonReaderFactory<>::Create(Resp->GetContentAsString());
				if (FJsonSerializer::Deserialize(Reader, Messages) && Messages.Num() > 0)
				{
					const TSharedPtr<FJsonObject>* MsgPtr = nullptr;
					if (Messages[0]->TryGetObject(MsgPtr) && MsgPtr)
					{
						(*MsgPtr)->TryGetStringField(TEXT("id"), LastMessageId);
					}
				}

				if (LastMessageId.IsEmpty())
				{
					// Channel has no messages yet – use a snowflake for "right now"
					// so we only process messages posted after server start.
					// Discord epoch: 2015-01-01T00:00:00.000Z = 1420070400000 ms
					static constexpr int64 DiscordEpochMs = 1420070400000LL;
					const int64 NowMs =
						FDateTime::UtcNow().ToUnixTimestamp() * 1000LL;
					LastMessageId = FString::Printf(
						TEXT("%lld"), (NowMs - DiscordEpochMs) << 22);
				}

				UE_LOG(LogTemp, Log,
				       TEXT("DiscordBridge: Baseline message ID = %s"), *LastMessageId);
			}
			else
			{
				const int32 Code = (Resp.IsValid()) ? Resp->GetResponseCode() : 0;
				UE_LOG(LogTemp, Warning,
				       TEXT("DiscordBridge: Baseline fetch returned HTTP %d – "
				            "polling will start from now."), Code);

				// Same fallback: synthesise a "now" snowflake.
				static constexpr int64 DiscordEpochMs = 1420070400000LL;
				const int64 NowMs =
					FDateTime::UtcNow().ToUnixTimestamp() * 1000LL;
				LastMessageId = FString::Printf(
					TEXT("%lld"), (NowMs - DiscordEpochMs) << 22);
			}

			bBaselineEstablished = true;
			bPollingActive       = true;

			// Send server-online notification now that we know the token is valid.
			if (!Config.ServerOnlineMessage.IsEmpty())
			{
				SendStatusMessageToDiscord(Config.ServerOnlineMessage);
			}

			// Signal Blueprint listeners that the bridge is live.
			OnDiscordConnected.Broadcast();

			// Start the repeating poll ticker.
			PollTickerHandle = FTSTicker::GetCoreTicker().AddTicker(
				FTickerDelegate::CreateUObject(this, &UDiscordBridgeSubsystem::PollTick),
				Config.PollIntervalSeconds);

			UE_LOG(LogTemp, Log,
			       TEXT("DiscordBridge: Polling started (every %.1f s)."),
			       Config.PollIntervalSeconds);
		});

	Request->ProcessRequest();
}

// ─────────────────────────────────────────────────────────────────────────────
// Repeating poll
// ─────────────────────────────────────────────────────────────────────────────

bool UDiscordBridgeSubsystem::PollTick(float /*DeltaTime*/)
{
	if (bBaselineEstablished)
	{
		PollNewMessages();
	}
	return true; // keep ticking
}

void UDiscordBridgeSubsystem::PollNewMessages()
{
	// GET /channels/{id}/messages?after={last_id}&limit=100
	// Discord returns messages newer than last_id in ascending (oldest-first) order.
	const FString Url = FString::Printf(
		TEXT("%s/channels/%s/messages?after=%s&limit=100"),
		*DiscordApiBase, *Config.ChannelId, *LastMessageId);

	TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request = MakeApiRequest(TEXT("GET"), Url);

	Request->OnProcessRequestComplete().BindWeakLambda(
		this,
		[this](FHttpRequestPtr /*Req*/, FHttpResponsePtr Resp, bool bConnected)
		{
			if (!bConnected || !Resp.IsValid())
			{
				UE_LOG(LogTemp, Warning,
				       TEXT("DiscordBridge: Poll HTTP request failed (network error)."));
				return;
			}

			const int32 Code = Resp->GetResponseCode();
			if (Code == 401)
			{
				UE_LOG(LogTemp, Error,
				       TEXT("DiscordBridge: Poll returned 401 Unauthorized. "
				            "Verify BotToken in Saved/Config/DiscordBridge.ini. "
				            "Stopping poll."));
				Disconnect();
				return;
			}
			if (Code == 403)
			{
				UE_LOG(LogTemp, Error,
				       TEXT("DiscordBridge: Poll returned 403 Forbidden. "
				            "Ensure the bot has 'Read Message History' permission "
				            "and Message Content Intent is enabled. Stopping poll."));
				Disconnect();
				return;
			}
			if (Code != 200)
			{
				UE_LOG(LogTemp, Warning,
				       TEXT("DiscordBridge: Poll returned HTTP %d: %s"),
				       Code, *Resp->GetContentAsString());
				return; // transient error – retry next tick
			}

			TArray<TSharedPtr<FJsonValue>> Messages;
			TSharedRef<TJsonReader<>> Reader =
				TJsonReaderFactory<>::Create(Resp->GetContentAsString());
			if (!FJsonSerializer::Deserialize(Reader, Messages))
			{
				UE_LOG(LogTemp, Warning, TEXT("DiscordBridge: Failed to parse poll response JSON."));
				return;
			}

			const FString NewestId = ProcessMessageArray(Messages);
			if (!NewestId.IsEmpty())
			{
				LastMessageId = NewestId;
			}
		});

	Request->ProcessRequest();
}

FString UDiscordBridgeSubsystem::ProcessMessageArray(
	const TArray<TSharedPtr<FJsonValue>>& Messages)
{
	// Messages arrive in ascending snowflake order (oldest → newest).
	// We process all of them in order and track the largest ID seen.
	FString NewestId;

	for (const TSharedPtr<FJsonValue>& MsgVal : Messages)
	{
		const TSharedPtr<FJsonObject>* MsgPtr = nullptr;
		if (!MsgVal.IsValid() || !MsgVal->TryGetObject(MsgPtr) || !MsgPtr)
		{
			continue;
		}
		const TSharedPtr<FJsonObject>& Msg = *MsgPtr;

		// Always track the newest ID regardless of whether we process the message.
		FString MsgId;
		Msg->TryGetStringField(TEXT("id"), MsgId);
		if (!MsgId.IsEmpty())
		{
			NewestId = MsgId;
		}

		// ── Author filtering ──────────────────────────────────────────────────

		const TSharedPtr<FJsonObject>* AuthorPtr = nullptr;
		if (!Msg->TryGetObjectField(TEXT("author"), AuthorPtr) || !AuthorPtr)
		{
			continue;
		}
		const TSharedPtr<FJsonObject>& Author = *AuthorPtr;

		// Skip self-sent messages (bot's own posts) to prevent echo loops.
		FString AuthorId;
		Author->TryGetStringField(TEXT("id"), AuthorId);
		if (!BotUserId.IsEmpty() && AuthorId == BotUserId)
		{
			continue;
		}

		// Optionally skip all bot-flagged accounts.
		if (Config.bIgnoreBotMessages)
		{
			bool bIsBot = false;
			Author->TryGetBoolField(TEXT("bot"), bIsBot);
			if (bIsBot)
			{
				continue;
			}
		}

		// ── Content ───────────────────────────────────────────────────────────

		FString Content;
		Msg->TryGetStringField(TEXT("content"), Content);
		if (Content.IsEmpty())
		{
			// Embed-only or attachment-only messages, or Message Content Intent
			// is not enabled on the bot account (content will be empty string).
			continue;
		}

		// ── Display name ──────────────────────────────────────────────────────
		// Prefer global_name (Discord username 2.0 display name), fall back to username.

		FString Username;
		if (!Author->TryGetStringField(TEXT("global_name"), Username) || Username.IsEmpty())
		{
			Author->TryGetStringField(TEXT("username"), Username);
		}

		UE_LOG(LogTemp, Log,
		       TEXT("DiscordBridge: [%s] %s"), *Username, *Content);

		OnDiscordMessageReceived.Broadcast(Username, Content);
	}

	return NewestId;
}

// ─────────────────────────────────────────────────────────────────────────────
// REST API – sending
// ─────────────────────────────────────────────────────────────────────────────

void UDiscordBridgeSubsystem::SendGameMessageToDiscord(const FString& PlayerName,
                                                        const FString& Message)
{
	if (Config.BotToken.IsEmpty() || Config.ChannelId.IsEmpty())
	{
		UE_LOG(LogTemp, Warning,
		       TEXT("DiscordBridge: Cannot send – BotToken or ChannelId not configured."));
		return;
	}

	FString FormattedContent = Config.GameToDiscordFormat;
	FormattedContent = FormattedContent.Replace(TEXT("{PlayerName}"), *PlayerName);
	FormattedContent = FormattedContent.Replace(TEXT("{Message}"),    *Message);

	TSharedPtr<FJsonObject> Body = MakeShared<FJsonObject>();
	Body->SetStringField(TEXT("content"), FormattedContent);

	FString BodyString;
	TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&BodyString);
	FJsonSerializer::Serialize(Body.ToSharedRef(), Writer);

	const FString Url = FString::Printf(
		TEXT("%s/channels/%s/messages"), *DiscordApiBase, *Config.ChannelId);

	TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request = MakeApiRequest(TEXT("POST"), Url);
	Request->SetHeader(TEXT("Content-Type"), TEXT("application/json"));
	Request->SetContentAsString(BodyString);

	Request->OnProcessRequestComplete().BindWeakLambda(
		this,
		[PlayerName](FHttpRequestPtr, FHttpResponsePtr Resp, bool bConnected)
		{
			if (!bConnected || !Resp.IsValid())
			{
				UE_LOG(LogTemp, Warning,
				       TEXT("DiscordBridge: HTTP send failed for player '%s'."), *PlayerName);
				return;
			}
			if (Resp->GetResponseCode() < 200 || Resp->GetResponseCode() >= 300)
			{
				UE_LOG(LogTemp, Warning,
				       TEXT("DiscordBridge: Discord REST API returned %d: %s"),
				       Resp->GetResponseCode(), *Resp->GetContentAsString());
			}
		});

	Request->ProcessRequest();
}

void UDiscordBridgeSubsystem::SendStatusMessageToDiscord(const FString& Message)
{
	if (Config.BotToken.IsEmpty() || Config.ChannelId.IsEmpty())
	{
		return;
	}

	TSharedPtr<FJsonObject> Body = MakeShared<FJsonObject>();
	Body->SetStringField(TEXT("content"), Message);

	FString BodyString;
	TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&BodyString);
	FJsonSerializer::Serialize(Body.ToSharedRef(), Writer);

	const FString Url = FString::Printf(
		TEXT("%s/channels/%s/messages"), *DiscordApiBase, *Config.ChannelId);

	TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request = MakeApiRequest(TEXT("POST"), Url);
	Request->SetHeader(TEXT("Content-Type"), TEXT("application/json"));
	Request->SetContentAsString(BodyString);

	Request->OnProcessRequestComplete().BindWeakLambda(
		this,
		[Message](FHttpRequestPtr, FHttpResponsePtr Resp, bool bConnected)
		{
			if (!bConnected || !Resp.IsValid())
			{
				UE_LOG(LogTemp, Warning,
				       TEXT("DiscordBridge: HTTP send failed for status message '%s'."),
				       *Message);
				return;
			}
			if (Resp->GetResponseCode() < 200 || Resp->GetResponseCode() >= 300)
			{
				UE_LOG(LogTemp, Warning,
				       TEXT("DiscordBridge: Discord REST API returned %d: %s"),
				       Resp->GetResponseCode(), *Resp->GetContentAsString());
			}
		});

	Request->ProcessRequest();
}

TSharedRef<IHttpRequest, ESPMode::ThreadSafe>
UDiscordBridgeSubsystem::MakeApiRequest(const FString& Verb, const FString& Url) const
{
	TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request =
		FHttpModule::Get().CreateRequest();
	Request->SetURL(Url);
	Request->SetVerb(Verb);
	Request->SetHeader(TEXT("Authorization"),
	                   FString::Printf(TEXT("Bot %s"), *Config.BotToken));
	return Request;
}

// ─────────────────────────────────────────────────────────────────────────────
// Chat-manager delegate handler (Game → Discord)
// ─────────────────────────────────────────────────────────────────────────────

void UDiscordBridgeSubsystem::OnNewChatMessage()
{
	if (!IsValid(BoundChatManager)) return;

	TArray<FChatMessageStruct> Messages;
	BoundChatManager->GetReceivedChatMessages(Messages);

	for (int32 i = NumSeenChatMessages; i < Messages.Num(); ++i)
	{
		const FChatMessageStruct& Msg = Messages[i];
		if (Msg.MessageType == EFGChatMessageType::CMT_PlayerMessage)
		{
			SendGameMessageToDiscord(Msg.MessageSender.ToString(),
			                        Msg.MessageText.ToString());
		}
	}
	NumSeenChatMessages = Messages.Num();
}

// ─────────────────────────────────────────────────────────────────────────────
// Discord → Game chat relay
// ─────────────────────────────────────────────────────────────────────────────

void UDiscordBridgeSubsystem::RelayDiscordMessageToGame(const FString& Username,
                                                         const FString& Message)
{
	UWorld* World = GetWorld();
	if (!World)
	{
		UE_LOG(LogTemp, Warning,
		       TEXT("DiscordBridge: No world – cannot relay Discord message to game chat."));
		return;
	}

	AFGChatManager* ChatManager = AFGChatManager::Get(World);
	if (!ChatManager)
	{
		UE_LOG(LogTemp, Warning,
		       TEXT("DiscordBridge: ChatManager not found – cannot relay Discord message."));
		return;
	}

	FString FormattedMessage = Config.DiscordToGameFormat;
	FormattedMessage = FormattedMessage.Replace(TEXT("{Username}"), *Username);
	FormattedMessage = FormattedMessage.Replace(TEXT("{Message}"),  *Message);

	FChatMessageStruct ChatMsg;
	ChatMsg.MessageText   = FText::FromString(FormattedMessage);
	ChatMsg.MessageType   = EFGChatMessageType::CMT_SystemMessage;
	ChatMsg.MessageSender = FText::FromString(TEXT("Discord"));

	ChatManager->BroadcastChatMessage(ChatMsg);

	UE_LOG(LogTemp, Log,
	       TEXT("DiscordBridge: Relayed to game chat: [%s] %s"), *Username, *Message);
}

