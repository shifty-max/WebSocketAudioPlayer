// Maksym Khytra

#include "WebSocketManager.h"
#include "IWebSocket.h"
#include "WebSocketsModule.h"

void UWebSocketManager::Connect(const FString& WSUrl)
{
	if (FModuleManager::Get().IsModuleLoaded("WebSockets") == false)
	{
		FModuleManager::Get().LoadModule("WebSockets");
	}

	WebSocket = FWebSocketsModule::Get().CreateWebSocket(WSUrl);

	WebSocket->OnConnected().AddLambda([this]()
		{
			OnConnected.Broadcast();
			GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Green, "Connected to WS");
		});
	WebSocket->OnConnectionError().AddLambda([this](const FString& Error)
		{
			OnConnectionError.Broadcast();
			GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Red, Error);
		});
	WebSocket->OnClosed().AddLambda([this](int32 StatusCode, const FString& Reason, bool bWasClean)
		{
			OnClosed.Broadcast();
			GEngine->AddOnScreenDebugMessage(-1, 15.0f, bWasClean ? FColor::Green : FColor::Red, "Connection closed " + Reason);
		});

	WebSocket->OnMessage().AddLambda([this](const FString& MessageString)
		{
			OnMessageReceived.Broadcast(MessageString);
			UE_LOG(LogTemp, Warning, TEXT("WS Message Received: %s"), *MessageString);
		});
	WebSocket->OnMessageSent().AddLambda([](const FString& MessageString)
		{
			UE_LOG(LogTemp, Warning, TEXT("Message to WS Sent: %s"), *MessageString);
		});

	WebSocket->Connect();
}

void UWebSocketManager::Close() const
{
	if (WebSocket.IsValid())
	{
		WebSocket->Close();
	}
}

void UWebSocketManager::SendMessage(const FString& Message) const
{
	if (WebSocket.IsValid())
	{
		WebSocket->Send(Message);
	}
}

void UWebSocketManager::BeginDestroy()
{
	Close();
	OnConnected.Clear();
	OnClosed.Clear();
	OnConnectionError.Clear();
	OnMessageReceived.Clear();
	WebSocket.Reset();
	Super::BeginDestroy();
}
