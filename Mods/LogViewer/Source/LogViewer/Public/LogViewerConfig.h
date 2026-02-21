// Copyright Coffee Stain Studios. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

/**
 * Configuration for the Log Viewer mod.
 *
 * Values are read from and written to
 *   <ServerDir>/FactoryGame/Configs/LogViewer.cfg   (JSON format)
 *
 * The file is created automatically with defaults on the first server start.
 *
 * To access the log viewer open a browser to:
 *   http://<server-ip>:<Port>/logs
 *
 * If AuthToken is set, append it as a query parameter:
 *   http://<server-ip>:<Port>/logs?token=<AuthToken>
 */
struct LOGVIEWER_API FLogViewerConfig
{
	// ── Network ───────────────────────────────────────────────────────────────

	/**
	 * TCP port the built-in HTTP log viewer listens on.
	 * Choose a port that does not conflict with the Satisfactory game port (7777),
	 * query port (15777), or server API port (7778).
	 * Default: 8080
	 */
	int32 Port{ 8080 };

	// ── Content ───────────────────────────────────────────────────────────────

	/**
	 * Number of log lines shown on the /logs page.
	 * Set to 0 to show the entire log file (may be very large).
	 * Clamped to [0, 10000].
	 * Default: 200
	 */
	int32 LogLineCount{ 200 };

	// ── Security ──────────────────────────────────────────────────────────────

	/**
	 * Optional secret token required to access the /logs endpoint.
	 * When set, requests must include ?token=<AuthToken> in the URL.
	 * Leave empty to allow unauthenticated access (suitable for private LANs).
	 */
	FString AuthToken;

	/**
	 * Loads configuration from <ProjectDir>/Configs/LogViewer.cfg.
	 * If the file does not exist it is created with default values and those
	 * defaults are returned.
	 */
	static FLogViewerConfig LoadOrCreate();

	/** Returns the absolute path to the JSON config file. */
	static FString GetConfigFilePath();
};
