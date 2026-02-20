// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Module/GameInstanceModule.h"
#include "SMLWebSocketGameInstanceModule.generated.h"

class UDiscordGatewayWebSocket;

/**
 * Root GameInstanceModule for the SMLWebSocket plugin.
 *
 * SML auto-discovers this class because bRootModule = true (set in the
 * constructor).  On the INITIALIZATION lifecycle phase it reads the plugin
 * configuration file (Config/PluginSettings.ini) and, when bAutoConnect is
 * true and a BotToken is provided, automatically creates and connects a
 * UDiscordGatewayWebSocket to the Discord Gateway.
 *
 * Configuration keys (section [/SMLWebSocket/Connection] in PluginSettings.ini):
 *   BotToken     – Raw Discord bot token (no "Bot " prefix needed).
 *   Intents      – Integer bitmask of EDiscordGatewayIntent flags (default 33280).
 *   bAutoConnect – Set to true to connect automatically on game startup.
 */
UCLASS()
class SMLWEBSOCKET_API USMLWebSocketGameInstanceModule : public UGameInstanceModule
{
	GENERATED_BODY()

public:
	USMLWebSocketGameInstanceModule();

	virtual void DispatchLifecycleEvent(ELifecyclePhase Phase) override;

private:
	/** Reads PluginSettings.ini and initiates the Discord Gateway connection. */
	void AutoConnect();

	/** Active Discord Gateway WebSocket (kept as UPROPERTY to prevent GC). */
	UPROPERTY()
	UDiscordGatewayWebSocket* GatewayWebSocket;
};
