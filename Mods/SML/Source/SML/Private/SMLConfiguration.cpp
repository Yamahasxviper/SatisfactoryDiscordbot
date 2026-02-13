#include "SMLConfiguration.h"
#include "Dom/JsonObject.h"

FSMLConfiguration::FSMLConfiguration() :
    bEnableFunchookLogging(false),
    InGameChatNameFormat(TEXT("{name}")),
    DiscordNameFormat(TEXT("{name}")),
    bEnableNameFormatting(false) {
}

void FSMLConfiguration::ReadFromJson(const TSharedPtr<FJsonObject>& Json, FSMLConfiguration& OutConfiguration, bool* OutIsMissingSections) {
    bool bIsMissingSectionsInternal = false;
    
    if (Json->HasTypedField<EJson::Array>(TEXT("disabledChatCommands"))) {
        const TArray<TSharedPtr<FJsonValue>>& DisabledChatCommands = Json->GetArrayField(TEXT("disabledChatCommands"));
        for (const TSharedPtr<FJsonValue>& Value : DisabledChatCommands) {
            if (Value->Type == EJson::String)
                OutConfiguration.DisabledChatCommands.Add(Value->AsString());
        }
    } else {
        bIsMissingSectionsInternal = true;
    }
    
    if (Json->HasTypedField<EJson::Boolean>(TEXT("enableFunchookLogging"))) {
        OutConfiguration.bEnableFunchookLogging = Json->GetBoolField(TEXT("enableFunchookLogging"));
    } else {
        bIsMissingSectionsInternal = true;
    }

    if (Json->HasTypedField<EJson::String>(TEXT("inGameChatNameFormat"))) {
        OutConfiguration.InGameChatNameFormat = Json->GetStringField(TEXT("inGameChatNameFormat"));
    } else {
        bIsMissingSectionsInternal = true;
    }

    if (Json->HasTypedField<EJson::String>(TEXT("discordNameFormat"))) {
        OutConfiguration.DiscordNameFormat = Json->GetStringField(TEXT("discordNameFormat"));
    } else {
        bIsMissingSectionsInternal = true;
    }

    if (Json->HasTypedField<EJson::Boolean>(TEXT("enableNameFormatting"))) {
        OutConfiguration.bEnableNameFormatting = Json->GetBoolField(TEXT("enableNameFormatting"));
    } else {
        bIsMissingSectionsInternal = true;
    }

    if (OutIsMissingSections) {
        *OutIsMissingSections = bIsMissingSectionsInternal;
    }
}

void FSMLConfiguration::WriteToJson(const TSharedPtr<FJsonObject>& OutJson, const FSMLConfiguration& Configuration) {
    TArray<TSharedPtr<FJsonValue>> DisabledChatCommands;
    for (const FString& Value : Configuration.DisabledChatCommands) {
        DisabledChatCommands.Add(MakeShareable(new FJsonValueString(Value)));
    }
    OutJson->SetArrayField(TEXT("disabledChatCommands"), DisabledChatCommands);

    OutJson->SetBoolField(TEXT("enableFunchookLogging"), Configuration.bEnableFunchookLogging);

    OutJson->SetStringField(TEXT("inGameChatNameFormat"), Configuration.InGameChatNameFormat);

    OutJson->SetStringField(TEXT("discordNameFormat"), Configuration.DiscordNameFormat);

    OutJson->SetBoolField(TEXT("enableNameFormatting"), Configuration.bEnableNameFormatting);
}
