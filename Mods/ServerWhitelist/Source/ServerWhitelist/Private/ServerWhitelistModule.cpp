#include "ServerWhitelistModule.h"
#include "ServerWhitelistManager.h"
#include "HAL/IConsoleManager.h"
#include "Misc/OutputDeviceRedirector.h"

DEFINE_LOG_CATEGORY_STATIC(LogServerWhitelistModule, Log, All);

IMPLEMENT_MODULE(FServerWhitelistModule, ServerWhitelist);

void FServerWhitelistModule::StartupModule()
{
	// -----------------------------------------------------------------------
	// Server console commands
	// These work in the dedicated server console and any RCON tool.
	// Usage: open the server console and type the command.
	// -----------------------------------------------------------------------

	IConsoleManager::Get().RegisterConsoleCommand(
		TEXT("whitelist.on"),
		TEXT("Enable the server whitelist. Only whitelisted players will be able to join."),
		FConsoleCommandDelegate::CreateLambda([]()
		{
			FServerWhitelistManager::SetEnabled(true);
			UE_LOG(LogServerWhitelistModule, Display, TEXT("Whitelist ENABLED and saved."));
		}),
		ECVF_Default);

	IConsoleManager::Get().RegisterConsoleCommand(
		TEXT("whitelist.off"),
		TEXT("Disable the server whitelist. All players can join freely."),
		FConsoleCommandDelegate::CreateLambda([]()
		{
			FServerWhitelistManager::SetEnabled(false);
			UE_LOG(LogServerWhitelistModule, Display, TEXT("Whitelist disabled and saved."));
		}),
		ECVF_Default);

	IConsoleManager::Get().RegisterConsoleCommand(
		TEXT("whitelist.add"),
		TEXT("Add a player to the whitelist.  Usage: whitelist.add PlayerName"),
		FConsoleCommandWithArgsDelegate::CreateLambda([](const TArray<FString>& Args)
		{
			if (Args.Num() < 1)
			{
				UE_LOG(LogServerWhitelistModule, Warning, TEXT("Usage: whitelist.add <PlayerName>"));
				return;
			}
			const FString Name = Args[0];
			if (FServerWhitelistManager::AddPlayer(Name))
			{
				UE_LOG(LogServerWhitelistModule, Display, TEXT("Added '%s' to the whitelist."), *Name);
			}
			else
			{
				UE_LOG(LogServerWhitelistModule, Display, TEXT("'%s' is already on the whitelist."), *Name);
			}
		}),
		ECVF_Default);

	IConsoleManager::Get().RegisterConsoleCommand(
		TEXT("whitelist.remove"),
		TEXT("Remove a player from the whitelist.  Usage: whitelist.remove PlayerName"),
		FConsoleCommandWithArgsDelegate::CreateLambda([](const TArray<FString>& Args)
		{
			if (Args.Num() < 1)
			{
				UE_LOG(LogServerWhitelistModule, Warning, TEXT("Usage: whitelist.remove <PlayerName>"));
				return;
			}
			const FString Name = Args[0];
			if (FServerWhitelistManager::RemovePlayer(Name))
			{
				UE_LOG(LogServerWhitelistModule, Display, TEXT("Removed '%s' from the whitelist."), *Name);
			}
			else
			{
				UE_LOG(LogServerWhitelistModule, Display, TEXT("'%s' is not on the whitelist."), *Name);
			}
		}),
		ECVF_Default);

	IConsoleManager::Get().RegisterConsoleCommand(
		TEXT("whitelist.list"),
		TEXT("List all whitelisted players and show whether the whitelist is enabled."),
		FConsoleCommandDelegate::CreateLambda([]()
		{
			const bool bOn = FServerWhitelistManager::IsEnabled();
			const TArray<FString> All = FServerWhitelistManager::GetAll();
			UE_LOG(LogServerWhitelistModule, Display,
				TEXT("Whitelist is %s. Players (%d): %s"),
				bOn ? TEXT("ENABLED") : TEXT("disabled"),
				All.Num(),
				All.Num() > 0 ? *FString::Join(All, TEXT(", ")) : TEXT("(none)"));
		}),
		ECVF_Default);

	IConsoleManager::Get().RegisterConsoleCommand(
		TEXT("whitelist.status"),
		TEXT("Show whether the whitelist is currently enabled or disabled."),
		FConsoleCommandDelegate::CreateLambda([]()
		{
			UE_LOG(LogServerWhitelistModule, Display,
				TEXT("Whitelist is %s."),
				FServerWhitelistManager::IsEnabled() ? TEXT("ENABLED") : TEXT("disabled"));
		}),
		ECVF_Default);

	UE_LOG(LogServerWhitelistModule, Display,
		TEXT("Server Whitelist module started. Commands: whitelist.on/off/add/remove/list/status"));
}

void FServerWhitelistModule::ShutdownModule()
{
}
