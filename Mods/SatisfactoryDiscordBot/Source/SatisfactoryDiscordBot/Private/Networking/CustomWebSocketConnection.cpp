// Copyright SatisfactoryDiscordbot. All Rights Reserved.

#include "Networking/CustomWebSocketConnection.h"
#include "Networking/WSClientConnection.h"

void UCustomWebSocketConnection::InitWithInternalConnection(
	TSharedPtr<FWSClientConnection> InConnection)
{
	InternalConnection = MoveTemp(InConnection);
}

bool UCustomWebSocketConnection::ProcessPendingMessages()
{
	if (!InternalConnection.IsValid())
	{
		return false;
	}

	FWSMessage Msg;
	while (InternalConnection->TryGetNextMessage(Msg))
	{
		if (Msg.bIsClosed)
		{
			OnClosed.Broadcast(Msg.CloseStatusCode, Msg.CloseReason);
			return false; // Signal to the server that this connection is done.
		}
		else if (Msg.bIsText)
		{
			OnTextMessage.Broadcast(Msg.TextData);
		}
		else
		{
			const int32 Len = Msg.BinaryData.Num();
			OnBinaryMessage.Broadcast(Msg.BinaryData, Len);
		}
	}

	// Also return false if the underlying socket has gone away (abnormal close).
	return InternalConnection->IsConnected() || InternalConnection->HasPendingMessages();
}

// ---------------------------------------------------------------------------
// Blueprint API
// ---------------------------------------------------------------------------

void UCustomWebSocketConnection::SendText(const FString& Message)
{
	if (InternalConnection.IsValid())
	{
		InternalConnection->SendText(Message);
	}
}

void UCustomWebSocketConnection::SendBinary(const TArray<uint8>& Data)
{
	if (InternalConnection.IsValid())
	{
		InternalConnection->SendBinary(Data);
	}
}

void UCustomWebSocketConnection::Close(int32 StatusCode, const FString& Reason)
{
	if (InternalConnection.IsValid())
	{
		InternalConnection->SendClose(static_cast<uint16>(StatusCode), Reason);
	}
}

bool UCustomWebSocketConnection::IsConnected() const
{
	return InternalConnection.IsValid() && InternalConnection->IsConnected();
}

FString UCustomWebSocketConnection::GetRemoteAddress() const
{
	return InternalConnection.IsValid()
		? InternalConnection->GetRemoteAddress()
		: FString();
}
