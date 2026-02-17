// Copyright (c) 2024 Yamahasxviper
// Custom WebSocket Plugin Module

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

/**
 * Custom WebSocket Plugin Module
 * 
 * This module provides a platform-agnostic WebSocket implementation
 * that works on all platforms without external dependencies.
 * 
 * Features:
 * - RFC 6455 compliant WebSocket protocol
 * - Platform-agnostic (Win64, Linux, Mac, Servers)
 * - TLS/SSL support for wss://
 * - No dependency on Unreal's WebSocket module
 * - Uses only core Unreal modules (Sockets, OpenSSL)
 */
class FCustomWebSocketModule : public IModuleInterface
{
public:
	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
};
