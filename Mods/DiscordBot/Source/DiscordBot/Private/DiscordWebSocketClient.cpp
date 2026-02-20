// Copyright Yamahasxviper. All Rights Reserved.
//
// Fully custom WebSocket client.
// The build includes UE's Sockets and SSL modules for other code in the mod
// to use if needed, but this file deliberately bypasses them and implements
// the full TCP + TLS + WebSocket stack directly so that it works regardless
// of whether the custom UE build ships those modules in usable form:
//   • TCP  — WinSock2 (Win64) / POSIX sockets (Linux)
//   • TLS  — OpenSSL API called directly (ThirdParty module)
//   • WebSocket — RFC 6455 framing implemented from scratch

#include "DiscordWebSocketClient.h"

#include "Misc/Base64.h"
#include "HAL/PlatformProcess.h"

// ---------------------------------------------------------------------------
// Platform socket + OpenSSL includes
// ---------------------------------------------------------------------------

#if PLATFORM_WINDOWS
    #include "Windows/AllowWindowsPlatformTypes.h"
    #pragma warning(push)
    #pragma warning(disable: 4005 4668)
    #include <winsock2.h>
    #include <ws2tcpip.h>
    #pragma warning(pop)
    #include "Windows/HideWindowsPlatformTypes.h"

    using SocketHandleType = SOCKET;
    static constexpr SocketHandleType InvalidSock = INVALID_SOCKET;
    #define SOCK_CLOSE(s) ::closesocket(s)
    #define SOCK_ERRNO    WSAGetLastError()
#else
    #include <sys/socket.h>
    #include <netinet/in.h>
    #include <arpa/inet.h>
    #include <netdb.h>
    #include <unistd.h>
    #include <errno.h>

    using SocketHandleType = int;
    static constexpr SocketHandleType InvalidSock = -1;
    #define SOCK_CLOSE(s) ::close(s)
    #define SOCK_ERRNO    errno
#endif

// OpenSSL — ThirdParty module, forward-declared as void* in the header
THIRD_PARTY_INCLUDES_START
#pragma warning(push)
#pragma warning(disable: 4191 4668)
#include <openssl/ssl.h>
#include <openssl/err.h>
#pragma warning(pop)
THIRD_PARTY_INCLUDES_END

// ---------------------------------------------------------------------------
// Internal helpers
// ---------------------------------------------------------------------------

DEFINE_LOG_CATEGORY_STATIC(LogDiscordWS, Log, All);

// Maximum accepted payload per frame (16 MB)
static constexpr int64 MaxFramePayload = 16LL * 1024 * 1024;

static inline SSL_CTX*         GetCtx(void* p)  { return static_cast<SSL_CTX*>(p); }
static inline SSL*             GetSSL(void* p)  { return static_cast<SSL*>(p);     }
static inline SocketHandleType GetSock(uintptr_t h) { return static_cast<SocketHandleType>(h); }

static void LogSSLErrors(const TCHAR* Where)
{
    char Buf[256];
    unsigned long Err;
    while ((Err = ERR_get_error()) != 0)
    {
        ERR_error_string_n(Err, Buf, sizeof(Buf));
        UE_LOG(LogDiscordWS, Error, TEXT("%s: %hs"), Where, Buf);
    }
}

// ---------------------------------------------------------------------------
// Construction / Destruction
// ---------------------------------------------------------------------------

FDiscordWebSocketClient::FDiscordWebSocketClient(const FString& InURL)
{
    ParseURL(InURL);
}

FDiscordWebSocketClient::~FDiscordWebSocketClient()
{
    Disconnect();
}

bool FDiscordWebSocketClient::ParseURL(const FString& InURL)
{
    FString Rest, Scheme;
    if (InURL.Split(TEXT("://"), &Scheme, &Rest))
    {
        bUseTLS = Scheme.Equals(TEXT("wss"), ESearchCase::IgnoreCase);
        Port    = bUseTLS ? 443 : 80;
    }
    else
    {
        Rest = InURL; bUseTLS = true; Port = 443;
    }

    FString Authority;
    int32 Idx = INDEX_NONE;
    if (Rest.FindChar(TCHAR('/'), Idx)) { Authority = Rest.Left(Idx); Path = Rest.Mid(Idx); }
    else                               { Authority = Rest; Path = TEXT("/"); }

    FString PortStr;
    if (Authority.Split(TEXT(":"), &Host, &PortStr)) Port = (uint16)FCString::Atoi(*PortStr);
    else                                             Host = Authority;

    UE_LOG(LogDiscordWS, Log, TEXT("URL  host=%s  port=%u  tls=%d  path=%s"),
        *Host, Port, bUseTLS, *Path);
    return true;
}

