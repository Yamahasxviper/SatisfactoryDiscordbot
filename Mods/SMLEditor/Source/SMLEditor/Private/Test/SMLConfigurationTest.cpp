// Copyright Epic Games, Inc. All Rights Reserved.

#include "CoreTypes.h"
#include "SMLConfiguration.h"
#include "Dom/JsonObject.h"
#include "Serialization/JsonSerializer.h"
#include "Serialization/JsonWriter.h"
#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FSMLConfigurationSerializationTest, "SMLEditor.Configuration.Serialization", EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::SmokeFilter)

bool FSMLConfigurationSerializationTest::RunTest(const FString& Parameters)
{
	// Create a configuration with test values
	FSMLConfiguration TestConfig;
	TestConfig.bEnableFunchookLogging = true;
	TestConfig.DisabledChatCommands.Add(TEXT("test:command1"));
	TestConfig.DisabledChatCommands.Add(TEXT("test:command2"));
	
	// Discord configuration
	TestConfig.bEnableDiscordChatSync = true;
	TestConfig.DiscordWebhookURL = TEXT("https://discord.com/api/webhooks/123456789/abcdef");
	TestConfig.DiscordBotToken = TEXT("test_bot_token_12345");
	TestConfig.DiscordChannelID = TEXT("987654321");
	TestConfig.DiscordMessagePrefix = TEXT("[Discord]");
	TestConfig.GameMessagePrefix = TEXT("[Game]");
	TestConfig.bSyncSystemMessages = true;
	TestConfig.bSyncPlayerMessages = false;
	TestConfig.DiscordSyncBlacklist.Add(TEXT("Player1"));
	TestConfig.DiscordSyncBlacklist.Add(TEXT("Player2"));

	// Serialize to JSON
	TSharedPtr<FJsonObject> JsonObject = MakeShareable(new FJsonObject());
	FSMLConfiguration::WriteToJson(JsonObject, TestConfig);

	// Verify JSON fields
	if (!TestResult(JsonObject->HasField(TEXT("enableFunchookLogging")), TEXT("JSON should have enableFunchookLogging field"))) return false;
	if (!TestResult(JsonObject->HasField(TEXT("disabledChatCommands")), TEXT("JSON should have disabledChatCommands field"))) return false;
	if (!TestResult(JsonObject->HasField(TEXT("enableDiscordChatSync")), TEXT("JSON should have enableDiscordChatSync field"))) return false;
	if (!TestResult(JsonObject->HasField(TEXT("discordWebhookURL")), TEXT("JSON should have discordWebhookURL field"))) return false;
	if (!TestResult(JsonObject->HasField(TEXT("discordBotToken")), TEXT("JSON should have discordBotToken field"))) return false;
	if (!TestResult(JsonObject->HasField(TEXT("discordChannelID")), TEXT("JSON should have discordChannelID field"))) return false;
	if (!TestResult(JsonObject->HasField(TEXT("discordMessagePrefix")), TEXT("JSON should have discordMessagePrefix field"))) return false;
	if (!TestResult(JsonObject->HasField(TEXT("gameMessagePrefix")), TEXT("JSON should have gameMessagePrefix field"))) return false;
	if (!TestResult(JsonObject->HasField(TEXT("syncSystemMessages")), TEXT("JSON should have syncSystemMessages field"))) return false;
	if (!TestResult(JsonObject->HasField(TEXT("syncPlayerMessages")), TEXT("JSON should have syncPlayerMessages field"))) return false;
	if (!TestResult(JsonObject->HasField(TEXT("discordSyncBlacklist")), TEXT("JSON should have discordSyncBlacklist field"))) return false;

	// Deserialize from JSON
	FSMLConfiguration DeserializedConfig;
	bool bIsMissingSections = false;
	FSMLConfiguration::ReadFromJson(JsonObject, DeserializedConfig, &bIsMissingSections);

	// Verify deserialization
	if (!TestResult(!bIsMissingSections, TEXT("Deserialization should not report missing sections"))) return false;
	if (!TestResult(DeserializedConfig.bEnableFunchookLogging == TestConfig.bEnableFunchookLogging, TEXT("bEnableFunchookLogging should match"))) return false;
	if (!TestResult(DeserializedConfig.DisabledChatCommands.Num() == TestConfig.DisabledChatCommands.Num(), TEXT("DisabledChatCommands count should match"))) return false;
	
	// Discord configuration verification
	if (!TestResult(DeserializedConfig.bEnableDiscordChatSync == TestConfig.bEnableDiscordChatSync, TEXT("bEnableDiscordChatSync should match"))) return false;
	if (!TestResult(DeserializedConfig.DiscordWebhookURL == TestConfig.DiscordWebhookURL, TEXT("DiscordWebhookURL should match"))) return false;
	if (!TestResult(DeserializedConfig.DiscordBotToken == TestConfig.DiscordBotToken, TEXT("DiscordBotToken should match"))) return false;
	if (!TestResult(DeserializedConfig.DiscordChannelID == TestConfig.DiscordChannelID, TEXT("DiscordChannelID should match"))) return false;
	if (!TestResult(DeserializedConfig.DiscordMessagePrefix == TestConfig.DiscordMessagePrefix, TEXT("DiscordMessagePrefix should match"))) return false;
	if (!TestResult(DeserializedConfig.GameMessagePrefix == TestConfig.GameMessagePrefix, TEXT("GameMessagePrefix should match"))) return false;
	if (!TestResult(DeserializedConfig.bSyncSystemMessages == TestConfig.bSyncSystemMessages, TEXT("bSyncSystemMessages should match"))) return false;
	if (!TestResult(DeserializedConfig.bSyncPlayerMessages == TestConfig.bSyncPlayerMessages, TEXT("bSyncPlayerMessages should match"))) return false;
	if (!TestResult(DeserializedConfig.DiscordSyncBlacklist.Num() == TestConfig.DiscordSyncBlacklist.Num(), TEXT("DiscordSyncBlacklist count should match"))) return false;

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FSMLConfigurationDefaultValuesTest, "SMLEditor.Configuration.DefaultValues", EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::SmokeFilter)

