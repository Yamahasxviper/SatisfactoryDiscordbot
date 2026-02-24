#include "Player/WhitelistConfig.h"
#include "Dom/JsonObject.h"
#include "Dom/JsonValue.h"
#include "HAL/PlatformFileManager.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"
#include "Serialization/JsonWriter.h"

DEFINE_LOG_CATEGORY_STATIC(LogWhitelistConfig, Log, All);

// ---------------------------------------------------------------------------
// FWhitelistConfig
// ---------------------------------------------------------------------------

FWhitelistConfig::FWhitelistConfig() :
    bEnableWhitelist(false),
    WhitelistRole(TEXT("admin")),
    WhitelistChannel(TEXT("")),
    DiscordBotToken(TEXT("")),
    DiscordGuildId(TEXT("")),
    DiscordWhitelistRoleId(TEXT("")),
    DiscordWhitelistChannelId(TEXT("")) {
}

void FWhitelistConfig::ReadFromJson(const TSharedPtr<FJsonObject>& Json, FWhitelistConfig& OutConfig, bool* OutIsMissingSections) {
    bool bMissing = false;

    if (Json->HasTypedField<EJson::Boolean>(TEXT("enableWhitelist"))) {
        OutConfig.bEnableWhitelist = Json->GetBoolField(TEXT("enableWhitelist"));
    } else { bMissing = true; }

    if (Json->HasTypedField<EJson::String>(TEXT("whitelistRole"))) {
        OutConfig.WhitelistRole = Json->GetStringField(TEXT("whitelistRole"));
    } else { bMissing = true; }

    if (Json->HasTypedField<EJson::String>(TEXT("whitelistChannel"))) {
        OutConfig.WhitelistChannel = Json->GetStringField(TEXT("whitelistChannel"));
    } else { bMissing = true; }

    if (Json->HasTypedField<EJson::String>(TEXT("discordBotToken"))) {
        OutConfig.DiscordBotToken = Json->GetStringField(TEXT("discordBotToken"));
    } else { bMissing = true; }

    if (Json->HasTypedField<EJson::String>(TEXT("discordGuildId"))) {
        OutConfig.DiscordGuildId = Json->GetStringField(TEXT("discordGuildId"));
    } else { bMissing = true; }

    if (Json->HasTypedField<EJson::String>(TEXT("discordWhitelistRoleId"))) {
        OutConfig.DiscordWhitelistRoleId = Json->GetStringField(TEXT("discordWhitelistRoleId"));
    } else { bMissing = true; }

    if (Json->HasTypedField<EJson::String>(TEXT("discordWhitelistChannelId"))) {
        OutConfig.DiscordWhitelistChannelId = Json->GetStringField(TEXT("discordWhitelistChannelId"));
    } else { bMissing = true; }

    if (OutIsMissingSections) {
        *OutIsMissingSections = bMissing;
    }
}

void FWhitelistConfig::WriteToJson(const TSharedPtr<FJsonObject>& OutJson, const FWhitelistConfig& Config) {
    OutJson->SetBoolField(TEXT("enableWhitelist"), Config.bEnableWhitelist);
    OutJson->SetStringField(TEXT("whitelistRole"), Config.WhitelistRole);
    OutJson->SetStringField(TEXT("whitelistChannel"), Config.WhitelistChannel);
    OutJson->SetStringField(TEXT("discordBotToken"), Config.DiscordBotToken);
    OutJson->SetStringField(TEXT("discordGuildId"), Config.DiscordGuildId);
    OutJson->SetStringField(TEXT("discordWhitelistRoleId"), Config.DiscordWhitelistRoleId);
    OutJson->SetStringField(TEXT("discordWhitelistChannelId"), Config.DiscordWhitelistChannelId);
}

// ---------------------------------------------------------------------------
// FWhitelistConfigManager
// ---------------------------------------------------------------------------

FWhitelistConfig FWhitelistConfigManager::ConfigPrivate;

FString FWhitelistConfigManager::GetConfigFilePath() {
    return FPaths::ProjectDir() + TEXT("Configs/SML_WhitelistConfig.json");
}

void FWhitelistConfigManager::LoadConfig() {
    const FString FilePath = GetConfigFilePath();
    bool bShouldSave = false;

    if (!FPlatformFileManager::Get().GetPlatformFile().FileExists(*FilePath)) {
        UE_LOG(LogWhitelistConfig, Display,
            TEXT("Whitelist config not found at %s – creating with defaults."), *FilePath);
        bShouldSave = true;
    } else {
        FString RawJson;
        if (!FFileHelper::LoadFileToString(RawJson, *FilePath)) {
            UE_LOG(LogWhitelistConfig, Error,
                TEXT("Failed to read whitelist config from %s"), *FilePath);
        } else {
            TSharedPtr<FJsonObject> JsonObject;
            const TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(RawJson);
            if (!FJsonSerializer::Deserialize(Reader, JsonObject) || !JsonObject.IsValid()) {
                UE_LOG(LogWhitelistConfig, Warning,
                    TEXT("Whitelist config JSON is malformed – resetting to defaults."));
                ConfigPrivate = FWhitelistConfig();
                bShouldSave = true;
            } else {
                FWhitelistConfig::ReadFromJson(JsonObject, ConfigPrivate, &bShouldSave);
                UE_LOG(LogWhitelistConfig, Display,
                    TEXT("Whitelist config loaded. Whitelist %s."),
                    ConfigPrivate.bEnableWhitelist ? TEXT("ENABLED") : TEXT("disabled"));
            }
        }
    }

    if (bShouldSave) {
        SaveConfig();
    }
}

void FWhitelistConfigManager::SaveConfig() {
    const FString FilePath = GetConfigFilePath();
    FPlatformFileManager::Get().GetPlatformFile().CreateDirectoryTree(*FPaths::GetPath(FilePath));

    const TSharedRef<FJsonObject> JsonObject = MakeShareable(new FJsonObject());
    FWhitelistConfig::WriteToJson(JsonObject, ConfigPrivate);

    FString OutJson;
    const TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&OutJson);
    FJsonSerializer::Serialize(JsonObject, Writer);

    if (!FFileHelper::SaveStringToFile(OutJson, *FilePath)) {
        UE_LOG(LogWhitelistConfig, Error,
            TEXT("Failed to save whitelist config to %s"), *FilePath);
        return;
    }
    UE_LOG(LogWhitelistConfig, Display,
        TEXT("Whitelist config saved to %s"), *FilePath);
}

FWhitelistConfig FWhitelistConfigManager::GetConfig() {
    return ConfigPrivate;
}

void FWhitelistConfigManager::SetConfig(const FWhitelistConfig& NewConfig) {
    ConfigPrivate = NewConfig;
}
