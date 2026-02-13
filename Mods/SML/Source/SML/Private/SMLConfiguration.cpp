#include "SMLConfiguration.h"
#include "Dom/JsonObject.h"

FSMLConfiguration::FSMLConfiguration() :
    bEnableFunchookLogging(false),
    bEnableDiscordChatSync(false),
    DiscordWebhookURL(TEXT("")),
    DiscordBotToken(TEXT("")),
    DiscordChannelID(TEXT("")),
    DiscordMessagePrefix(TEXT("[Discord]")),
    GameMessagePrefix(TEXT("[Game]")),
    bSyncSystemMessages(true),
    bSyncPlayerMessages(true) {
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

    // Discord chat sync configuration
    if (Json->HasTypedField<EJson::Boolean>(TEXT("enableDiscordChatSync"))) {
        OutConfiguration.bEnableDiscordChatSync = Json->GetBoolField(TEXT("enableDiscordChatSync"));
    } else {
        bIsMissingSectionsInternal = true;
    }

    if (Json->HasTypedField<EJson::String>(TEXT("discordWebhookURL"))) {
        OutConfiguration.DiscordWebhookURL = Json->GetStringField(TEXT("discordWebhookURL"));
    } else {
        bIsMissingSectionsInternal = true;
    }

    if (Json->HasTypedField<EJson::String>(TEXT("discordBotToken"))) {
        OutConfiguration.DiscordBotToken = Json->GetStringField(TEXT("discordBotToken"));
    } else {
        bIsMissingSectionsInternal = true;
    }

    if (Json->HasTypedField<EJson::String>(TEXT("discordChannelID"))) {
        OutConfiguration.DiscordChannelID = Json->GetStringField(TEXT("discordChannelID"));
    } else {
        bIsMissingSectionsInternal = true;
    }

    if (Json->HasTypedField<EJson::String>(TEXT("discordMessagePrefix"))) {
        OutConfiguration.DiscordMessagePrefix = Json->GetStringField(TEXT("discordMessagePrefix"));
    } else {
        bIsMissingSectionsInternal = true;
    }

    if (Json->HasTypedField<EJson::String>(TEXT("gameMessagePrefix"))) {
        OutConfiguration.GameMessagePrefix = Json->GetStringField(TEXT("gameMessagePrefix"));
    } else {
        bIsMissingSectionsInternal = true;
    }

    if (Json->HasTypedField<EJson::Boolean>(TEXT("syncSystemMessages"))) {
        OutConfiguration.bSyncSystemMessages = Json->GetBoolField(TEXT("syncSystemMessages"));
    } else {
        bIsMissingSectionsInternal = true;
    }

    if (Json->HasTypedField<EJson::Boolean>(TEXT("syncPlayerMessages"))) {
        OutConfiguration.bSyncPlayerMessages = Json->GetBoolField(TEXT("syncPlayerMessages"));
    } else {
        bIsMissingSectionsInternal = true;
    }

    if (Json->HasTypedField<EJson::Array>(TEXT("discordSyncBlacklist"))) {
        const TArray<TSharedPtr<FJsonValue>>& DiscordSyncBlacklist = Json->GetArrayField(TEXT("discordSyncBlacklist"));
        for (const TSharedPtr<FJsonValue>& Value : DiscordSyncBlacklist) {
            if (Value->Type == EJson::String)
                OutConfiguration.DiscordSyncBlacklist.Add(Value->AsString());
        }
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

    // Discord chat sync configuration
    OutJson->SetBoolField(TEXT("enableDiscordChatSync"), Configuration.bEnableDiscordChatSync);
    OutJson->SetStringField(TEXT("discordWebhookURL"), Configuration.DiscordWebhookURL);
    OutJson->SetStringField(TEXT("discordBotToken"), Configuration.DiscordBotToken);
    OutJson->SetStringField(TEXT("discordChannelID"), Configuration.DiscordChannelID);
    OutJson->SetStringField(TEXT("discordMessagePrefix"), Configuration.DiscordMessagePrefix);
    OutJson->SetStringField(TEXT("gameMessagePrefix"), Configuration.GameMessagePrefix);
    OutJson->SetBoolField(TEXT("syncSystemMessages"), Configuration.bSyncSystemMessages);
    OutJson->SetBoolField(TEXT("syncPlayerMessages"), Configuration.bSyncPlayerMessages);

    TArray<TSharedPtr<FJsonValue>> DiscordSyncBlacklist;
    for (const FString& Value : Configuration.DiscordSyncBlacklist) {
        DiscordSyncBlacklist.Add(MakeShareable(new FJsonValueString(Value)));
    }
    OutJson->SetArrayField(TEXT("discordSyncBlacklist"), DiscordSyncBlacklist);
}
