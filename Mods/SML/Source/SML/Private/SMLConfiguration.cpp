#include "SMLConfiguration.h"
#include "Dom/JsonObject.h"

FSMLConfiguration::FSMLConfiguration() :
    bEnableFunchookLogging(false),
    ChatCommandPrefix(TEXT("/")),
    ChatCommandCooldown(0.0f),
    MaxChatMessageLength(0),
    bEnableChatLogging(false),
    bEnableDiscordIntegration(false),
    DiscordWebhookURL(TEXT("")),
    DiscordBotToken(TEXT("")),
    DiscordChannelID(TEXT("")),
    DiscordMessageFormat(TEXT("**{player}**: {message}")) {
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

    // In-game chat configuration settings
    if (Json->HasTypedField<EJson::String>(TEXT("chatCommandPrefix"))) {
        OutConfiguration.ChatCommandPrefix = Json->GetStringField(TEXT("chatCommandPrefix"));
    } else {
        bIsMissingSectionsInternal = true;
    }

    if (Json->HasTypedField<EJson::Number>(TEXT("chatCommandCooldown"))) {
        OutConfiguration.ChatCommandCooldown = Json->GetNumberField(TEXT("chatCommandCooldown"));
    } else {
        bIsMissingSectionsInternal = true;
    }

    if (Json->HasTypedField<EJson::Number>(TEXT("maxChatMessageLength"))) {
        OutConfiguration.MaxChatMessageLength = Json->GetIntegerField(TEXT("maxChatMessageLength"));
    } else {
        bIsMissingSectionsInternal = true;
    }

    if (Json->HasTypedField<EJson::Boolean>(TEXT("enableChatLogging"))) {
        OutConfiguration.bEnableChatLogging = Json->GetBoolField(TEXT("enableChatLogging"));
    } else {
        bIsMissingSectionsInternal = true;
    }

    // Discord integration configuration settings
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

    // In-game chat configuration settings
    OutJson->SetStringField(TEXT("chatCommandPrefix"), Configuration.ChatCommandPrefix);
    OutJson->SetNumberField(TEXT("chatCommandCooldown"), Configuration.ChatCommandCooldown);
    OutJson->SetNumberField(TEXT("maxChatMessageLength"), Configuration.MaxChatMessageLength);
    OutJson->SetBoolField(TEXT("enableChatLogging"), Configuration.bEnableChatLogging);

    // Discord integration configuration settings
    OutJson->SetBoolField(TEXT("enableDiscordIntegration"), Configuration.bEnableDiscordIntegration);
    OutJson->SetStringField(TEXT("discordWebhookURL"), Configuration.DiscordWebhookURL);
    OutJson->SetStringField(TEXT("discordBotToken"), Configuration.DiscordBotToken);
    OutJson->SetStringField(TEXT("discordChannelID"), Configuration.DiscordChannelID);
    OutJson->SetStringField(TEXT("discordMessageFormat"), Configuration.DiscordMessageFormat);
}
