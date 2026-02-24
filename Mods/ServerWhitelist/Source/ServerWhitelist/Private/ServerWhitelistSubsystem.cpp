#include "ServerWhitelistSubsystem.h"
#include "ServerWhitelistManager.h"
#include "GameFramework/GameModeBase.h"
#include "GameFramework/GameSession.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/PlayerState.h"

DEFINE_LOG_CATEGORY_STATIC(LogServerWhitelistSubsystem, Log, All);

void UServerWhitelistSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	// Load the whitelist from disk so it is ready before any player joins.
	FServerWhitelistManager::Load();

	// FGameModeEvents::GameModePostLoginEvent is a pure UE4 global multicast
	// delegate — no SML or any other mod framework required.
	PostLoginHandle = FGameModeEvents::GameModePostLoginEvent.AddUObject(
		this, &UServerWhitelistSubsystem::OnPostLogin);

	UE_LOG(LogServerWhitelistSubsystem, Display,
		TEXT("Server Whitelist subsystem initialised"));
}

void UServerWhitelistSubsystem::Deinitialize()
{
	FGameModeEvents::GameModePostLoginEvent.Remove(PostLoginHandle);
	Super::Deinitialize();
}

void UServerWhitelistSubsystem::OnPostLogin(AGameModeBase* GameMode, APlayerController* Controller)
{
	if (!Controller)
	{
		return;
	}

	// Local (listen-server host) players are never checked.
	if (Controller->IsLocalController())
	{
		return;
	}

	if (!FServerWhitelistManager::IsEnabled())
	{
		return;
	}

	const APlayerState* PS = Controller->GetPlayerState<APlayerState>();
	const FString PlayerName = PS ? PS->GetPlayerName() : FString();

	if (FServerWhitelistManager::IsWhitelisted(PlayerName))
	{
		return;
	}

	// Player is not whitelisted — kick them with a clear message.
	UE_LOG(LogServerWhitelistSubsystem, Warning,
		TEXT("Whitelist: kicking non-whitelisted player '%s'"), *PlayerName);

	if (GameMode && GameMode->GameSession)
	{
		GameMode->GameSession->KickPlayer(
			Controller,
			FText::FromString(TEXT("You are not on this server's whitelist. Contact the server admin to be added.")));
	}
}
