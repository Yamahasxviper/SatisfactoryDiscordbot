#pragma once
#include "Modules/ModuleManager.h"

class FServerWhitelistModule : public IModuleInterface
{
public:
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
};
