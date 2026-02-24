#include "Command/SMLCommands/WhitelistCommandInstance.h"
#include "Command/CommandSender.h"
#include "Player/DiscordRoleChecker.h"
#include "Player/SMLWhitelistManager.h"
#include "Player/WhitelistConfig.h"

AWhitelistCommandInstance::AWhitelistCommandInstance() {
    CommandName = TEXT("whitelist");
    MinNumberOfArguments = 1;
    Usage = NSLOCTEXT("SML", "ChatCommand.Whitelist.Usage",
        "/whitelist <add|remove|list|on|off|link> [player|discordId] - Manage the server whitelist");
}

// ---------------------------------------------------------------------------
// Internal helpers
// ---------------------------------------------------------------------------

/** Executes the privileged subcommand after Discord role verification passes. */
static void ExecutePrivilegedSubcommand(
    UCommandSender* Sender,
    const FString& SubCommand,
    const TArray<FString>& Arguments,
    const FWhitelistConfig& Config) {

    // /whitelist on
    if (SubCommand == TEXT("on")) {
        if (Config.bEnableWhitelist) {
            Sender->SendChatMessage(TEXT("Whitelist is already enabled."));
        } else {
            FWhitelistConfig Updated = Config;
            Updated.bEnableWhitelist = true;
            FWhitelistConfigManager::SetConfig(Updated);
            FWhitelistConfigManager::SaveConfig();
            Sender->SendChatMessage(TEXT("Whitelist enabled and saved to config."));
        }
        return;
    }

    // /whitelist off
    if (SubCommand == TEXT("off")) {
        if (!Config.bEnableWhitelist) {
            Sender->SendChatMessage(TEXT("Whitelist is already disabled."));
        } else {
            FWhitelistConfig Updated = Config;
            Updated.bEnableWhitelist = false;
            FWhitelistConfigManager::SetConfig(Updated);
            FWhitelistConfigManager::SaveConfig();
            Sender->SendChatMessage(TEXT("Whitelist disabled and saved to config."));
        }
        return;
    }

    // /whitelist add <player>
    if (SubCommand == TEXT("add")) {
        if (Arguments.Num() < 2) {
            Sender->SendChatMessage(TEXT("Usage: /whitelist add <playerName>"), FLinearColor::Red);
            return;
        }
        const FString PlayerName = Arguments[1];
        if (FSMLWhitelistManager::AddPlayer(PlayerName)) {
            Sender->SendChatMessage(FString::Printf(TEXT("Added '%s' to the whitelist."), *PlayerName));
        } else {
            Sender->SendChatMessage(FString::Printf(TEXT("'%s' is already on the whitelist."), *PlayerName));
        }
        return;
    }

    // /whitelist remove <player>
    if (SubCommand == TEXT("remove")) {
        if (Arguments.Num() < 2) {
            Sender->SendChatMessage(TEXT("Usage: /whitelist remove <playerName>"), FLinearColor::Red);
            return;
        }
        const FString PlayerName = Arguments[1];
        if (FSMLWhitelistManager::RemovePlayer(PlayerName)) {
            Sender->SendChatMessage(FString::Printf(TEXT("Removed '%s' from the whitelist."), *PlayerName));
        } else {
            Sender->SendChatMessage(FString::Printf(TEXT("'%s' is not on the whitelist."), *PlayerName));
        }
        return;
    }
}

// ---------------------------------------------------------------------------
// Command entry point
// ---------------------------------------------------------------------------

