// Copyright (c) 2024 Yamahasxviper

#include "DiscordChatRelay.h"
#include "DiscordBotSubsystem.h"
#include "Engine/World.h"
#include "Engine/GameInstance.h"
#include "Logging/LogMacros.h"

DEFINE_LOG_CATEGORY_STATIC(LogDiscordChatRelay, Log, All);

void UDiscordChatRelay::Initialize(AFGChatManager* ChatManager)
{
    if (!ChatManager)
    {
        UE_LOG(LogDiscordChatRelay, Error, TEXT("Cannot initialize chat relay: ChatManager is null"));
        return;
    }

    CachedChatManager = ChatManager;

    // Initialize to the current message count so we don't re-relay historical messages on startup
    TArray<FChatMessageStruct> ExistingMessages;
    ChatManager->GetReceivedChatMessages(ExistingMessages);
    LastMessageCount = ExistingMessages.Num();

    // Bind to chat message added event
    ChatManager->OnChatMessageAdded.AddDynamic(this, &UDiscordChatRelay::OnChatMessageAdded);

    UE_LOG(LogDiscordChatRelay, Log, TEXT("Discord Chat Relay initialized"));
}

void UDiscordChatRelay::Deinitialize()
{
    if (CachedChatManager)
    {
        CachedChatManager->OnChatMessageAdded.RemoveDynamic(this, &UDiscordChatRelay::OnChatMessageAdded);
        CachedChatManager = nullptr;
    }

    UE_LOG(LogDiscordChatRelay, Log, TEXT("Discord Chat Relay deinitialized"));
}

void UDiscordChatRelay::OnChatMessageAdded()
{
    if (!CachedChatManager)
    {
        return;
    }

    // Get all received messages
    TArray<FChatMessageStruct> Messages;
    CachedChatManager->GetReceivedChatMessages(Messages);

    // Determine which messages are new.
    // AFGChatManager keeps a rolling buffer of mMaxNumMessagesInHistory messages (default 50).
    // Once the buffer is full, each new message drops the oldest entry, keeping the count constant.
    // In that case Messages.Num() == LastMessageCount even though there is a new message; the
    // newest message is always the last element of the returned array.
    if (Messages.Num() > LastMessageCount)
    {
        // Normal case: messages were appended without rolling over
        for (int32 i = LastMessageCount; i < Messages.Num(); i++)
        {
            const FChatMessageStruct& Message = Messages[i];

            // Only relay player messages to Discord (not system, ADA, or custom messages from Discord)
            if (Message.MessageType == EFGChatMessageType::CMT_PlayerMessage)
            {
                FString PlayerName = Message.MessageSender.ToString();
                FString MessageText = Message.MessageText.ToString();

                // Forward to Discord Bot Subsystem
                if (UWorld* World = CachedChatManager->GetWorld())
                {
                    if (UGameInstance* GameInstance = World->GetGameInstance())
                    {
                        if (UDiscordBotSubsystem* Subsystem = GameInstance->GetSubsystem<UDiscordBotSubsystem>())
                        {
                            Subsystem->OnGameChatMessage(PlayerName, MessageText);
                            UE_LOG(LogDiscordChatRelay, Log, TEXT("Relayed game message to Discord: [%s] %s"), *PlayerName, *MessageText);
                        }
                    }
                }
            }
        }

        LastMessageCount = Messages.Num();
    }
    else if (Messages.Num() > 0)
    {
        // Buffer is at max capacity: the oldest message was dropped and the new message is the
        // last element. Process only that last message to avoid replaying the entire history.
        const FChatMessageStruct& Message = Messages.Last();

        if (Message.MessageType == EFGChatMessageType::CMT_PlayerMessage)
        {
            FString PlayerName = Message.MessageSender.ToString();
            FString MessageText = Message.MessageText.ToString();

            if (UWorld* World = CachedChatManager->GetWorld())
            {
                if (UGameInstance* GameInstance = World->GetGameInstance())
                {
                    if (UDiscordBotSubsystem* Subsystem = GameInstance->GetSubsystem<UDiscordBotSubsystem>())
                    {
                        Subsystem->OnGameChatMessage(PlayerName, MessageText);
                        UE_LOG(LogDiscordChatRelay, Log, TEXT("Relayed game message to Discord: [%s] %s"), *PlayerName, *MessageText);
                    }
                }
            }
        }

        // LastMessageCount stays equal to Messages.Num() (the maximum buffer size)
    }
}
