#include "DiscordGatewayClient.h"

#include "HttpModule.h"
#include "Interfaces/IHttpRequest.h"
#include "Interfaces/IHttpResponse.h"
#include "Dom/JsonObject.h"
#include "Dom/JsonValue.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"
#include "TimerManager.h"
#include "Engine/World.h"

DEFINE_LOG_CATEGORY_STATIC(LogDiscordBot, Log, All);

static const FString DISCORD_API_BASE = TEXT("https://discord.com/api/v10");

// ---- Helpers ---------------------------------------------------------------

static TSharedPtr<FJsonObject> ParseJson(const FString& Raw)
{
    TSharedPtr<FJsonObject> Obj;
    TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Raw);
    FJsonSerializer::Deserialize(Reader, Obj);
    return Obj;
}

// ---- UDiscordGatewayClient -------------------------------------------------

UDiscordGatewayClient::UDiscordGatewayClient()
{
}

// ---- HTTP helper -----------------------------------------------------------

TSharedRef<IHttpRequest, ESPMode::ThreadSafe> UDiscordGatewayClient::MakeRequest(const FString& Path) const
{
    TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request = FHttpModule::Get().CreateRequest();
    Request->SetURL(DISCORD_API_BASE + Path);
    Request->SetVerb(TEXT("GET"));
    Request->SetHeader(TEXT("Authorization"), BotToken);
    Request->SetHeader(TEXT("Content-Type"), TEXT("application/json"));
    Request->SetHeader(TEXT("User-Agent"), TEXT("satisfactory-discord-bot (https://github.com/satisfactorymodding/SatisfactoryModLoader, 1)"));
    return Request;
}

// ---- Connection ------------------------------------------------------------

void UDiscordGatewayClient::Connect(const FString& InBotToken, int32 /*InIntents*/)
{
    if (bPolling)
    {
        UE_LOG(LogDiscordBot, Warning, TEXT("Already polling. Call Disconnect() first."));
        return;
    }

    BotToken     = InBotToken;
    LastMessageId = TEXT("");
    bPolling      = false;

    if (BotToken.IsEmpty())
    {
        UE_LOG(LogDiscordBot, Error, TEXT("BotToken is empty — cannot connect."));
        OnConnected.Broadcast(false, TEXT("BotToken is empty"));
        return;
    }

    // Verify the token and kick off polling on success
    VerifyToken();
}

void UDiscordGatewayClient::Disconnect()
{
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().ClearTimer(PollTimerHandle);
    }
    bPolling = false;
    UE_LOG(LogDiscordBot, Log, TEXT("Discord HTTP polling stopped."));
}

// ---- Token verification ----------------------------------------------------

void UDiscordGatewayClient::VerifyToken()
{
    TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request = MakeRequest(TEXT("/users/@me"));
    Request->OnProcessRequestComplete().BindUObject(this, &UDiscordGatewayClient::OnVerifyResponse);
    Request->ProcessRequest();
    UE_LOG(LogDiscordBot, Log, TEXT("Verifying Discord bot token via /users/@me ..."));
}

void UDiscordGatewayClient::OnVerifyResponse(FHttpRequestPtr /*Request*/,
                                               FHttpResponsePtr  Response,
                                               bool              bSucceeded)
{
    if (!bSucceeded || !Response.IsValid())
    {
        const FString Err = TEXT("HTTP request failed (no response)");
        UE_LOG(LogDiscordBot, Error, TEXT("Token verification failed: %s"), *Err);
        OnConnected.Broadcast(false, Err);
        return;
    }

    const int32 Code = Response->GetResponseCode();
    if (Code != 200)
    {
        const FString Err = FString::Printf(TEXT("Discord returned HTTP %d: %s"), Code, *Response->GetContentAsString());
        UE_LOG(LogDiscordBot, Error, TEXT("Token verification failed: %s"), *Err);
        OnConnected.Broadcast(false, Err);
        return;
    }

    const TSharedPtr<FJsonObject> Body = ParseJson(Response->GetContentAsString());
    const FString Username = Body.IsValid() ? Body->GetStringField(TEXT("username")) : TEXT("unknown");
    UE_LOG(LogDiscordBot, Log, TEXT("Discord bot verified — logged in as: %s"), *Username);

    bPolling = true;
    OnConnected.Broadcast(true, TEXT(""));

    // Start the recurring poll timer
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().SetTimer(
            PollTimerHandle,
            this,
            &UDiscordGatewayClient::Poll,
            PollIntervalSeconds,
            /*bLoop=*/true,
            /*FirstDelay=*/0.f
        );
    }
}

// ---- Poll cycle ------------------------------------------------------------

