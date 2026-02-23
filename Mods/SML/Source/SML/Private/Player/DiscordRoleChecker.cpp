#include "Player/DiscordRoleChecker.h"
#include "Dom/JsonObject.h"
#include "Dom/JsonValue.h"
#include "HAL/PlatformFileManager.h"
#include "HttpModule.h"
#include "Interfaces/IHttpRequest.h"
#include "Interfaces/IHttpResponse.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"
#include "Serialization/JsonWriter.h"

DEFINE_LOG_CATEGORY_STATIC(LogDiscordRoleChecker, Log, All);

TMap<FString, FString> FDiscordRoleChecker::PlayerToDiscordId;

FString FDiscordRoleChecker::GetLinksFilePath() {
    return FPaths::ProjectDir() + TEXT("Configs/SML_DiscordLinks.json");
}

void FDiscordRoleChecker::LoadLinks() {
    const FString FilePath = GetLinksFilePath();
    PlayerToDiscordId.Empty();

    if (!FPlatformFileManager::Get().GetPlatformFile().FileExists(*FilePath)) {
        UE_LOG(LogDiscordRoleChecker, Display,
            TEXT("Discord links file not found at %s, creating empty file."), *FilePath);
        SaveLinks();
        return;
    }

    FString RawJson;
    if (!FFileHelper::LoadFileToString(RawJson, *FilePath)) {
        UE_LOG(LogDiscordRoleChecker, Error,
            TEXT("Failed to read Discord links file from %s"), *FilePath);
        return;
    }

    TSharedPtr<FJsonObject> JsonObject;
    const TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(RawJson);
    if (!FJsonSerializer::Deserialize(Reader, JsonObject) || !JsonObject.IsValid()) {
        UE_LOG(LogDiscordRoleChecker, Warning,
            TEXT("Discord links JSON is malformed, starting with empty mapping."));
        SaveLinks();
        return;
    }

    if (JsonObject->HasTypedField<EJson::Object>(TEXT("links"))) {
        const TSharedPtr<FJsonObject> LinksObj = JsonObject->GetObjectField(TEXT("links"));
        for (const auto& Pair : LinksObj->Values) {
            if (Pair.Value->Type == EJson::String) {
                PlayerToDiscordId.Add(Pair.Key.ToLower(), Pair.Value->AsString());
            }
        }
    }

    UE_LOG(LogDiscordRoleChecker, Display,
        TEXT("Loaded %d Discord link(s) from disk."), PlayerToDiscordId.Num());
}

void FDiscordRoleChecker::SaveLinks() {
    const FString FilePath = GetLinksFilePath();
    FPlatformFileManager::Get().GetPlatformFile().CreateDirectoryTree(*FPaths::GetPath(FilePath));

    const TSharedRef<FJsonObject> LinksObj = MakeShareable(new FJsonObject());
    for (const auto& Pair : PlayerToDiscordId) {
        LinksObj->SetStringField(Pair.Key, Pair.Value);
    }

    const TSharedRef<FJsonObject> Root = MakeShareable(new FJsonObject());
    Root->SetObjectField(TEXT("links"), LinksObj);

    FString OutJson;
    const TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&OutJson);
    FJsonSerializer::Serialize(Root, Writer);

    if (!FFileHelper::SaveStringToFile(OutJson, *FilePath)) {
        UE_LOG(LogDiscordRoleChecker, Error,
            TEXT("Failed to save Discord links to %s"), *FilePath);
        return;
    }

    UE_LOG(LogDiscordRoleChecker, Display,
        TEXT("Saved %d Discord link(s) to %s"), PlayerToDiscordId.Num(), *FilePath);
}

void FDiscordRoleChecker::LinkPlayer(const FString& PlayerName, const FString& DiscordUserId) {
    PlayerToDiscordId.Add(PlayerName.ToLower(), DiscordUserId);
    SaveLinks();
}

FString FDiscordRoleChecker::GetDiscordUserId(const FString& PlayerName) {
    const FString* Found = PlayerToDiscordId.Find(PlayerName.ToLower());
    return Found ? *Found : FString();
}

void FDiscordRoleChecker::CheckMemberHasRole(
    const FString& DiscordUserId,
    const FString& GuildId,
    const FString& RoleId,
    const FString& BotToken,
    TFunction<void(bool)> Callback) {

    if (DiscordUserId.IsEmpty() || GuildId.IsEmpty() || RoleId.IsEmpty() || BotToken.IsEmpty()) {
        UE_LOG(LogDiscordRoleChecker, Warning,
            TEXT("CheckMemberHasRole called with empty parameter(s) – denying access."));
        Callback(false);
        return;
    }

    const FString Url = FString::Printf(
        TEXT("https://discord.com/api/v10/guilds/%s/members/%s"),
        *GuildId, *DiscordUserId);

    TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request =
        FHttpModule::Get().CreateRequest();

    Request->SetURL(Url);
    Request->SetVerb(TEXT("GET"));
    Request->SetHeader(TEXT("Authorization"),
        FString::Printf(TEXT("Bot %s"), *BotToken));
    Request->SetHeader(TEXT("Content-Type"), TEXT("application/json"));

    // Capture by value so the lambda owns the data it needs
    Request->OnProcessRequestComplete().BindLambda(
        [RoleId, Callback](FHttpRequestPtr /*Req*/, FHttpResponsePtr Response, bool bConnectedSuccessfully) {

            if (!bConnectedSuccessfully || !Response.IsValid()) {
                UE_LOG(LogDiscordRoleChecker, Warning,
                    TEXT("Discord API request failed (connection error)."));
                Callback(false);
                return;
            }

            const int32 StatusCode = Response->GetResponseCode();
            if (StatusCode != 200) {
                UE_LOG(LogDiscordRoleChecker, Warning,
                    TEXT("Discord API returned HTTP %d for guild member lookup."), StatusCode);
                Callback(false);
                return;
            }

            // Parse the member object – we only need the "roles" array
            TSharedPtr<FJsonObject> MemberObject;
            const TSharedRef<TJsonReader<>> Reader =
                TJsonReaderFactory<>::Create(Response->GetContentAsString());

            if (!FJsonSerializer::Deserialize(Reader, MemberObject) ||
                !MemberObject.IsValid() ||
                !MemberObject->HasTypedField<EJson::Array>(TEXT("roles"))) {
                UE_LOG(LogDiscordRoleChecker, Warning,
                    TEXT("Discord API response is malformed or missing 'roles' field."));
                Callback(false);
                return;
            }

            for (const TSharedPtr<FJsonValue>& RoleVal :
                 MemberObject->GetArrayField(TEXT("roles"))) {
                if (RoleVal->Type == EJson::String &&
                    RoleVal->AsString() == RoleId) {
                    Callback(true);
                    return;
                }
            }

            Callback(false);
        });

    Request->ProcessRequest();
}
