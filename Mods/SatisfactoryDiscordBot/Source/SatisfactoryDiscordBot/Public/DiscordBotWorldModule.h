// Copyright SatisfactoryDiscordbot. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Module/GameWorldModule.h"
#include "Networking/CustomWebSocketServer.h"
#include "DiscordBotWorldModule.generated.h"

/**
 * SML Game World Module for the SatisfactoryDiscordBot mod.
 *
 * This class is the standard Satisfactory Mod Loader (SML) integration point.
 * Because bRootModule is set to true it is automatically discovered by SML
 * and instantiated once per game world – you do not need to spawn it manually.
 *
 * How to use in your project:
 *   1. Open Unreal Editor with the mod loaded.
 *   2. Create a Blueprint subclass of UDiscordBotWorldModule.
 *   3. In the Blueprint Class Defaults, set WebSocketPort to the port you want.
 *   4. Set bRootModule = true on the Blueprint (it inherits this flag).
 *   5. The server will start automatically when the game world initialises and
 *      stop when it is torn down.
 *   6. Retrieve the running server from any Blueprint:
 *      "Get Discord Bot World Module" node → WebSocketServer property.
 */
UCLASS(Blueprintable)
class SATISFACTORYDISCORDBOT_API UDiscordBotWorldModule : public UGameWorldModule
{
	GENERATED_BODY()

public:
	UDiscordBotWorldModule();

	// -------------------------------------------------------------------------
	// Configuration (set in Blueprint Class Defaults)
	// -------------------------------------------------------------------------

	/**
	 * TCP port the WebSocket server will listen on.
	 * Change this in the Blueprint Class Defaults of your subclass.
	 * Default: 8765.
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Discord Bot|WebSocket",
		meta = (ClampMin = "1", ClampMax = "65535"))
	int32 WebSocketPort{8765};

	/**
	 * Set to true to enable TLS (wss://) encryption on the WebSocket server.
	 * Requires TLSCertificatePath and TLSPrivateKeyPath to be filled in.
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Discord Bot|WebSocket|TLS")
	bool bUseTLS{false};

	/**
	 * Absolute path to the PEM-encoded TLS certificate file on the server host.
	 * Only used when bUseTLS is true.  Example: /etc/ssl/certs/myserver.crt
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Discord Bot|WebSocket|TLS")
	FString TLSCertificatePath;

	/**
	 * Absolute path to the PEM-encoded TLS private key file on the server host.
	 * Only used when bUseTLS is true.  Example: /etc/ssl/private/myserver.key
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Discord Bot|WebSocket|TLS")
	FString TLSPrivateKeyPath;

	// -------------------------------------------------------------------------
	// Runtime state
	// -------------------------------------------------------------------------

	/**
	 * The live WebSocket server instance.
	 * Valid (non-null and listening) after the INITIALIZATION lifecycle phase.
	 * Use this reference to call SendText / SendBinary on connected clients,
	 * or to bind the OnClientConnected delegate.
	 */
	UPROPERTY(BlueprintReadOnly, Transient, Category = "Discord Bot|WebSocket")
	TObjectPtr<UCustomWebSocketServer> WebSocketServer;

	// -------------------------------------------------------------------------
	// Static accessor – call from any Blueprint
	// -------------------------------------------------------------------------

	/**
	 * Returns the UDiscordBotWorldModule for the world of the caller.
	 * Equivalent to: WorldModuleManager → FindModule("SatisfactoryDiscordBot").
	 *
	 * Returns nullptr if the module has not been initialised yet or if the mod
	 * is not installed in the current session.
	 */
	UFUNCTION(BlueprintPure, Category = "Discord Bot|WebSocket",
		meta = (WorldContext = "WorldContext", DisplayName = "Get Discord Bot World Module"))
	static UDiscordBotWorldModule* GetModule(UObject* WorldContext);

	// -------------------------------------------------------------------------
	// UGameWorldModule / UModModule overrides
	// -------------------------------------------------------------------------

	/**
	 * Receives SML lifecycle events.
	 * - INITIALIZATION: creates and starts the WebSocket server.
	 * - Always calls Super so that base-class content registration still works.
	 */
	virtual void DispatchLifecycleEvent(ELifecyclePhase Phase) override;

	// -------------------------------------------------------------------------
	// UObject overrides
	// -------------------------------------------------------------------------

	/** Stops the WebSocket server when the world module is destroyed. */
	virtual void BeginDestroy() override;
};