// ---------------------------------------------------------------------------
// Game-thread API
// ---------------------------------------------------------------------------

bool FDiscordWebSocketClient::Connect()
{
    if (Thread) { UE_LOG(LogDiscordWS, Warning, TEXT("Already running")); return false; }
    bStopRequest = false;
    Thread = FRunnableThread::Create(this, TEXT("DiscordWSClient"), 0, TPri_Normal);
    return Thread != nullptr;
}

void FDiscordWebSocketClient::Disconnect()
{
    bStopRequest = true;
    if (Thread) { Thread->WaitForCompletion(); delete Thread; Thread = nullptr; }
}

void FDiscordWebSocketClient::SendText(const FString& Message)
{
    FTCHARToUTF8 Conv(*Message);
    const uint8* Payload = reinterpret_cast<const uint8*>(Conv.Get());
    const int32  PayLen  = Conv.Length();

    TArray<uint8> Frame;
    Frame.Add(0x80 | static_cast<uint8>(EWSOpcode::Text)); // FIN + Text

    if      (PayLen <= 125)   { Frame.Add(0x80 | static_cast<uint8>(PayLen)); }
    else if (PayLen <= 65535) { Frame.Add(0x80 | 126); Frame.Add(PayLen>>8); Frame.Add(PayLen&0xFF); }
    else
    {
        Frame.Add(0x80 | 127);
        for (int i = 7; i >= 0; --i) Frame.Add((PayLen >> (i*8)) & 0xFF);
    }

    uint8 Mask[4];
    for (int i = 0; i < 4; ++i) Mask[i] = static_cast<uint8>(FMath::Rand() & 0xFF);
    Frame.Append(Mask, 4);
    for (int32 i = 0; i < PayLen; ++i) Frame.Add(Payload[i] ^ Mask[i & 3]);

    OutboundFrames.Enqueue(MoveTemp(Frame));
}

// ---------------------------------------------------------------------------
// FRunnable
// ---------------------------------------------------------------------------

bool FDiscordWebSocketClient::Init() { return true; }

uint32 FDiscordWebSocketClient::Run()
{
    UE_LOG(LogDiscordWS, Log, TEXT("I/O thread started"));

    if (!TCPConnect())       { PushInbound(EWSMessageKind::Error, TEXT("TCP connect failed")); return 1; }
    if (bUseTLS && !TLSConnect()) { PushInbound(EWSMessageKind::Error, TEXT("TLS handshake failed")); CloseSocket(); return 1; }
    if (!WebSocketHandshake()) { PushInbound(EWSMessageKind::Error, TEXT("WebSocket upgrade failed")); CloseSocket(); return 1; }

    PushInbound(EWSMessageKind::Connected);
    UE_LOG(LogDiscordWS, Log, TEXT("WebSocket ready"));

    while (!bStopRequest)
    {
        if (!DrainOutbound()) break;

        // Non-blocking data check via select (5 ms timeout)
        bool bHasData = false;
        {
            SocketHandleType Sock = GetSock(SockHandle);
            fd_set Fds; FD_ZERO(&Fds); FD_SET(Sock, &Fds);
            timeval Tv = { 0, 5000 };
#if PLATFORM_WINDOWS
            bHasData = ::select(0, &Fds, nullptr, nullptr, &Tv) > 0;
#else
            bHasData = ::select(Sock + 1, &Fds, nullptr, nullptr, &Tv) > 0;
#endif
            // Also check OpenSSL's internal buffer
            if (!bHasData && SSLConn)
                bHasData = SSL_pending(GetSSL(SSLConn)) > 0;
        }

        if (!bHasData) { FPlatformProcess::SleepNoStats(0.0f); continue; }

        EWSOpcode    Opcode;
        TArray<uint8> Payload;
        if (!ReadFrame(Opcode, Payload)) { PushInbound(EWSMessageKind::Error, TEXT("Frame read error")); break; }

        switch (Opcode)
        {
        case EWSOpcode::Text:
        {
            Payload.Add(0);
            PushInbound(EWSMessageKind::TextReceived,
                FString(UTF8_TO_TCHAR(reinterpret_cast<const ANSICHAR*>(Payload.GetData()))));
            break;
        }
        case EWSOpcode::Binary:
            PushInbound(EWSMessageKind::BinaryReceived,
                FString::FromHexBlob(Payload.GetData(), Payload.Num()));
            break;
        case EWSOpcode::Ping:  SendPong(Payload); break;
        case EWSOpcode::Pong:  break;
        case EWSOpcode::Close:
        {
            uint16 Code = 1000; FString Reason;
            if (Payload.Num() >= 2)
            {
                Code = (static_cast<uint16>(Payload[0]) << 8) | Payload[1];
                if (Payload.Num() > 2)
                {
                    TArray<uint8> RB(Payload.GetData()+2, Payload.Num()-2); RB.Add(0);
                    Reason = FString(UTF8_TO_TCHAR(reinterpret_cast<const ANSICHAR*>(RB.GetData())));
                }
            }
            SendCloseFrame(Code);
            PushInbound(EWSMessageKind::Closed, MoveTemp(Reason), Code);
            bStopRequest = true;
            break;
        }
        default: break;
        }
    }

    CloseSocket();
    UE_LOG(LogDiscordWS, Log, TEXT("I/O thread exiting"));
    return 0;
}

