// Copyright Coffee Stain Studios. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "LogViewerConfig.h"
#include "LogViewerSubsystem.generated.h"

typedef TSharedPtr<const struct FHttpRouteHandleInternal> FHttpRouteHandle;

// FHttpResultCallback is defined in IHttpRouter.h (private dep); duplicate it here
// to avoid exposing HTTPServer headers through our public header.
typedef TFunction<void(TUniquePtr<struct FHttpServerResponse>&&)> FLogViewerHttpResultCallback;

/**
 * ULogViewerSubsystem
 *
 * A GameInstance-level subsystem that starts a lightweight HTTP server on the
 * dedicated server so administrators can view the Unreal Engine server log in
 * any web browser without needing direct file-system access to the machine.
 *
 * How it works
 * ────────────
 *  • On Initialize() it binds two routes using UE's built-in HTTPServer module:
 *      GET /logs        → returns a self-refreshing HTML page with the last
 *                         LogLineCount lines of FactoryGame.log
 *      GET /logs/raw    → returns the same content as plain text
 *  • On Deinitialize() the routes are unbound and the listener stopped.
 *
 * Setup
 * ─────
 *  1. The mod starts automatically on every dedicated server.
 *  2. On first start a config file is written to
 *       <ServerDir>/FactoryGame/Configs/LogViewer.cfg
 *     Edit it to change the port, line count, or set an auth token.
 *  3. Open a browser to  http://<server-ip>:<Port>/logs
 *     If AuthToken is set:  http://<server-ip>:<Port>/logs?token=<AuthToken>
 *  4. Restart the server for config changes to take effect.
 *
 * Security note
 * ─────────────
 *  The log may contain sensitive information (player names, IP addresses, etc.).
 *  It is strongly recommended to set a non-empty AuthToken and to restrict
 *  the listening port at the firewall level.
 */
UCLASS(BlueprintType)
class LOGVIEWER_API ULogViewerSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	// ── USubsystem ────────────────────────────────────────────────────────────

	/** Restrict this subsystem to dedicated servers only. */
	virtual bool ShouldCreateSubsystem(UObject* Outer) const override;
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

private:
	// ── HTTP route handlers ───────────────────────────────────────────────────

	/** Serve the HTML log viewer page (GET /logs). */
	bool HandleHtmlRequest(const struct FHttpServerRequest& Request,
	                       const FLogViewerHttpResultCallback& OnComplete);

	/** Serve the raw plain-text log (GET /logs/raw). */
	bool HandleRawRequest(const struct FHttpServerRequest& Request,
	                      const FLogViewerHttpResultCallback& OnComplete);

	// ── Helpers ───────────────────────────────────────────────────────────────

	/** Read the last LogLineCount lines from FactoryGame.log.
	 *  Returns false and sets OutError if the file could not be read. */
	bool ReadLogExcerpt(FString& OutExcerpt, FString& OutError) const;

	/** Build the full HTML response body. */
	FString BuildHtmlPage(const FString& LogExcerpt, const FString& Error) const;

	/** Return true if the request carries a valid auth token (or no token is required). */
	bool IsAuthorized(const struct FHttpServerRequest& Request) const;

	// ── State ─────────────────────────────────────────────────────────────────

	/** Loaded configuration (populated in Initialize()). */
	FLogViewerConfig Config;

	/** Route handles – kept alive to allow unbinding on shutdown. */
	FHttpRouteHandle HtmlRouteHandle;
	FHttpRouteHandle RawRouteHandle;
};
