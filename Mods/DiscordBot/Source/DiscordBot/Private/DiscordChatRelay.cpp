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
    LastMessageCount = 0;

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

    // Process only new messages
    if (Messages.Num() > LastMessageCount)
    {
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
}
