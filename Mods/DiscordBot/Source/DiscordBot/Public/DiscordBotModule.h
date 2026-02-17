// Copyright (c) 2024 Yamahasxviper

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

class FDiscordBotModule : public IModuleInterface
{
public:
    /** IModuleInterface implementation */
    virtual void StartupModule() override;
    virtual void ShutdownModule() override;
};
