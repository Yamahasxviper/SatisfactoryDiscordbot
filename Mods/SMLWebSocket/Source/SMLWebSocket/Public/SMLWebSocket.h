// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "HAL/Runnable.h"
#include "HAL/ThreadSafeBool.h"
#include "SMLWebSocket.generated.h"

class FSocket;
class FRunnableThread;

// ---------------------------------------------------------------------------
// Delegate declarations
// ---------------------------------------------------------------------------

/** Fired when the WebSocket connection is successfully established. */
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnSMLWebSocketConnected);

/** Fired when a connection attempt fails before or during the handshake. */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSMLWebSocketConnectionError, const FString&, Error);

/**
 * Fired when the WebSocket connection is closed (either by us or by the server).
 * @param StatusCode  WebSocket close status code (e.g. 1000 = normal closure).
 * @param Reason      Human-readable reason string sent with the close frame.
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnSMLWebSocketClosed, int32, StatusCode, const FString&, Reason);

/** Fired when a complete text message is received from the server. */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSMLWebSocketMessageReceived, const FString&, Message);

/**
 * Fired just before an automatic reconnect attempt begins.
 * @param AttemptNumber  1-based reconnect attempt index.
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSMLWebSocketReconnecting, int32, AttemptNumber);

/**
 * Fired when any complete message (text or binary) is received from the server.
 * @param Data      Raw bytes of the payload.
 * @param NumBytes  Number of valid bytes in Data.
 * @param bIsBinary True if this was a binary frame, false if text.
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnSMLWebSocketRawMessageReceived,
	const TArray<uint8>&, Data, int32, NumBytes, bool, bIsBinary);

// ---------------------------------------------------------------------------
// Forward declarations for internal worker class
// ---------------------------------------------------------------------------

class FSMLWebSocketWorker;

// ---------------------------------------------------------------------------
// USMLWebSocket
// ---------------------------------------------------------------------------

/**
 * Custom WebSocket client for use when the standard Unreal Engine "WebSockets"
 * module is not available (e.g. Coffee Stain's custom UE build for Satisfactory).
 *
 * Implemented on top of Unreal's low-level TCP socket API (ISocketSubsystem / FSocket)
 * and performs the RFC 6455 WebSocket upgrade handshake manually.
 *
 * Supports:  ws://  (plain TCP)
 * Use a TLS-terminating proxy if you need  wss://  connectivity.
 *
 * Usage (C++):
 * @code
 *   USMLWebSocket* WS = NewObject<USMLWebSocket>(this);
 *   WS->OnConnected.AddDynamic(this, &UMyClass::HandleConnected);
 *   WS->OnMessageReceived.AddDynamic(this, &UMyClass::HandleMessage);
 *   WS->Connect(TEXT("ws://localhost:8765/gateway"));
 * @endcode
 */
UCLASS(BlueprintType, Blueprintable)
class SMLWEBSOCKET_API USMLWebSocket : public UObject
{
	GENERATED_BODY()

public:
	USMLWebSocket();
	virtual ~USMLWebSocket() override;
	virtual void BeginDestroy() override;

	// -----------------------------------------------------------------------
	// Connection API
	// -----------------------------------------------------------------------

	/**
	 * Initiate a WebSocket connection.
	 * The URL must use the  ws://  scheme (e.g. "ws://localhost:8765/path").
	 * Connection is established asynchronously; listen to OnConnected /
	 * OnConnectionError for the result.
	 */
	UFUNCTION(BlueprintCallable, Category = "SML|WebSocket")
	void Connect(const FString& Url);

	/**
	 * Same as Connect(), but lets you supply additional HTTP headers that are
	 * sent during the WebSocket upgrade handshake (e.g. "Authorization").
	 */
	UFUNCTION(BlueprintCallable, Category = "SML|WebSocket")
	void ConnectWithHeaders(const FString& Url, const TMap<FString, FString>& Headers);

	// -----------------------------------------------------------------------
	// Reconnect settings
	// -----------------------------------------------------------------------

	/**
	 * When true, automatically retry the connection after a failure or
	 * unexpected disconnect.  This ensures internet access at server startup
	 * is not required at the exact moment Connect() is called.
	 * Defaults to true.
	 */
	UPROPERTY(BlueprintReadWrite, Category = "SML|WebSocket")
	bool bAutoReconnect;

	/**
	 * Delay in seconds before the first reconnect attempt.
	 * Subsequent attempts use exponential back-off, capped at 60 seconds.
	 * Defaults to 2.0.
	 */
	UPROPERTY(BlueprintReadWrite, Category = "SML|WebSocket")
	float ReconnectInitialDelaySeconds;