void FDiscordWebSocketClient::Stop() { bStopRequest = true; }

// ---------------------------------------------------------------------------
// TCPConnect — raw WinSock2 / POSIX
// ---------------------------------------------------------------------------

bool FDiscordWebSocketClient::TCPConnect()
{
#if PLATFORM_WINDOWS
    WSADATA Wsa;
    if (::WSAStartup(MAKEWORD(2,2), &Wsa) != 0)
    {
        UE_LOG(LogDiscordWS, Error, TEXT("WSAStartup failed: %d"), SOCK_ERRNO);
        return false;
    }
#endif

    addrinfo Hints = {};
    Hints.ai_family   = AF_UNSPEC;
    Hints.ai_socktype = SOCK_STREAM;
    Hints.ai_protocol = IPPROTO_TCP;

    FTCHARToUTF8 HostA(*Host);
    FTCHARToUTF8 PortA(*FString::FromInt(Port));

    addrinfo* List = nullptr;
    if (::getaddrinfo(HostA.Get(), PortA.Get(), &Hints, &List) != 0)
    {
        UE_LOG(LogDiscordWS, Error, TEXT("getaddrinfo failed for %s: %d"), *Host, SOCK_ERRNO);
        return false;
    }

    SocketHandleType Sock = InvalidSock;
    for (addrinfo* C = List; C; C = C->ai_next)
    {
        Sock = ::socket(C->ai_family, C->ai_socktype, C->ai_protocol);
        if (Sock == InvalidSock) continue;
        if (::connect(Sock, C->ai_addr, static_cast<int>(C->ai_addrlen)) == 0) break;
        SOCK_CLOSE(Sock); Sock = InvalidSock;
    }
    ::freeaddrinfo(List);

    if (Sock == InvalidSock)
    {
        UE_LOG(LogDiscordWS, Error, TEXT("Could not connect to %s:%d"), *Host, Port);
        return false;
    }

    SockHandle = static_cast<uintptr_t>(Sock);
    UE_LOG(LogDiscordWS, Log, TEXT("TCP connected to %s:%d"), *Host, Port);
    return true;
}

// ---------------------------------------------------------------------------
// TLSConnect — direct OpenSSL, no UE SSL module
// ---------------------------------------------------------------------------

bool FDiscordWebSocketClient::TLSConnect()
{
    OPENSSL_init_ssl(OPENSSL_INIT_LOAD_SSL_STRINGS | OPENSSL_INIT_LOAD_CRYPTO_STRINGS, nullptr);

    SSL_CTX* Ctx = SSL_CTX_new(TLS_client_method());
    if (!Ctx) { LogSSLErrors(TEXT("SSL_CTX_new")); return false; }

    SSL_CTX_set_min_proto_version(Ctx, TLS1_2_VERSION);
    // Peer verification is disabled here for mod use.
    // To enable: SSL_CTX_set_verify(Ctx, SSL_VERIFY_PEER, nullptr)
    // and provide a CA bundle via SSL_CTX_load_verify_locations().
    SSL_CTX_set_verify(Ctx, SSL_VERIFY_NONE, nullptr);

    SSL* Ssl = SSL_new(Ctx);
    if (!Ssl) { LogSSLErrors(TEXT("SSL_new")); SSL_CTX_free(Ctx); return false; }

    FTCHARToUTF8 HostA(*Host);
    SSL_set_tlsext_host_name(Ssl, HostA.Get()); // SNI
    SSL_set_fd(Ssl, static_cast<int>(SockHandle));

    if (SSL_connect(Ssl) != 1)
    {
        LogSSLErrors(TEXT("SSL_connect"));
        SSL_free(Ssl); SSL_CTX_free(Ctx);
        return false;
    }

    SSLCtx = Ctx; SSLConn = Ssl;
    UE_LOG(LogDiscordWS, Log, TEXT("TLS ready  cipher=%hs"), SSL_get_cipher(Ssl));
    return true;
}

