#include "SMLConfiguration.h"
#include "Dom/JsonObject.h"

FSMLConfiguration::FSMLConfiguration() :
    bEnableFunchookLogging(false),
    bEnableWhitelist(false),
    WhitelistRole(TEXT("admin")),
    WhitelistChannel(TEXT("")),
    DiscordBotToken(TEXT("")),
    DiscordGuildId(TEXT("")),
    DiscordWhitelistRoleId(TEXT("")),
    DiscordWhitelistChannelId(TEXT("")) {
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

    if (Json->HasTypedField<EJson::Boolean>(TEXT("enableWhitelist"))) {
        OutConfiguration.bEnableWhitelist = Json->GetBoolField(TEXT("enableWhitelist"));
    } else {
        bIsMissingSectionsInternal = true;
    }

    if (Json->HasTypedField<EJson::String>(TEXT("whitelistRole"))) {
        OutConfiguration.WhitelistRole = Json->GetStringField(TEXT("whitelistRole"));
    } else {
        bIsMissingSectionsInternal = true;
    }

    if (Json->HasTypedField<EJson::String>(TEXT("whitelistChannel"))) {
        OutConfiguration.WhitelistChannel = Json->GetStringField(TEXT("whitelistChannel"));
    } else {
        bIsMissingSectionsInternal = true;
    }

    if (Json->HasTypedField<EJson::String>(TEXT("discordBotToken"))) {
        OutConfiguration.DiscordBotToken = Json->GetStringField(TEXT("discordBotToken"));
    } else {
        bIsMissingSectionsInternal = true;
    }

    if (Json->HasTypedField<EJson::String>(TEXT("discordGuildId"))) {
        OutConfiguration.DiscordGuildId = Json->GetStringField(TEXT("discordGuildId"));
    } else {
        bIsMissingSectionsInternal = true;
    }

    if (Json->HasTypedField<EJson::String>(TEXT("discordWhitelistRoleId"))) {
        OutConfiguration.DiscordWhitelistRoleId = Json->GetStringField(TEXT("discordWhitelistRoleId"));
    } else {
        bIsMissingSectionsInternal = true;
    }

    if (Json->HasTypedField<EJson::String>(TEXT("discordWhitelistChannelId"))) {
        OutConfiguration.DiscordWhitelistChannelId = Json->GetStringField(TEXT("discordWhitelistChannelId"));
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

    OutJson->SetBoolField(TEXT("enableWhitelist"), Configuration.bEnableWhitelist);
    OutJson->SetStringField(TEXT("whitelistRole"), Configuration.WhitelistRole);
    OutJson->SetStringField(TEXT("whitelistChannel"), Configuration.WhitelistChannel);

    OutJson->SetStringField(TEXT("discordBotToken"), Configuration.DiscordBotToken);
    OutJson->SetStringField(TEXT("discordGuildId"), Configuration.DiscordGuildId);
    OutJson->SetStringField(TEXT("discordWhitelistRoleId"), Configuration.DiscordWhitelistRoleId);
    OutJson->SetStringField(TEXT("discordWhitelistChannelId"), Configuration.DiscordWhitelistChannelId);
}
