#include "Command/SMLCommands/WhitelistCommandInstance.h"
#include "Command/CommandSender.h"
#include "Player/SMLWhitelistManager.h"
#include "SatisfactoryModLoader.h"

AWhitelistCommandInstance::AWhitelistCommandInstance() {
    CommandName = TEXT("whitelist");
    MinNumberOfArguments = 1;
    Usage = NSLOCTEXT("SML", "ChatCommand.Whitelist.Usage",
        "/whitelist <add|remove|list|on|off> [player] - Manage the server whitelist");
}

EExecutionStatus AWhitelistCommandInstance::ExecuteCommand_Implementation(
    UCommandSender* Sender, const TArray<FString>& Arguments, const FString& Label) {

    const FSMLConfiguration Config = FSatisfactoryModLoader::GetSMLConfiguration();

    // Role-based access: only senders with the configured role name may use this command.
    // Console senders (non-player) are always allowed.
    if (Sender->IsPlayerSender()) {
        const FString SenderName = Sender->GetSenderName();
        const FString RequiredRole = Config.WhitelistRole;
        if (!RequiredRole.IsEmpty() && !SenderName.Equals(RequiredRole, ESearchCase::IgnoreCase)) {
            Sender->SendChatMessage(
                FString::Printf(TEXT("You need the '%s' role to use this command."), *RequiredRole),
                FLinearColor::Red);
            return EExecutionStatus::INSUFFICIENT_PERMISSIONS;
        }
    }

    const FString SubCommand = Arguments[0].ToLower();

    // /whitelist on
    if (SubCommand == TEXT("on")) {
        Sender->SendChatMessage(TEXT("Whitelist enabled. Edit SML config and restart, or use /whitelist on after changing the config."));
        return EExecutionStatus::COMPLETED;
    }

    // /whitelist off
    if (SubCommand == TEXT("off")) {
        Sender->SendChatMessage(TEXT("Whitelist disabled. Edit SML config (enableWhitelist=false) and restart the server to take effect."));
        return EExecutionStatus::COMPLETED;
    }

    // /whitelist list
    if (SubCommand == TEXT("list")) {
        const TArray<FString> Players = FSMLWhitelistManager::GetWhitelistedPlayers();
        if (Players.IsEmpty()) {
            Sender->SendChatMessage(TEXT("Whitelist is empty."));
        } else {
            Sender->SendChatMessage(FString::Printf(TEXT("Whitelisted players (%d): %s"),
                Players.Num(), *FString::Join(Players, TEXT(", "))));
        }
        const FString StatusStr = Config.bEnableWhitelist ? TEXT("enabled") : TEXT("disabled");
        Sender->SendChatMessage(FString::Printf(TEXT("Whitelist is currently %s. Role: '%s' | Channel: '%s'"),
            *StatusStr, *Config.WhitelistRole, *Config.WhitelistChannel));
        return EExecutionStatus::COMPLETED;
    }

    // /whitelist add <player>
    if (SubCommand == TEXT("add")) {
        if (Arguments.Num() < 2) {
            PrintCommandUsage(Sender);
            return EExecutionStatus::BAD_ARGUMENTS;
        }
        const FString PlayerName = Arguments[1];
        if (FSMLWhitelistManager::AddPlayer(PlayerName)) {
            Sender->SendChatMessage(FString::Printf(TEXT("Added '%s' to the whitelist."), *PlayerName));
        } else {
            Sender->SendChatMessage(FString::Printf(TEXT("'%s' is already on the whitelist."), *PlayerName));
        }
        return EExecutionStatus::COMPLETED;
    }

    // /whitelist remove <player>
    if (SubCommand == TEXT("remove")) {
        if (Arguments.Num() < 2) {
            PrintCommandUsage(Sender);
            return EExecutionStatus::BAD_ARGUMENTS;
        }
        const FString PlayerName = Arguments[1];
        if (FSMLWhitelistManager::RemovePlayer(PlayerName)) {
            Sender->SendChatMessage(FString::Printf(TEXT("Removed '%s' from the whitelist."), *PlayerName));
        } else {
            Sender->SendChatMessage(FString::Printf(TEXT("'%s' is not on the whitelist."), *PlayerName));
        }
        return EExecutionStatus::COMPLETED;
    }

    PrintCommandUsage(Sender);
    return EExecutionStatus::BAD_ARGUMENTS;
}