// ---------------------------------------------------------------------------
// WebSocket HTTP upgrade (RFC 6455)
// ---------------------------------------------------------------------------

bool FDiscordWebSocketClient::WebSocketHandshake()
{
    uint8 RawKey[16];
    for (int i = 0; i < 16; ++i) RawKey[i] = static_cast<uint8>(FMath::Rand() & 0xFF);
    const FString Key = FBase64::Encode(RawKey, 16);

    FString Req = FString::Printf(
        TEXT("GET %s HTTP/1.1\r\nHost: %s\r\nUpgrade: websocket\r\n"
             "Connection: Upgrade\r\nSec-WebSocket-Key: %s\r\n"
             "Sec-WebSocket-Version: 13\r\n\r\n"),
        *Path, *Host, *Key);

    FTCHARToUTF8 ReqA(*Req);
    if (!RawWrite(ReqA.Get(), ReqA.Length()))
    {
        UE_LOG(LogDiscordWS, Error, TEXT("Failed to write HTTP upgrade request"));
        return false;
    }

    FString Resp;
    uint8 B = 0;
    while (true)
    {
        if (!RawRead(&B, 1)) return false;
        Resp.AppendChar(static_cast<TCHAR>(B));
        if (Resp.EndsWith(TEXT("\r\n\r\n"))) break;
        if (Resp.Len() > 8192) { UE_LOG(LogDiscordWS, Error, TEXT("Response header too large")); return false; }
    }

    if (!Resp.Contains(TEXT("101")))
    {
        UE_LOG(LogDiscordWS, Error, TEXT("No 101 response:\n%s"), *Resp);
        return false;
    }
    UE_LOG(LogDiscordWS, Log, TEXT("WebSocket upgrade accepted"));
    return true;
}

// ---------------------------------------------------------------------------
// RawRead / RawWrite — dispatches to SSL or plain socket
// ---------------------------------------------------------------------------

bool FDiscordWebSocketClient::RawRead(void* Buf, int32 Len)
{
    int32 Done = 0; uint8* Ptr = static_cast<uint8*>(Buf);
    while (Done < Len && !bStopRequest)
    {
        int32 n = 0;
        if (SSLConn)
        {
            n = SSL_read(GetSSL(SSLConn), Ptr+Done, Len-Done);
            if (n <= 0) { LogSSLErrors(TEXT("SSL_read")); return false; }
        }
        else
        {
            n = static_cast<int32>(::recv(GetSock(SockHandle),
                reinterpret_cast<char*>(Ptr+Done), Len-Done, 0));
            if (n <= 0) { UE_LOG(LogDiscordWS, Error, TEXT("recv: %d"), SOCK_ERRNO); return false; }
        }
        Done += n;
    }
    return !bStopRequest;
}

bool FDiscordWebSocketClient::RawWrite(const void* Buf, int32 Len)
{
    int32 Done = 0; const uint8* Ptr = static_cast<const uint8*>(Buf);
    while (Done < Len && !bStopRequest)
    {
        int32 n = 0;
        if (SSLConn)
        {
            n = SSL_write(GetSSL(SSLConn), Ptr+Done, Len-Done);
            if (n <= 0) { LogSSLErrors(TEXT("SSL_write")); return false; }
        }
        else
        {
            n = static_cast<int32>(::send(GetSock(SockHandle),
                reinterpret_cast<const char*>(Ptr+Done), Len-Done, 0));
            if (n <= 0) { UE_LOG(LogDiscordWS, Error, TEXT("send: %d"), SOCK_ERRNO); return false; }
        }
        Done += n;
    }
    return !bStopRequest;
}

// ---------------------------------------------------------------------------
// WebSocket frame codec (RFC 6455)
// ---------------------------------------------------------------------------

