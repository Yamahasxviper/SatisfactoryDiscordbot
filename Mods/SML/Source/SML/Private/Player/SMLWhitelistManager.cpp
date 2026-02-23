#include "Player/SMLWhitelistManager.h"
#include "Dom/JsonObject.h"
#include "Dom/JsonValue.h"
#include "HAL/PlatformFileManager.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"
#include "Serialization/JsonWriter.h"

DEFINE_LOG_CATEGORY_STATIC(LogSMLWhitelist, Log, All);

TArray<FString> FSMLWhitelistManager::WhitelistedPlayers;

FString FSMLWhitelistManager::GetWhitelistFilePath() {
    return FPaths::ProjectDir() + TEXT("Configs/SML_Whitelist.json");
}

void FSMLWhitelistManager::LoadWhitelist() {
    const FString FilePath = GetWhitelistFilePath();
    IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();

    WhitelistedPlayers.Empty();

    if (!PlatformFile.FileExists(*FilePath)) {
        UE_LOG(LogSMLWhitelist, Display, TEXT("Whitelist file not found at %s, creating empty whitelist."), *FilePath);
        SaveWhitelist();
        return;
    }

    FString RawJson;
    if (!FFileHelper::LoadFileToString(RawJson, *FilePath)) {
        UE_LOG(LogSMLWhitelist, Error, TEXT("Failed to read whitelist file from %s"), *FilePath);
        return;
    }

    TSharedPtr<FJsonObject> JsonObject;
    const TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(RawJson);
    if (!FJsonSerializer::Deserialize(Reader, JsonObject) || !JsonObject.IsValid()) {
        UE_LOG(LogSMLWhitelist, Warning, TEXT("Whitelist JSON is malformed, starting with empty whitelist."));
        SaveWhitelist();
        return;
    }

    if (JsonObject->HasTypedField<EJson::Array>(TEXT("whitelistedPlayers"))) {
        for (const TSharedPtr<FJsonValue>& Val : JsonObject->GetArrayField(TEXT("whitelistedPlayers"))) {
            if (Val->Type == EJson::String) {
                WhitelistedPlayers.AddUnique(Val->AsString().ToLower());
            }
        }
    }

    UE_LOG(LogSMLWhitelist, Display, TEXT("Loaded %d whitelisted player(s) from disk."), WhitelistedPlayers.Num());
}

void FSMLWhitelistManager::SaveWhitelist() {
    const FString FilePath = GetWhitelistFilePath();

    // Ensure the Configs directory exists
    FPlatformFileManager::Get().GetPlatformFile().CreateDirectoryTree(*FPaths::GetPath(FilePath));

    TArray<TSharedPtr<FJsonValue>> PlayerArray;
    for (const FString& Name : WhitelistedPlayers) {
        PlayerArray.Add(MakeShareable(new FJsonValueString(Name)));
    }

    const TSharedRef<FJsonObject> JsonObject = MakeShareable(new FJsonObject());
    JsonObject->SetArrayField(TEXT("whitelistedPlayers"), PlayerArray);

    FString OutJson;
    const TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&OutJson);
    FJsonSerializer::Serialize(JsonObject, Writer);

    if (!FFileHelper::SaveStringToFile(OutJson, *FilePath)) {
        UE_LOG(LogSMLWhitelist, Error, TEXT("Failed to save whitelist to %s"), *FilePath);
        return;
    }

    UE_LOG(LogSMLWhitelist, Display, TEXT("Whitelist saved to %s (%d player(s))."), *FilePath, WhitelistedPlayers.Num());
}

bool FSMLWhitelistManager::IsPlayerWhitelisted(const FString& PlayerName) {
    return WhitelistedPlayers.Contains(PlayerName.ToLower());
}

bool FSMLWhitelistManager::AddPlayer(const FString& PlayerName) {
    const FString Lower = PlayerName.ToLower();
    if (WhitelistedPlayers.Contains(Lower)) {
        return false;
    }
    WhitelistedPlayers.Add(Lower);
    SaveWhitelist();
    return true;
}

bool FSMLWhitelistManager::RemovePlayer(const FString& PlayerName) {
    const int32 Removed = WhitelistedPlayers.Remove(PlayerName.ToLower());
    if (Removed > 0) {
        SaveWhitelist();
        return true;
    }
    return false;
}

TArray<FString> FSMLWhitelistManager::GetWhitelistedPlayers() {
    return WhitelistedPlayers;
}
