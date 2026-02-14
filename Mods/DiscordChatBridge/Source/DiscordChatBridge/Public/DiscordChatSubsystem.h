// Copyright (c) 2024 Discord Chat Bridge Contributors

#pragma once

#include "CoreMinimal.h"
#include "Subsystem/ModSubsystem.h"
#include "FGChatManager.h"
#include "DiscordAPI.h"
#include "DiscordChatSubsystem.generated.h"

/**
 * Subsystem that bridges the Satisfactory in-game chat with Discord
 */
UCLASS()
class DISCORDCHATBRIDGE_API ADiscordChatSubsystem : public AModSubsystem
{
	GENERATED_BODY()

public:
	ADiscordChatSubsystem();

	/** Subsystem interface */
	virtual void Init() override;
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

protected:
	/** Load configuration from file */
	void LoadConfiguration();

	/** Handle chat messages from the game */
	UFUNCTION()
	void OnGameChatMessageAdded();

	/** Handle chat messages from Discord */
	void OnDiscordMessageReceived(const FString& Username, const FString& Message);

	/** Forward message from game to Discord */
	void ForwardGameMessageToDiscord(const FChatMessageStruct& Message);

	/** Forward message from Discord to game */
	void ForwardDiscordMessageToGame(const FString& Username, const FString& Message);

	/** Update bot activity status with current player count */
	void UpdateBotActivity();

	/** Get current player count */
	int32 GetPlayerCount() const;

private:
	/** Discord API instance */
	UPROPERTY()
	UDiscordAPI* DiscordAPI;

	/** Discord bot configuration */
	FDiscordBotConfig BotConfig;

	/** Reference to the game's chat manager */
	UPROPERTY()
	AFGChatManager* ChatManager;

	/** Index of last processed game message */
	int32 LastProcessedMessageIndex;

	/** Timer handle for activity updates */
	FTimerHandle ActivityTimerHandle;
};
