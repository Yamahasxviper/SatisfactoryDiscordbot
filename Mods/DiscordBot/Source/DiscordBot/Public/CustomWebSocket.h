// Copyright (c) 2024 Yamahasxviper
// Custom WebSocket Implementation - Platform Agnostic

#pragma once

#include "CoreMinimal.h"
#include "Sockets.h"
#include "SocketSubsystem.h"
#include "IPAddress.h"

DECLARE_DELEGATE_OneParam(FOnWebSocketConnected, bool /* bSuccess */);
DECLARE_DELEGATE_TwoParams(FOnWebSocketMessage, const FString& /* Message */, bool /* bIsText */);
DECLARE_DELEGATE_ThreeParams(FOnWebSocketClosed, int32 /* StatusCode */, const FString& /* Reason */, bool /* bWasClean */);
DECLARE_DELEGATE_OneParam(FOnWebSocketError, const FString& /* Error */);

/**
 * Custom WebSocket implementation using only Unreal's Sockets module
 * 
 * This implementation:
 * - Works on ALL platforms (Win64, Linux, Mac, Servers)
 * - No external dependencies beyond Sockets and OpenSSL
 * - RFC 6455 compliant
 * - Supports TLS/SSL for wss://
 * - Fully asynchronous with callbacks
 * 
 * Use this when:
 * - Native WebSocket module not available
 * - Need guaranteed cross-platform support
 * - Want full control over WebSocket behavior
 */
class DISCORDBOT_API FCustomWebSocket
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
    bool IsConnected() const { return bIsConnected; }

    /** Tick for async operations */
    void Tick(float DeltaTime);

    /** Event delegates */
    FOnWebSocketConnected OnConnected;
    FOnWebSocketMessage OnMessage;
    FOnWebSocketClosed OnClosed;
    FOnWebSocketError OnError;

private:
    /** Socket instance */
    FSocket* Socket;

    /** Socket subsystem */
    ISocketSubsystem* SocketSubsystem;

    /** Connection state */
    bool bIsConnected;
    bool bIsSecure; // wss:// vs ws://

    /** Server info */
    FString ServerHost;
    int32 ServerPort;
    FString ServerPath;

    /** WebSocket handshake */
    FString SecWebSocketKey;
    bool bHandshakeComplete;

    /** Receive buffer */
    TArray<uint8> ReceiveBuffer;

    /** Send queue */
    TQueue<TArray<uint8>> SendQueue;

    /** Frame parsing state */
    bool bParsingFrame;
    uint8 CurrentOpcode;
    TArray<uint8> FramePayload;

    /** Helper methods */
    bool ParseURL(const FString& URL, FString& OutHost, int32& OutPort, FString& OutPath, bool& OutIsSecure);
    bool PerformTCPConnection(const FString& Host, int32 Port);
    bool PerformTLSHandshake();
    bool PerformWebSocketHandshake();
    bool ProcessReceivedData();
    bool ParseFrame(const TArray<uint8>& Data, int32& OutBytesConsumed);
    TArray<uint8> CreateFrame(uint8 Opcode, const TArray<uint8>& Payload, bool bMask = true);
    FString GenerateWebSocketKey();
    FString CalculateAcceptKey(const FString& Key);
    void HandleFrame(uint8 Opcode, const TArray<uint8>& Payload);
    void SendPong(const TArray<uint8>& PingPayload);
};
