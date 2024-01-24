// Maksym Khytra
#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "WebSocketManager.generated.h"

class IWebSocket;

DECLARE_MULTICAST_DELEGATE_OneParam(OnMessageReceivedDelegate, const FString&);
DECLARE_MULTICAST_DELEGATE(OnConnectionErrorDelegate);
DECLARE_MULTICAST_DELEGATE(OnConnectedDelegate);
DECLARE_MULTICAST_DELEGATE(OnClosedDelegate);

UCLASS()
class WEBSOCKET_API UWebSocketManager : public UObject
{
	GENERATED_BODY()
	
public:
	void Connect(const FString& WSUrl);
	void Close() const;
	void SendMessage(const FString& Message) const;
	virtual void BeginDestroy() override;
	
	OnConnectedDelegate OnConnected;
	OnMessageReceivedDelegate OnMessageReceived;
	OnClosedDelegate OnClosed;
	OnConnectionErrorDelegate OnConnectionError;

private:
	TSharedPtr<IWebSocket> WebSocket;
};