void UDiscordGatewayClient::Poll()
{
    if (!ChannelId.IsEmpty())
    {
        PollMessages();
    }
    if (!GuildId.IsEmpty())
    {
        PollMembers();
    }
}

void UDiscordGatewayClient::PollMessages()
{
    // GET /channels/{id}/messages?after={last_id}&limit=100
    // Using ?after= means we only receive messages newer than the last one we saw.
    FString Path = FString::Printf(TEXT("/channels/%s/messages?limit=100"), *ChannelId);
    if (!LastMessageId.IsEmpty())
    {
        Path += FString::Printf(TEXT("&after=%s"), *LastMessageId);
    }

    TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request = MakeRequest(Path);
    Request->OnProcessRequestComplete().BindUObject(this, &UDiscordGatewayClient::OnMessagesResponse);
    Request->ProcessRequest();
}

void UDiscordGatewayClient::PollMembers()
{
    // GET /guilds/{id}/members?limit=1000
    // Returns up to 1000 members; for large guilds consider pagination.
    const FString Path = FString::Printf(TEXT("/guilds/%s/members?limit=1000"), *GuildId);

    TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request = MakeRequest(Path);
    Request->OnProcessRequestComplete().BindUObject(this, &UDiscordGatewayClient::OnMembersResponse);
    Request->ProcessRequest();
}

// ---- HTTP response handlers ------------------------------------------------

void UDiscordGatewayClient::OnMessagesResponse(FHttpRequestPtr /*Request*/,
                                                FHttpResponsePtr  Response,
                                                bool              bSucceeded)
{
    if (!bSucceeded || !Response.IsValid() || Response->GetResponseCode() != 200)
    {
        UE_LOG(LogDiscordBot, Warning, TEXT("Message poll failed (HTTP %d)"),
            Response.IsValid() ? Response->GetResponseCode() : 0);
        return;
    }

    // The response is a JSON array of message objects, newest-last when using ?after=
    TArray<TSharedPtr<FJsonValue>> Messages;
    TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Response->GetContentAsString());
    if (!FJsonSerializer::Deserialize(Reader, Messages))
    {
        UE_LOG(LogDiscordBot, Warning, TEXT("Failed to parse messages JSON array."));
        return;
    }

    for (const TSharedPtr<FJsonValue>& MsgVal : Messages)
    {
        const TSharedPtr<FJsonObject> Msg = MsgVal.IsValid() ? MsgVal->AsObject() : nullptr;
        if (!Msg.IsValid()) continue;

        // Track the highest message ID so we don't re-deliver
        const FString MsgId = Msg->GetStringField(TEXT("id"));
        if (LastMessageId.IsEmpty() || MsgId > LastMessageId)
        {
            LastMessageId = MsgId;
        }

        // Fire delegate (requires Message Content access in the Developer Portal)
        const FString Content = Msg->GetStringField(TEXT("content"));
        if (!Content.IsEmpty())
        {
            UE_LOG(LogDiscordBot, Log, TEXT("New message [%s]: %s"), *MsgId, *Content);
            OnMessageReceived.Broadcast(Content);
        }
    }
}

void UDiscordGatewayClient::OnMembersResponse(FHttpRequestPtr /*Request*/,
                                               FHttpResponsePtr  Response,
                                               bool              bSucceeded)
{
    if (!bSucceeded || !Response.IsValid() || Response->GetResponseCode() != 200)
    {
        UE_LOG(LogDiscordBot, Warning, TEXT("Member poll failed (HTTP %d)"),
            Response.IsValid() ? Response->GetResponseCode() : 0);
        return;
    }

    // Response is a JSON array of guild member objects
    TArray<TSharedPtr<FJsonValue>> Members;
    TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Response->GetContentAsString());
    if (!FJsonSerializer::Deserialize(Reader, Members))
    {
        UE_LOG(LogDiscordBot, Warning, TEXT("Failed to parse members JSON array."));
        return;
    }

    for (const TSharedPtr<FJsonValue>& MemberVal : Members)
    {
        const TSharedPtr<FJsonObject> Member = MemberVal.IsValid() ? MemberVal->AsObject() : nullptr;
        if (!Member.IsValid()) continue;

        const TSharedPtr<FJsonObject> User = Member->GetObjectField(TEXT("user"));
        if (!User.IsValid()) continue;

        const FString UserId = User->GetStringField(TEXT("id"));
        if (UserId.IsEmpty()) continue;

        // Fire the member delegate (requires Server Members access)
        OnMemberUpdated.Broadcast(UserId);

        // Best-effort presence: the REST member list does not include live presence,
        // but the bot can cross-reference by firing OnPresenceUpdated per known member.
        OnPresenceUpdated.Broadcast(UserId);
    }
}
