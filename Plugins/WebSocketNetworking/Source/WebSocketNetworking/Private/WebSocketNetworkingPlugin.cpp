// Copyright Epic Games, Inc. All Rights Reserved.

#include "IWebSocketNetworkingModule.h"
#include "WebSocketServer.h"
#include "Modules/ModuleManager.h"
#include "WebSocket.h"



class FWebSocketNetworkingPlugin : public IWebSocketNetworkingModule
{
	/** IModuleInterface implementation */
	virtual void StartupModule() override
	{
	}

	virtual void ShutdownModule() override
	{
	}


public:
	TUniquePtr<IWebSocketServer> CreateServer() override
	{
		return MakeUnique<FWebSocketServer>();
	}

	TSharedPtr<INetworkingWebSocket> CreateConnection(const FInternetAddr& ServerAddress) override
	{
		return MakeShared<FWebSocket>(ServerAddress);
	}

	TSharedPtr<INetworkingWebSocket> CreateConnection(const FString& Url, const FString& Protocol) override
	{
		return MakeShared<FWebSocket>(Url, Protocol);
	}

};

IMPLEMENT_MODULE(FWebSocketNetworkingPlugin, WebSocketNetworking)

DEFINE_LOG_CATEGORY(LogWebSocketNetworking);