bool FDiscordWebSocketClient::ReadFrame(EWSOpcode& OutOpcode, TArray<uint8>& OutPayload)
{
    OutPayload.Reset();
    bool bFinal = false, bFirst = true;

    while (!bFinal && !bStopRequest)
    {
        uint8 Hdr[2];
        if (!RawRead(Hdr, 2)) return false;

        bFinal            = (Hdr[0] & 0x80) != 0;
        const uint8 Op    = (Hdr[0] & 0x0F);
        const bool Masked = (Hdr[1] & 0x80) != 0;
        uint64 PayLen     = (Hdr[1] & 0x7F);

        if      (PayLen == 126) { uint8 E[2]; if (!RawRead(E,2)) return false; PayLen = ((uint64)E[0]<<8)|E[1]; }
        else if (PayLen == 127) { uint8 E[8]; if (!RawRead(E,8)) return false; PayLen=0; for(int i=0;i<8;++i) PayLen=(PayLen<<8)|E[i]; }

        if (static_cast<int64>(PayLen) > MaxFramePayload)
        {
            UE_LOG(LogDiscordWS, Error, TEXT("Frame too large: %llu"), PayLen);
            return false;
        }

        uint8 MaskKey[4] = {};
        if (Masked && !RawRead(MaskKey, 4)) return false;

        const int32 Off = OutPayload.Num();
        OutPayload.SetNumUninitialized(Off + static_cast<int32>(PayLen));
        if (PayLen > 0 && !RawRead(OutPayload.GetData()+Off, static_cast<int32>(PayLen))) return false;
        if (Masked) for (int32 i=0;i<static_cast<int32>(PayLen);++i) OutPayload[Off+i]^=MaskKey[i&3];

        if (bFirst) { OutOpcode = static_cast<EWSOpcode>(Op); bFirst = false; }
        if (Op >= 0x8) bFinal = true; // control frames are never fragmented
    }
    return !bStopRequest;
}

bool FDiscordWebSocketClient::WriteFrame(EWSOpcode Opcode, const TArray<uint8>& Payload)
{
    const int32 Len = Payload.Num();
    TArray<uint8> Frame;
    Frame.Reserve(Len + 14);
    Frame.Add(0x80 | static_cast<uint8>(Opcode));

    if      (Len <= 125)   { Frame.Add(0x80 | static_cast<uint8>(Len)); }
    else if (Len <= 65535) { Frame.Add(0x80|126); Frame.Add(Len>>8); Frame.Add(Len&0xFF); }
    else
    {
        Frame.Add(0x80|127);
        for (int i=7;i>=0;--i) Frame.Add((Len>>(i*8))&0xFF);
    }

    uint8 Mask[4];
    for (int i=0;i<4;++i) Mask[i]=static_cast<uint8>(FMath::Rand()&0xFF);
    Frame.Append(Mask, 4);
    for (int32 i=0;i<Len;++i) Frame.Add(Payload[i]^Mask[i&3]);

    return RawWrite(Frame.GetData(), Frame.Num());
}

bool FDiscordWebSocketClient::WriteFrame(EWSOpcode Opcode, const FString& Text)
{
    FTCHARToUTF8 Conv(*Text);
    TArray<uint8> Bytes(reinterpret_cast<const uint8*>(Conv.Get()), Conv.Length());
    return WriteFrame(Opcode, Bytes);
}

void FDiscordWebSocketClient::SendPong(const TArray<uint8>& Data)  { WriteFrame(EWSOpcode::Pong, Data); }

void FDiscordWebSocketClient::SendCloseFrame(uint16 Code)
{
    TArray<uint8> P;
    P.Add(static_cast<uint8>(Code>>8));
    P.Add(static_cast<uint8>(Code&0xFF));
    WriteFrame(EWSOpcode::Close, P);
}

bool FDiscordWebSocketClient::DrainOutbound()
{
    TArray<uint8> Frame;
    while (OutboundFrames.Dequeue(Frame))
        if (!RawWrite(Frame.GetData(), Frame.Num())) return false;
    return true;
}

// ---------------------------------------------------------------------------
// Cleanup
// ---------------------------------------------------------------------------

void FDiscordWebSocketClient::CloseSocket()
{
    if (SSLConn) { SSL_shutdown(GetSSL(SSLConn)); SSL_free(GetSSL(SSLConn)); SSLConn = nullptr; }
    if (SSLCtx)  { SSL_CTX_free(GetCtx(SSLCtx));  SSLCtx  = nullptr; }
    if (SockHandle != static_cast<uintptr_t>(-1))
    {
        SOCK_CLOSE(GetSock(SockHandle));
        SockHandle = static_cast<uintptr_t>(-1);
    }
#if PLATFORM_WINDOWS
    ::WSACleanup();
#endif
}

void FDiscordWebSocketClient::PushInbound(EWSMessageKind Kind, FString Payload, int32 Code)
{
    FWSInboundMessage M; M.Kind=Kind; M.Payload=MoveTemp(Payload); M.CloseCode=Code;
    InboundMessages.Enqueue(MoveTemp(M));
}
