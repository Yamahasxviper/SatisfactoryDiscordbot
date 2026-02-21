#pragma once
#include "Modules/ModuleManager.h"

class FDiscordBotModule : public FDefaultGameModuleImpl
{
public:
    virtual void StartupModule() override;
    virtual void ShutdownModule() override;
};
