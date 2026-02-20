// Copyright Yamahasxviper. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

class FDiscordBotModule : public IModuleInterface
{
public:
	/**
	 * Returns a reference to the DiscordBot module.
	 * Call this from any C++ code (Blueprint functions, other mods, etc.) to
	 * access the module after it has been loaded.
	 *
	 * Example:
	 *   FDiscordBotModule& Mod = FDiscordBotModule::Get();
	 */
	static FDiscordBotModule& Get()
	{
		return FModuleManager::LoadModuleChecked<FDiscordBotModule>(TEXT("DiscordBot"));
	}

	/**
	 * Returns true if the module is loaded and ready to use.
	 * Safe to call at any point during the application lifetime.
	 */
	static bool IsAvailable()
	{
		return FModuleManager::Get().IsModuleLoaded(TEXT("DiscordBot"));
	}

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
};

