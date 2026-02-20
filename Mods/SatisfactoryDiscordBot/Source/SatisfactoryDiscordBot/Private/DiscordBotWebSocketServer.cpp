#include "DiscordBotWebSocketServer.h"
#include "IWebSocketNetworkingModule.h"
#include "INetworkingWebSocket.h"
#include "IWebSocketServer.h"

DEFINE_LOG_CATEGORY_STATIC(LogDiscordBotWebSocket, Log, All);

void UDiscordBotWebSocketServer::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	StartServer();
}

void UDiscordBotWebSocketServer::Deinitialize()
{
	StopServer();
	Super::Deinitialize();
}

void UDiscordBotWebSocketServer::StartServer()
{
	IWebSocketNetworkingModule* Module = FModuleManager::LoadModulePtr<IWebSocketNetworkingModule>("WebSocketNetworking");
	if (!Module)
	{
		UE_LOG(LogDiscordBotWebSocket, Error, TEXT("Failed to load WebSocketNetworking module"));
		return;
	}

	WebSocketServer = Module->CreateServer();
	if (!WebSocketServer.IsValid())
	{
		UE_LOG(LogDiscordBotWebSocket, Error, TEXT("Failed to create WebSocket server instance"));
		return;
	}

	FWebSocketClientConnectedCallBack ConnectedCallback;
	ConnectedCallback.BindUObject(this, &UDiscordBotWebSocketServer::OnClientConnectedCallback);

	if (!WebSocketServer->Init(static_cast<uint32>(Port), ConnectedCallback))
	{
		UE_LOG(LogDiscordBotWebSocket, Error, TEXT("Failed to start WebSocket server on port %d"), Port);
		WebSocketServer.Reset();
		return;
	}

	// Register a core ticker so the WebSocket server is pumped every frame
	TickHandle = FTSTicker::GetCoreTicker().AddTicker(
		FTickerDelegate::CreateUObject(this, &UDiscordBotWebSocketServer::OnTick));

	UE_LOG(LogDiscordBotWebSocket, Log, TEXT("WebSocket server listening on port %d"), Port);
}

void UDiscordBotWebSocketServer::StopServer()
{
	if (TickHandle.IsValid())
	{
		FTSTicker::GetCoreTicker().RemoveTicker(TickHandle);
		TickHandle.Reset();
	}

	ConnectedClients.Empty();
	WebSocketServer.Reset();

	UE_LOG(LogDiscordBotWebSocket, Log, TEXT("WebSocket server stopped"));
}

void UDiscordBotWebSocketServer::OnClientConnectedCallback(INetworkingWebSocket* Socket)
{
	const int32 ClientIndex = ConnectedClients.Add(Socket);

	// Wire up receive callback
	FWebSocketPacketReceivedCallBack ReceivedCallback;
	ReceivedCallback.BindLambda([this, ClientIndex](void* Data, int32 Size)
	{
		// Convert the raw UTF-8 bytes to an FString
		const FString Message = FString(UTF8_TO_TCHAR(static_cast<char*>(Data)));
		OnMessageReceived.Broadcast(ClientIndex, Message);
	});
	Socket->SetReceiveCallBack(ReceivedCallback);

	// Wire up close callback
	FWebSocketInfoCallBack ClosedCallback;
	ClosedCallback.BindLambda([this, Socket]()
	{
		const int32 Index = ConnectedClients.Find(Socket);
		if (Index != INDEX_NONE)
		{
			ConnectedClients.RemoveAt(Index);
			OnClientDisconnected.Broadcast(Index);
		}
	});
	Socket->SetSocketClosedCallBack(ClosedCallback);

	OnClientConnected.Broadcast(ClientIndex);

	UE_LOG(LogDiscordBotWebSocket, Log, TEXT("Client %d connected"), ClientIndex);
}

bool UDiscordBotWebSocketServer::OnTick(float DeltaTime)
{
	if (WebSocketServer.IsValid())
	{
		WebSocketServer->Tick();
	}
	return true;
}

void UDiscordBotWebSocketServer::BroadcastMessage(const FString& Message)
{
	if (!WebSocketServer.IsValid() || ConnectedClients.Num() == 0)
	{
		return;
	}

	const FTCHARToUTF8 Converter(*Message);
	const uint8* Data = reinterpret_cast<const uint8*>(Converter.Get());
	const int32 Size = Converter.Length();

	for (INetworkingWebSocket* Client : ConnectedClients)
	{
		Client->Send(Data, Size, false);
	}
}

void UDiscordBotWebSocketServer::SendMessageToClient(int32 ClientIndex, const FString& Message)
{
	if (!WebSocketServer.IsValid() || !ConnectedClients.IsValidIndex(ClientIndex))
	{
		return;
	}

	const FTCHARToUTF8 Converter(*Message);
	const uint8* Data = reinterpret_cast<const uint8*>(Converter.Get());
	const int32 Size = Converter.Length();

	ConnectedClients[ClientIndex]->Send(Data, Size, false);
}

int32 UDiscordBotWebSocketServer::GetConnectedClientCount() const
{
	return ConnectedClients.Num();
}