	/**
	 * Maximum number of reconnect attempts after the initial connection
	 * attempt.  0 means unlimited (keep retrying forever).
	 * Defaults to 0.
	 */
	UPROPERTY(BlueprintReadWrite, Category = "SML|WebSocket")
	int32 MaxReconnectAttempts;

	// -----------------------------------------------------------------------
	// Send API
	// -----------------------------------------------------------------------

	/**
	 * Send a UTF-8 text message to the server.
	 * Must be called only after OnConnected has fired.
	 */
	UFUNCTION(BlueprintCallable, Category = "SML|WebSocket")
	void Send(const FString& Data);

	/**
	 * Send a raw binary message to the server.
	 * Must be called only after OnConnected has fired.
	 */
	UFUNCTION(BlueprintCallable, Category = "SML|WebSocket")
	void SendBinary(const TArray<uint8>& Data);

	// -----------------------------------------------------------------------
	// Close API
	// -----------------------------------------------------------------------

	/**
	 * Initiate a clean WebSocket close handshake.
	 * @param Code    WebSocket status code (1000 = normal closure).
	 * @param Reason  Human-readable reason (max 123 bytes in UTF-8).
	 */
	UFUNCTION(BlueprintCallable, Category = "SML|WebSocket")
	void Close(int32 Code = 1000, const FString& Reason = TEXT(""));

	// -----------------------------------------------------------------------
	// State queries
	// -----------------------------------------------------------------------

	/** Returns true if the connection is open and ready to send/receive. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "SML|WebSocket")
	bool IsConnected() const;

	// -----------------------------------------------------------------------
	// Events (Blueprint-assignable delegates)
	// -----------------------------------------------------------------------

	/** Fired once on the game thread when the connection is fully established. */
	UPROPERTY(BlueprintAssignable, Category = "SML|WebSocket")
	FOnSMLWebSocketConnected OnConnected;

	/** Fired on the game thread if the connection attempt fails. */
	UPROPERTY(BlueprintAssignable, Category = "SML|WebSocket")
	FOnSMLWebSocketConnectionError OnConnectionError;

	/** Fired on the game thread when the connection is closed. */
	UPROPERTY(BlueprintAssignable, Category = "SML|WebSocket")
	FOnSMLWebSocketClosed OnClosed;

	/** Fired on the game thread when a text message is received. */
	UPROPERTY(BlueprintAssignable, Category = "SML|WebSocket")
	FOnSMLWebSocketMessageReceived OnMessageReceived;

	/** Fired on the game thread when any message (text or binary) is received. */
	UPROPERTY(BlueprintAssignable, Category = "SML|WebSocket")
	FOnSMLWebSocketRawMessageReceived OnRawMessageReceived;

	/**
	 * Fired on the game thread just before each automatic reconnect attempt.
	 * Use this to log reconnect activity or update UI.
	 */
	UPROPERTY(BlueprintAssignable, Category = "SML|WebSocket")
	FOnSMLWebSocketReconnecting OnReconnecting;

private:
	friend class FSMLWebSocketWorker;

	// The underlying TCP socket. Owned by this object; accessed only inside
	// SendFrame() (which is also called from the worker thread).
	FSocket* Socket;

	// Worker that runs the connect+handshake+read loop on a background thread.
	FSMLWebSocketWorker* Worker;
	FRunnableThread*     WorkerThread;

	// Shared stop flag – set by Close()/BeginDestroy() to signal the worker.
	FThreadSafeBool bStopping;
	FThreadSafeBool bConnected;

	// Serialises concurrent calls to SendFrame().
	FCriticalSection SendLock;

	// ------------------------------------------------------------------
	// Helpers called from the worker thread
	// ------------------------------------------------------------------

	/**
	 * Send a WebSocket frame on the current socket.
	 * Client frames MUST be masked (RFC 6455 §5.3).
	 * @return false if the send failed.
	 */
	bool SendFrame(uint8 Opcode, const uint8* Payload, uint64 PayloadLength);

	/** Tear down the worker thread and socket (safe to call multiple times). */
	void TearDown();
};

// ---------------------------------------------------------------------------
// USMLWebSocketBlueprintLibrary  –  factory helper
// ---------------------------------------------------------------------------

/**
 * Blueprint function library with utility functions for SMLWebSocket.
 */
UCLASS()
class SMLWEBSOCKET_API USMLWebSocketBlueprintLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/**
	 * Create a new USMLWebSocket instance.  You must call Connect() (or
	 * ConnectWithHeaders()) on the returned object to open the connection.
	 *
	 * @param WorldContextObject  Any UObject in the current world (for GC rooting).
	 * @return A new, unconnected WebSocket object.
	 */
	UFUNCTION(BlueprintCallable, Category = "SML|WebSocket",
		meta = (WorldContext = "WorldContextObject"))
	static USMLWebSocket* CreateWebSocket(UObject* WorldContextObject);
};
