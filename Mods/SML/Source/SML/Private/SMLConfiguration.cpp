#include "SMLConfiguration.h"
#include "Dom/JsonObject.h"

FSMLConfiguration::FSMLConfiguration() :
    bEnableFunchookLogging(false),
    bEnableDiscordIntegration(false),
    DiscordWebhookURL(TEXT("")),
    DiscordBotToken(TEXT("")),
    DiscordChannelID(TEXT("")),
    bRelayChatToDiscord(true),
    bRelayDiscordToChat(true),
    ChatToDiscordPrefix(TEXT("[Game] ")),
    ChatToDiscordSuffix(TEXT("")),
    DiscordToChatPrefix(TEXT("[Discord] ")),
    DiscordToChatSuffix(TEXT("")),
    bFilterSystemMessages(true),
    bIncludePlayerNameInDiscord(true),
    DiscordMessageFormat(TEXT("**{PlayerName}**: {Message}")) {
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

    // Discord Integration Settings
    if (Json->HasTypedField<EJson::Boolean>(TEXT("enableDiscordIntegration"))) {
        OutConfiguration.bEnableDiscordIntegration = Json->GetBoolField(TEXT("enableDiscordIntegration"));
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

    // In-Game Chat Settings
    if (Json->HasTypedField<EJson::Boolean>(TEXT("relayChatToDiscord"))) {
        OutConfiguration.bRelayChatToDiscord = Json->GetBoolField(TEXT("relayChatToDiscord"));
    } else {
        bIsMissingSectionsInternal = true;
    }

    if (Json->HasTypedField<EJson::Boolean>(TEXT("relayDiscordToChat"))) {
        OutConfiguration.bRelayDiscordToChat = Json->GetBoolField(TEXT("relayDiscordToChat"));
    } else {
        bIsMissingSectionsInternal = true;
    }

    if (Json->HasTypedField<EJson::String>(TEXT("chatToDiscordPrefix"))) {
        OutConfiguration.ChatToDiscordPrefix = Json->GetStringField(TEXT("chatToDiscordPrefix"));
    } else {
        bIsMissingSectionsInternal = true;
    }

    if (Json->HasTypedField<EJson::String>(TEXT("chatToDiscordSuffix"))) {
        OutConfiguration.ChatToDiscordSuffix = Json->GetStringField(TEXT("chatToDiscordSuffix"));
    } else {
        bIsMissingSectionsInternal = true;
    }

    if (Json->HasTypedField<EJson::String>(TEXT("discordToChatPrefix"))) {
        OutConfiguration.DiscordToChatPrefix = Json->GetStringField(TEXT("discordToChatPrefix"));
    } else {
        bIsMissingSectionsInternal = true;
    }

    if (Json->HasTypedField<EJson::String>(TEXT("discordToChatSuffix"))) {
        OutConfiguration.DiscordToChatSuffix = Json->GetStringField(TEXT("discordToChatSuffix"));
    } else {
        bIsMissingSectionsInternal = true;
    }

    if (Json->HasTypedField<EJson::Boolean>(TEXT("filterSystemMessages"))) {
        OutConfiguration.bFilterSystemMessages = Json->GetBoolField(TEXT("filterSystemMessages"));
    } else {
        bIsMissingSectionsInternal = true;
    }

    if (Json->HasTypedField<EJson::Boolean>(TEXT("includePlayerNameInDiscord"))) {
        OutConfiguration.bIncludePlayerNameInDiscord = Json->GetBoolField(TEXT("includePlayerNameInDiscord"));
    } else {
        bIsMissingSectionsInternal = true;
    }

    if (Json->HasTypedField<EJson::String>(TEXT("discordMessageFormat"))) {
        OutConfiguration.DiscordMessageFormat = Json->GetStringField(TEXT("discordMessageFormat"));
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

    // Discord Integration Settings
    OutJson->SetBoolField(TEXT("enableDiscordIntegration"), Configuration.bEnableDiscordIntegration);
    OutJson->SetStringField(TEXT("discordWebhookURL"), Configuration.DiscordWebhookURL);
    OutJson->SetStringField(TEXT("discordBotToken"), Configuration.DiscordBotToken);
    OutJson->SetStringField(TEXT("discordChannelID"), Configuration.DiscordChannelID);

    // In-Game Chat Settings
    OutJson->SetBoolField(TEXT("relayChatToDiscord"), Configuration.bRelayChatToDiscord);
    OutJson->SetBoolField(TEXT("relayDiscordToChat"), Configuration.bRelayDiscordToChat);
    OutJson->SetStringField(TEXT("chatToDiscordPrefix"), Configuration.ChatToDiscordPrefix);
    OutJson->SetStringField(TEXT("chatToDiscordSuffix"), Configuration.ChatToDiscordSuffix);
    OutJson->SetStringField(TEXT("discordToChatPrefix"), Configuration.DiscordToChatPrefix);
    OutJson->SetStringField(TEXT("discordToChatSuffix"), Configuration.DiscordToChatSuffix);
    OutJson->SetBoolField(TEXT("filterSystemMessages"), Configuration.bFilterSystemMessages);
    OutJson->SetBoolField(TEXT("includePlayerNameInDiscord"), Configuration.bIncludePlayerNameInDiscord);
    OutJson->SetStringField(TEXT("discordMessageFormat"), Configuration.DiscordMessageFormat);
}
