// Copyright (c) 2024 Yamahasxviper

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "FGChatManager.h"
#include "DiscordChatRelay.generated.h"

/**
 * Helper class to relay in-game chat messages to Discord
 * This class hooks into the FGChatManager to intercept chat messages
 */
UCLASS()
class DISCORDBOT_API UDiscordChatRelay : public UObject
{
    GENERATED_BODY()

public:
    /** Initialize the chat relay with the chat manager */
    void Initialize(AFGChatManager* ChatManager);

    /** Deinitialize and unbind from chat manager */
    void Deinitialize();

private:
    /** Handle new chat message added to the game */
    UFUNCTION()
    void OnChatMessageAdded();

    /** Reference to the chat manager */
    UPROPERTY()
    AFGChatManager* CachedChatManager;

    /** Last processed message count to detect new messages */
    int32 LastMessageCount;
};
