// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "WSAudioPlayer.generated.h"

class UWebSocketManager;
class UImportedSoundWave;
class URuntimeAudioImporterLibrary;
struct FTimerHandle;

USTRUCT(BlueprintType)
struct FWSAudioPlayerOptions
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Meta = (DisplayName = "Web Socket URL"), Category = "WS Audio Player|Substrings To Trim")
	FString WSUrl{""};
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Meta = (DisplayName = "Message to Web Socket"), Category = "WS Audio Player|Substrings To Trim")
	FString MessageToSend{""};
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Meta = (DisplayName = "Left Substrings To Trim For Correct Base64"), Category = "WS Audio Player|Substrings To Trim")
	FString LeftSubStringToTrim{""};
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Meta = (DisplayName = "Right Substrings To Trim For Correct Base64"), Category = "WS Audio Player|Substrings To Trim")
	FString RightSubStringToTrim{""};
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Meta = (DisplayName = "Delay Before Start Play"), Category = "WS Audio Player|Substrings To Trim")
	float OffsetTimeToStartPlay{0};
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Meta = (DisplayName = "Minimum Audio Tracks In Pool Before Start Play"), Category = "WS Audio Player|Substrings To Trim")
	int MinAudioTracksInPoolToStartPlay{1};
};

UCLASS(ClassGroup = (Audio), meta = (BlueprintSpawnableComponent))
class WEBSOCKET_API UWSAudioPlayer : public UActorComponent
{
	GENERATED_BODY()
public:
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Meta = (DisplayName = "Substrings To Trim"), Category = "WS Audio Player|Options")
	FWSAudioPlayerOptions Options;

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Connect to Web Socket"), Category = "WS Audio Player|Connect")
	void Connect();

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Connect to Web Socket"), Category = "WS Audio Player|Send")
	void Send(const FString& SentMessage = TEXT("")) const;

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Disconect from Web Socket"), Category = "WS Audio Player|Disonnect")
	void Disconect() const;

	virtual void BeginDestroy() override;

private:
	void CacheReceivedMasseges(const FString& String);
	void TrimNot64BaseText(FString& String, const int LeftPartLenghtTrim, const int RightPartLenghtTrim);
	void ConvertStringToSoundWave(const FString& String);
	void PlayNext();

	float SoundWavesReceivingPing;
	TArray<FString> StringArray;
	TArray<UImportedSoundWave*> Playlist {};
	FTimerHandle TimerHandle;
	UWebSocketManager* WebSocket {};
	URuntimeAudioImporterLibrary* AudioImporter;
	bool IsConnected { false };
	bool IsPlaying {false};

	//for debug
	int TotalSounds {0};
};


