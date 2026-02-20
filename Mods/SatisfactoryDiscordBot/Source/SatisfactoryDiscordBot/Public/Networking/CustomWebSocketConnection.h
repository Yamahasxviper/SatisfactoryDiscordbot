// Copyright SatisfactoryDiscordbot. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "CustomWebSocketConnection.generated.h"

class FWSClientConnection;

// ---------------------------------------------------------------------------
// Delegates
// ---------------------------------------------------------------------------

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnWSTextMessage,
	const FString&, Message);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnWSBinaryMessage,
	const TArray<uint8>&, Data,
	int32, Length);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnWSConnectionClosed,
	int32, StatusCode,
	const FString&, Reason);

// ---------------------------------------------------------------------------

/**
 * Blueprint-accessible handle to a single accepted WebSocket client connection.
 *
 * Instances are created automatically by UCustomWebSocketServer when a client
 * completes the WebSocket handshake.  Do not construct this class directly.
 *
 * All delegates are broadcast on the game thread.
 */
UCLASS(BlueprintType, NotBlueprintable)
class SATISFACTORYDISCORDBOT_API UCustomWebSocketConnection : public UObject
{
	GENERATED_BODY()

public:
	// -------------------------------------------------------------------------
	// Blueprint API
	// -------------------------------------------------------------------------

	/**
	 * Send a UTF-8 text message to this client.
	 * @param Message  The string to send.
	 */
	UFUNCTION(BlueprintCallable, Category = "Discord Bot|WebSocket")
	void SendText(const FString& Message);

	/**
	 * Send a raw binary message to this client.
	 * @param Data  The bytes to send.
	 */
	UFUNCTION(BlueprintCallable, Category = "Discord Bot|WebSocket")
	void SendBinary(const TArray<uint8>& Data);

	/**
	 * Initiate a graceful WebSocket close handshake.
	 * @param StatusCode  RFC 6455 status code (1000 = normal closure).
	 * @param Reason      Optional human-readable close reason.
	 */
	UFUNCTION(BlueprintCallable, Category = "Discord Bot|WebSocket")
	void Close(int32 StatusCode = 1000, const FString& Reason = TEXT("Normal closure"));

	/** Returns true while the underlying TCP connection is open. */
	UFUNCTION(BlueprintPure, Category = "Discord Bot|WebSocket")
	bool IsConnected() const;

	/** Returns the remote IP address and port (e.g. "127.0.0.1:54321"). */
	UFUNCTION(BlueprintPure, Category = "Discord Bot|WebSocket")
	FString GetRemoteAddress() const;

	// -------------------------------------------------------------------------
	// Delegates
	// -------------------------------------------------------------------------

	/** Fired when a UTF-8 text frame is received from the client. */
	UPROPERTY(BlueprintAssignable, Category = "Discord Bot|WebSocket")
	FOnWSTextMessage OnTextMessage;

	/** Fired when a binary frame is received from the client. */
	UPROPERTY(BlueprintAssignable, Category = "Discord Bot|WebSocket")
	FOnWSBinaryMessage OnBinaryMessage;

	/** Fired when the connection is closed (by either side). */
	UPROPERTY(BlueprintAssignable, Category = "Discord Bot|WebSocket")
	FOnWSConnectionClosed OnClosed;

	// -------------------------------------------------------------------------
	// Internal helpers – not exposed to Blueprint
	// -------------------------------------------------------------------------

	/** Called once by UCustomWebSocketServer after accepting the raw connection. */
	void InitWithInternalConnection(TSharedPtr<FWSClientConnection> InConnection);

	/**
	 * Drains the internal message queue and broadcasts the appropriate delegates.
	 * Must be called on the game thread (e.g. from UCustomWebSocketServer's ticker).
	 * Returns true while the connection is still alive after processing.
	 */
	bool ProcessPendingMessages();

private:
	TSharedPtr<FWSClientConnection> InternalConnection;
};
