// Copyright Yamahasxviper. All Rights Reserved.
//
// Fully custom WebSocket client.
// The build also links UE's Sockets and SSL modules for use elsewhere in the
// mod, but this class bypasses both and talks directly to:
//   * UE Core  — FRunnable / FString / TArray / TQueue (basic types only)
//   * OpenSSL  — ThirdParty module, for TLS
//   * OS sockets — WinSock2 (Win64) or POSIX (Linux) for raw TCP

#pragma once

#include "CoreMinimal.h"
#include "HAL/Runnable.h"
#include "HAL/RunnableThread.h"
#include "Containers/Queue.h"

// ---------------------------------------------------------------------------
// Wire types shared between the I/O thread and the game thread
// ---------------------------------------------------------------------------

enum class EWSOpcode : uint8
{
    Continuation = 0x0,
    Text         = 0x1,
    Binary       = 0x2,
    Close        = 0x8,
    Ping         = 0x9,
    Pong         = 0xA,
};

enum class EWSMessageKind : uint8
{
    Connected,       // TCP + TLS handshake + HTTP upgrade complete
    TextReceived,    // A text frame arrived from the server
    BinaryReceived,  // A binary frame arrived (hex-encoded in Payload)
    Error,           // Fatal connection / protocol error
    Closed,          // Server sent a Close frame
};

struct FWSInboundMessage
{
    EWSMessageKind Kind      = EWSMessageKind::Error;
    FString        Payload;   // Text content, error description, or close reason
    int32          CloseCode = 0;
};

// ---------------------------------------------------------------------------
// FDiscordWebSocketClient
//
// Manages a single wss:// connection on a private background thread.
// All UE networking modules are bypassed — raw OS sockets and direct
// OpenSSL calls are used instead.
//
// Thread-safety contract
//   Game thread  : Connect(), Disconnect(), SendText(), drain InboundMessages
//   I/O thread   : reads OutboundFrames, writes InboundMessages
// ---------------------------------------------------------------------------
class DISCORDBOT_API FDiscordWebSocketClient : public FRunnable
{
public:
    explicit FDiscordWebSocketClient(const FString& InURL);
    virtual ~FDiscordWebSocketClient() override;

    // ---- Game-thread API --------------------------------------------------

    /** Start the background I/O thread and connect. */
    bool Connect();

    /** Signal the I/O thread to stop, then block until it exits (≤ 3 s). */
    void Disconnect();

    /**
     * Enqueue a UTF-8 text message to be sent as a WebSocket text frame.
     * Thread-safe: safe to call from the game thread at any time.
     */
    void SendText(const FString& Message);

    /** Receive queue — drain on the game thread only (SPSC). */
    TQueue<FWSInboundMessage, EQueueMode::Spsc> InboundMessages;

    // ---- FRunnable — do not call directly ---------------------------------
    virtual bool   Init() override;
    virtual uint32 Run()  override;
    virtual void   Stop() override;

private:
    // ---- URL components ---------------------------------------------------
    FString  Host;
    FString  Path;
    uint16   Port    = 443;
    bool     bUseTLS = true;

    // ---- Raw socket handle (uintptr_t holds SOCKET on Win64, int on Linux) -
    uintptr_t SockHandle = static_cast<uintptr_t>(-1);

    // ---- OpenSSL handles stored as void* to keep the header clean ---------
    // Actual types: SSL_CTX* and SSL*  (resolved in the .cpp only)
    void* SSLCtx = nullptr;
    void* SSLConn = nullptr;

    // ---- Outbound queue (game thread writes, I/O thread reads, SPSC) ------
    TQueue<TArray<uint8>, EQueueMode::Spsc> OutboundFrames;

    // ---- Thread lifecycle -------------------------------------------------
    FRunnableThread* Thread       = nullptr;
    FThreadSafeBool  bStopRequest { false };

    // ---- Private I/O helpers (I/O thread only) ----------------------------
    bool ParseURL(const FString& InURL);
    bool TCPConnect();
    bool TLSConnect();
    bool WebSocketHandshake();
    bool RawRead(void* Buf, int32 Len);
    bool RawWrite(const void* Buf, int32 Len);
    bool ReadFrame(EWSOpcode& OutOpcode, TArray<uint8>& OutPayload);
    bool WriteFrame(EWSOpcode Opcode, const TArray<uint8>& Payload);
    bool WriteFrame(EWSOpcode Opcode, const FString& Text);
    void SendPong(const TArray<uint8>& Data);
    void SendCloseFrame(uint16 Code = 1000);
    bool DrainOutbound();
    void CloseSocket();
    void PushInbound(EWSMessageKind Kind, FString Payload = TEXT(""), int32 Code = 0);
};

