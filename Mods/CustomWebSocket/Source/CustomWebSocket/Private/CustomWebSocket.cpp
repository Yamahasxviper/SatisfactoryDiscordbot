// Copyright (c) 2024 Yamahasxviper
// Custom WebSocket Implementation - UE5 Native WebSocket Wrapper

#include "CustomWebSocket.h"
#include "WebSocketsModule.h"
#include "Misc/Base64.h"

DEFINE_LOG_CATEGORY(LogCustomWebSocket);

FCustomWebSocket::FCustomWebSocket()
	: bIsConnected(false)
{
}

FCustomWebSocket::~FCustomWebSocket()
{
	Disconnect();
}

bool FCustomWebSocket::Connect(const FString& URL)
{
	if (bIsConnected || WebSocketImpl.IsValid())
	{
		UE_LOG(LogCustomWebSocket, Warning, TEXT("Already connected or connecting"));
		return false;
	}

	if (!FModuleManager::Get().IsModuleLoaded(TEXT("WebSockets")))
	{
		FModuleManager::Get().LoadModule(TEXT("WebSockets"));
	}

	TMap<FString, FString> UpgradeHeaders;
	WebSocketImpl = FWebSocketsModule::Get().CreateWebSocket(URL, TEXT(""), UpgradeHeaders);

	if (!WebSocketImpl.IsValid())
	{
		UE_LOG(LogCustomWebSocket, Error, TEXT("Failed to create WebSocket for URL: %s"), *URL);
		OnError.ExecuteIfBound(TEXT("Failed to create WebSocket"));
		return false;
	}

	WebSocketImpl->OnConnected().AddRaw(this, &FCustomWebSocket::OnWebSocketConnected_Internal);
	WebSocketImpl->OnConnectionError().AddRaw(this, &FCustomWebSocket::OnWebSocketConnectionError_Internal);
	WebSocketImpl->OnClosed().AddRaw(this, &FCustomWebSocket::OnWebSocketClosed_Internal);
	WebSocketImpl->OnMessage().AddRaw(this, &FCustomWebSocket::OnWebSocketMessage_Internal);
	WebSocketImpl->OnBinaryMessage().AddRaw(this, &FCustomWebSocket::OnWebSocketBinaryMessage_Internal);

	UE_LOG(LogCustomWebSocket, Log, TEXT("Connecting to: %s"), *URL);
	WebSocketImpl->Connect();
	return true;
}

void FCustomWebSocket::Disconnect(int32 StatusCode, const FString& Reason)
{
	if (WebSocketImpl.IsValid())
	{
		WebSocketImpl->OnConnected().RemoveAll(this);
		WebSocketImpl->OnConnectionError().RemoveAll(this);
		WebSocketImpl->OnClosed().RemoveAll(this);
		WebSocketImpl->OnMessage().RemoveAll(this);
		WebSocketImpl->OnBinaryMessage().RemoveAll(this);

		if (bIsConnected)
		{
			WebSocketImpl->Close(StatusCode, Reason);
		}

		WebSocketImpl = nullptr;
	}

	if (bIsConnected)
	{
		bIsConnected = false;
		UE_LOG(LogCustomWebSocket, Log, TEXT("WebSocket disconnected"));
		OnClosed.ExecuteIfBound(StatusCode, Reason, true);
	}
	else
	{
		bIsConnected = false;
	}
}

bool FCustomWebSocket::SendText(const FString& Message)
{
	if (!WebSocketImpl.IsValid() || !bIsConnected)
	{
		UE_LOG(LogCustomWebSocket, Warning, TEXT("Cannot send text: not connected"));
		return false;
	}

	WebSocketImpl->Send(Message);
	return true;
}

bool FCustomWebSocket::SendBinary(const TArray<uint8>& Data)
{
	if (!WebSocketImpl.IsValid() || !bIsConnected)
	{
		UE_LOG(LogCustomWebSocket, Warning, TEXT("Cannot send binary: not connected"));
		return false;
	}

	WebSocketImpl->Send(Data.GetData(), Data.Num(), true);
	return true;
}

bool FCustomWebSocket::IsConnected() const
{
	return bIsConnected && WebSocketImpl.IsValid() && WebSocketImpl->IsConnected();
}

void FCustomWebSocket::Tick(float DeltaTime)
{
	// No-op: UE's native WebSocket module manages its own event dispatch
}

void FCustomWebSocket::OnWebSocketConnected_Internal()
{
	bIsConnected = true;
	UE_LOG(LogCustomWebSocket, Log, TEXT("========================================"));
	UE_LOG(LogCustomWebSocket, Log, TEXT("CustomWebSocket: Successfully connected!"));
	UE_LOG(LogCustomWebSocket, Log, TEXT("CustomWebSocket: WebSocket ready (native TLS support active)"));
	UE_LOG(LogCustomWebSocket, Log, TEXT("========================================"));
	OnConnected.ExecuteIfBound(true);
}

void FCustomWebSocket::OnWebSocketConnectionError_Internal(const FString& Error)
{
	bIsConnected = false;
	UE_LOG(LogCustomWebSocket, Error, TEXT("WebSocket connection error: %s"), *Error);
	OnConnected.ExecuteIfBound(false);
	OnError.ExecuteIfBound(Error);
}

void FCustomWebSocket::OnWebSocketClosed_Internal(int32 StatusCode, const FString& Reason, bool bWasClean)
{
	bIsConnected = false;
	UE_LOG(LogCustomWebSocket, Log, TEXT("WebSocket closed: %d - %s (clean: %s)"),
		StatusCode, *Reason, bWasClean ? TEXT("yes") : TEXT("no"));
	OnClosed.ExecuteIfBound(StatusCode, Reason, bWasClean);
}

void FCustomWebSocket::OnWebSocketMessage_Internal(const FString& Message)
{
	OnMessage.ExecuteIfBound(Message, true);
}

void FCustomWebSocket::OnWebSocketBinaryMessage_Internal(const TArray<uint8>& Data)
{
	// Discord Gateway uses JSON text messages; binary messages are unexpected
	FString Base64Data = FBase64::Encode(Data);
	OnMessage.ExecuteIfBound(Base64Data, false);
}
