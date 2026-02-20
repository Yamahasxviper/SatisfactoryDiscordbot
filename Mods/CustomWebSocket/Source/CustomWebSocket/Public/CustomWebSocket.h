// Copyright (c) 2024 Yamahasxviper
// Custom WebSocket Implementation - UE5 Native WebSocket Wrapper

#pragma once

#include "CoreMinimal.h"
#include "IWebSocket.h"

DECLARE_LOG_CATEGORY_EXTERN(LogCustomWebSocket, Log, All);

DECLARE_DELEGATE_OneParam(FOnWebSocketConnected, bool /* bSuccess */);
DECLARE_DELEGATE_TwoParams(FOnWebSocketMessage, const FString& /* Message */, bool /* bIsText */);
DECLARE_DELEGATE_ThreeParams(FOnWebSocketClosed, int32 /* StatusCode */, const FString& /* Reason */, bool /* bWasClean */);
DECLARE_DELEGATE_OneParam(FOnWebSocketError, const FString& /* Error */);

/**
 * WebSocket wrapper around Unreal Engine's native IWebSocket module
 *
 * This implementation:
 * - Works on ALL platforms (Win64, Linux, Mac, Dedicated Servers)
 * - Native TLS/SSL support for wss:// via UE's WebSocket module
 * - RFC 6455 compliant
 * - Fully asynchronous, event-driven callbacks
 * - No custom TLS or socket handling required
 */
class CUSTOMWEBSOCKET_API FCustomWebSocket
{
public:
	FCustomWebSocket();
	~FCustomWebSocket();

	/** Connect to WebSocket server */
	bool Connect(const FString& URL);

	/** Disconnect from server */
	void Disconnect(int32 StatusCode = 1000, const FString& Reason = TEXT(""));

	/** Send text message */
	bool SendText(const FString& Message);

	/** Send binary message */
	bool SendBinary(const TArray<uint8>& Data);

	/** Check if connected */
	bool IsConnected() const;

	/** Tick - no-op for UE native WebSocket (events fire automatically) */
	void Tick(float DeltaTime);

	/** Event delegates */
	FOnWebSocketConnected OnConnected;
	FOnWebSocketMessage OnMessage;
	FOnWebSocketClosed OnClosed;
	FOnWebSocketError OnError;

private:
	TSharedPtr<IWebSocket> WebSocketImpl;
	bool bIsConnected;

	void OnWebSocketConnected_Internal();
	void OnWebSocketConnectionError_Internal(const FString& Error);
	void OnWebSocketClosed_Internal(int32 StatusCode, const FString& Reason, bool bWasClean);
	void OnWebSocketMessage_Internal(const FString& Message);
	void OnWebSocketBinaryMessage_Internal(const void* Data, SIZE_T Size, bool bIsLastFragment);
};
