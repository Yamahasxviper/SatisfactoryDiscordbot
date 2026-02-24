#pragma once
#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "ServerWhitelistSubsystem.generated.h"

/**
 * Game-instance subsystem that enforces the whitelist at player join time.
 *
 * Subscribes to the vanilla UE4 FGameModeEvents::GameModePostLoginEvent
 * delegate (no SML required) and kicks non-whitelisted players immediately
 * after PostLogin using AGameSession::KickPlayer.
 *
 * The subsystem is created automatically when the GameInstance starts because
 * UGameInstanceSubsystem::ShouldCreateSubsystem returns true by default.
 */
UCLASS()
class SERVERWHITELIST_API UServerWhitelistSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()
public:
	// Begin UGameInstanceSubsystem interface
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	// End UGameInstanceSubsystem interface

private:
	/** Called after every player PostLogin — enforces the whitelist. */
	void OnPostLogin(AGameModeBase* GameMode, APlayerController* Controller);

	FDelegateHandle PostLoginHandle;
};