bool FSMLConfigurationDefaultValuesTest::RunTest(const FString& Parameters)
{
	// Create a default configuration
	FSMLConfiguration DefaultConfig;

	// Verify default values
	if (!TestResult(DefaultConfig.bEnableFunchookLogging == false, TEXT("bEnableFunchookLogging should default to false"))) return false;
	if (!TestResult(DefaultConfig.bEnableDiscordChatSync == false, TEXT("bEnableDiscordChatSync should default to false"))) return false;
	if (!TestResult(DefaultConfig.DiscordWebhookURL.IsEmpty(), TEXT("DiscordWebhookURL should default to empty"))) return false;
	if (!TestResult(DefaultConfig.DiscordBotToken.IsEmpty(), TEXT("DiscordBotToken should default to empty"))) return false;
	if (!TestResult(DefaultConfig.DiscordChannelID.IsEmpty(), TEXT("DiscordChannelID should default to empty"))) return false;
	if (!TestResult(DefaultConfig.DiscordMessagePrefix == TEXT("[Discord]"), TEXT("DiscordMessagePrefix should default to [Discord]"))) return false;
	if (!TestResult(DefaultConfig.GameMessagePrefix == TEXT("[Game]"), TEXT("GameMessagePrefix should default to [Game]"))) return false;
	if (!TestResult(DefaultConfig.bSyncSystemMessages == true, TEXT("bSyncSystemMessages should default to true"))) return false;
	if (!TestResult(DefaultConfig.bSyncPlayerMessages == true, TEXT("bSyncPlayerMessages should default to true"))) return false;
	if (!TestResult(DefaultConfig.DiscordSyncBlacklist.Num() == 0, TEXT("DiscordSyncBlacklist should default to empty"))) return false;

	return true;
}

#endif //WITH_DEV_AUTOMATION_TESTS
