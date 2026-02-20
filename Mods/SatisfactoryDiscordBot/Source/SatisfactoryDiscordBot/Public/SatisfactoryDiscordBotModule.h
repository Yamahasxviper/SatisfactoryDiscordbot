#pragma once

#include "Modules/ModuleManager.h"

class FSatisfactoryDiscordBotModule : public IModuleInterface
{
public:
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
};