EExecutionStatus AWhitelistCommandInstance::ExecuteCommand_Implementation(
    UCommandSender* Sender, const TArray<FString>& Arguments, const FString& Label) {

    const FWhitelistConfig Config = FWhitelistConfigManager::GetConfig();
    const FString SubCommand = Arguments[0].ToLower();

    // ------------------------------------------------------------------
    // /whitelist link <discordUserId>
    // Any player may link their own account; no role check needed.
    // ------------------------------------------------------------------
    if (SubCommand == TEXT("link")) {
        if (Arguments.Num() < 2) {
            Sender->SendChatMessage(TEXT("Usage: /whitelist link <discordUserId>"), FLinearColor::Red);
            if (!Config.DiscordWhitelistChannelId.IsEmpty()) {
                Sender->SendChatMessage(
                    FString::Printf(TEXT("Find your Discord user ID and run this command in channel <#%s>."),
                        *Config.DiscordWhitelistChannelId));
            }
            return EExecutionStatus::BAD_ARGUMENTS;
        }
        const FString DiscordUserId = Arguments[1];
        const FString PlayerName = Sender->GetSenderName();
        FDiscordRoleChecker::LinkPlayer(PlayerName, DiscordUserId);
        Sender->SendChatMessage(
            FString::Printf(TEXT("Linked your account to Discord user ID %s."), *DiscordUserId));
        return EExecutionStatus::COMPLETED;
    }

    // ------------------------------------------------------------------
    // /whitelist list  – read-only, no role check required
    // ------------------------------------------------------------------
    if (SubCommand == TEXT("list")) {
        const TArray<FString> Players = FSMLWhitelistManager::GetWhitelistedPlayers();
        if (Players.IsEmpty()) {
            Sender->SendChatMessage(TEXT("Whitelist is empty."));
        } else {
            Sender->SendChatMessage(FString::Printf(TEXT("Whitelisted players (%d): %s"),
                Players.Num(), *FString::Join(Players, TEXT(", "))));
        }
        const FString StatusStr = Config.bEnableWhitelist ? TEXT("enabled") : TEXT("disabled");
        const FString ChannelHint = Config.WhitelistChannel.IsEmpty()
            ? TEXT("any channel")
            : FString::Printf(TEXT("channel '%s'"), *Config.WhitelistChannel);
        Sender->SendChatMessage(FString::Printf(
            TEXT("Whitelist: %s | Discord role ID: '%s' | Channel: %s"),
            *StatusStr, *Config.DiscordWhitelistRoleId, *ChannelHint));
        return EExecutionStatus::COMPLETED;
    }

    // ------------------------------------------------------------------
    // Privileged subcommands (add / remove / on / off)
    // Console senders bypass the Discord role check.
    // ------------------------------------------------------------------
    if (SubCommand != TEXT("add") && SubCommand != TEXT("remove") &&
        SubCommand != TEXT("on")  && SubCommand != TEXT("off")) {
        PrintCommandUsage(Sender);
        return EExecutionStatus::BAD_ARGUMENTS;
    }

    // Console / non-player senders are trusted without Discord verification
    if (!Sender->IsPlayerSender()) {
        ExecutePrivilegedSubcommand(Sender, SubCommand, Arguments, Config);
        return EExecutionStatus::COMPLETED;
    }

    // Determine whether Discord role checking is configured
    const bool bDiscordConfigured =
        !Config.DiscordBotToken.IsEmpty() &&
        !Config.DiscordGuildId.IsEmpty() &&
        !Config.DiscordWhitelistRoleId.IsEmpty();

    if (!bDiscordConfigured) {
        // Fall back to the plain WhitelistRole name check when Discord is not configured.
        const FString SenderName = Sender->GetSenderName();
        if (!Config.WhitelistRole.IsEmpty() &&
            !SenderName.Equals(Config.WhitelistRole, ESearchCase::IgnoreCase)) {
            Sender->SendChatMessage(
                FString::Printf(TEXT("You need the '%s' role to use this command."),
                    *Config.WhitelistRole),
                FLinearColor::Red);
            return EExecutionStatus::INSUFFICIENT_PERMISSIONS;
        }
        ExecutePrivilegedSubcommand(Sender, SubCommand, Arguments, Config);
        return EExecutionStatus::COMPLETED;
    }

    // Retrieve the Discord user ID linked to this player
    const FString PlayerName = Sender->GetSenderName();
    const FString DiscordUserId = FDiscordRoleChecker::GetDiscordUserId(PlayerName);

    if (DiscordUserId.IsEmpty()) {
        Sender->SendChatMessage(
            TEXT("Your Discord account is not linked. Run /whitelist link <discordUserId> first."),
            FLinearColor::Red);
        if (!Config.DiscordWhitelistChannelId.IsEmpty()) {
            Sender->SendChatMessage(
                FString::Printf(TEXT("Find your Discord user ID in channel <#%s>."),
                    *Config.DiscordWhitelistChannelId));
        }
        return EExecutionStatus::INSUFFICIENT_PERMISSIONS;
    }

    // Inform the player that the async check has started
    Sender->SendChatMessage(TEXT("Verifying Discord role, please wait..."));

    // Capture everything needed for the callback by value
    const FString BotToken    = Config.DiscordBotToken;
    const FString GuildId     = Config.DiscordGuildId;
    const FString RoleId      = Config.DiscordWhitelistRoleId;
    const FString CapturedSub = SubCommand;
    const TArray<FString> Args = Arguments;
    const FWhitelistConfig Cfg = Config;

    // Use a TWeakObjectPtr so the callback is safe if the sender disconnects
    // before the HTTP response arrives.
    TWeakObjectPtr<UCommandSender> WeakSender(Sender);

    FDiscordRoleChecker::CheckMemberHasRole(
        DiscordUserId, GuildId, RoleId, BotToken,
        [WeakSender, CapturedSub, Args, Cfg](bool bHasRole) {
            UCommandSender* ResolvedSender = WeakSender.Get();
            if (!ResolvedSender) return; // Player left before the reply arrived

            if (!bHasRole) {
                ResolvedSender->SendChatMessage(
                    FString::Printf(
                        TEXT("Access denied: your Discord account does not hold the required role (ID: %s)."),
                        *Cfg.DiscordWhitelistRoleId),
                    FLinearColor::Red);
                return;
            }

            ExecutePrivilegedSubcommand(ResolvedSender, CapturedSub, Args, Cfg);
        });

    // The actual work happens asynchronously in the callback above.
    return EExecutionStatus::COMPLETED;
}

