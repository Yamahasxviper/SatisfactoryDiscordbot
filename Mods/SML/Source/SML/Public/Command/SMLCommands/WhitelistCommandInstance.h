#pragma once
#include "Command/ChatCommandInstance.h"
#include "WhitelistCommandInstance.generated.h"

/**
 * In-game chat command for managing the SML server whitelist.
 *
 * Usage:
 *   /whitelist add <player>    - Add a player to the whitelist
 *   /whitelist remove <player> - Remove a player from the whitelist
 *   /whitelist list            - Show all whitelisted players
 *   /whitelist on              - Enable the whitelist
 *   /whitelist off             - Disable the whitelist
 *
 * Only senders whose name matches the configured WhitelistRole (from SML config)
 * may use this command.  When WhitelistChannel is non-empty the command is
 * described as belonging to that channel (informational; enforcement is handled
 * by the calling code if desired).
 */
UCLASS(MinimalAPI)
class AWhitelistCommandInstance : public AChatCommandInstance {
    GENERATED_BODY()
public:
    AWhitelistCommandInstance();
    EExecutionStatus ExecuteCommand_Implementation(UCommandSender* Sender, const TArray<FString>& Arguments, const FString& Label) override;
};
