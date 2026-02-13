// Copyright (c) 2024 Discord Chat Bridge Contributors

#pragma once

#include "CoreMinimal.h"
#include "Module/GameInstanceModule.h"
#include "DiscordChatGameInstanceModule.generated.h"

UCLASS()
class DISCORDCHATBRIDGE_API UDiscordChatGameInstanceModule : public UGameInstanceModule
{
	GENERATED_BODY()

public:
	virtual void DispatchLifecycleEvent(ELifecyclePhase Phase) override;
};
