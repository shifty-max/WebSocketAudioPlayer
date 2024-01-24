// Maksym Khytra

#include "WSAudioPlayer.h"
#include "WebSocketManager.h"
#include "RuntimeAudioImporterLibrary.h"
#include "Sound/ImportedSoundWave.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"

#include "Misc/FileHelper.h" ///don't need if 

void UWSAudioPlayer::Connect()
{
	if (IsConnected)
	{
		return;
	}

	WebSocket = NewObject<UWebSocketManager>();
	WebSocket->OnMessageReceived.AddUObject(this, &UWSAudioPlayer::CacheReceivedMasseges);
	WebSocket->OnConnected.AddWeakLambda(this, [this] () {IsConnected = true;});
	WebSocket->OnClosed.AddWeakLambda(this, [this]() {IsConnected = false; });
	WebSocket->OnConnectionError.AddWeakLambda(this, [this]() {IsConnected = false; });
	if (Options.WSUrl == "")
	{
		GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Red, "Web Socket URL not provided,check options");
		return;
	}
	WebSocket->Connect(Options.WSUrl);
	Send(Options.MessageToSend);
	AudioImporter = URuntimeAudioImporterLibrary::CreateRuntimeAudioImporter();
	//GetWorld()->GetTimerManager().SetTimer(TimerHandle, this, &UWSAudioPlayer::PlayNext, Options.OffsetTimeToStartPlay, false);
}


void UWSAudioPlayer::Disconect() const
{
	if(IsValid(WebSocket))
	{
		WebSocket->Close();
	}
}

void UWSAudioPlayer::BeginDestroy()
{
	Disconect();
	Super::BeginDestroy();
}

void UWSAudioPlayer::Send(const FString& SentMessage) const
{
	if (!SentMessage.IsEmpty())
	{
		WebSocket->SendMessage(SentMessage);
	}
}

void UWSAudioPlayer::CacheReceivedMasseges(const FString& String)
{


	FString TrimmedString = String;

	if (!Options.LeftSubStringToTrim.IsEmpty() || !Options.RightSubStringToTrim.IsEmpty())
	{
		int leftTrim = String.Find(Options.LeftSubStringToTrim, ESearchCase::CaseSensitive);
		int rightTrim = String.Find(Options.RightSubStringToTrim, ESearchCase::CaseSensitive);
		if (leftTrim != INDEX_NONE && rightTrim != INDEX_NONE)
		{
			TrimNot64BaseText(TrimmedString, Options.LeftSubStringToTrim.Len(), Options.RightSubStringToTrim.Len());
		}
		else
		{
			GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Red, "Substring to trim must be checked!");
		}
	}
	StringArray.Add(TrimmedString);
	
	//Saving into files for debug
	
	FString SavePath = (FPaths::ProjectDir() + "WS_Message" + FString::FromInt(StringArray.Num()) + ".txt");
	FFileHelper::SaveStringToFile(String, *SavePath);
	FString SavePathTrim = (FPaths::ProjectDir() + "WS_Message_Trimmed" + FString::FromInt(StringArray.Num()) + ".txt");
	FFileHelper::SaveStringToFile(TrimmedString, *SavePathTrim);
	
	ConvertStringToSoundWave(TrimmedString);
}

void UWSAudioPlayer::TrimNot64BaseText(FString& String, const int LeftPartLenghtTrim, const int RightPartLenghtTrim)
{
	if (LeftPartLenghtTrim + RightPartLenghtTrim >= String.Len())
	{
		GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Red, "Can't Trim The Message Because It Is Smaller Than Chopped Parts");
		return;
	}

	String.RightChopInline(LeftPartLenghtTrim);
	String.LeftChopInline(RightPartLenghtTrim);
}

void UWSAudioPlayer::ConvertStringToSoundWave(const FString& String)
{
	TArray<uint8> Bytes;
	FBase64::Decode(String, Bytes);

	//Saving into mp3 files (txt need to be renamed)
	/*
	FString SavePathTrim = (FPaths::ProjectDir() + "WS_bytes" + FString::FromInt(Bytes.Num()) + ".txt");
	FFileHelper::SaveArrayToFile(Bytes, *SavePathTrim);
	*/

	AudioImporter = URuntimeAudioImporterLibrary::CreateRuntimeAudioImporter();
	if (!IsValid(AudioImporter))
	{
		AudioImporter = URuntimeAudioImporterLibrary::CreateRuntimeAudioImporter();
	}

	AudioImporter->OnResultNative.AddWeakLambda(this, [&](URuntimeAudioImporterLibrary* Importer, UImportedSoundWave* ImportedSoundWave, ERuntimeImportStatus Status)
		{
			if (Status == ERuntimeImportStatus::SuccessfulImport)
			{
				UE_LOG(LogTemp, Warning, TEXT("Successfully imported audio with sound wave %s"), *ImportedSoundWave->GetName());
				ImportedSoundWave->OnAudioPlaybackFinishedNative.AddUObject(this, &UWSAudioPlayer::PlayNext);
				Playlist.Add(ImportedSoundWave);
				++TotalSounds;
				if (Playlist.Num() >= Options.MinAudioTracksInPoolToStartPlay && !IsPlaying)
				{
					PlayNext();
				}
				else
				{

				}
			}
			else
			{
				UE_LOG(LogTemp, Error, TEXT("Failed to import audio"));
			}
		});		
	
	AudioImporter->ImportAudioFromBuffer(Bytes, ERuntimeAudioFormat::Auto);
}

void UWSAudioPlayer::PlayNext()
{
	//GEngine->AddOnScreenDebugMessage(-1, 25.0f, FColor::Cyan, "!!!!! Started TO Play");
	//GEngine->AddOnScreenDebugMessage(-1, 25.0f, FColor::Cyan, "!!!!! Total tracks - " + FString::FromInt(Playlist.Num()));
	if (Playlist.Num() > 0)
	{
			UGameplayStatics::PlaySound2D(GetWorld(), Playlist[0]);
			Playlist.RemoveAt(0);
			IsPlaying = true;
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Successfully imported audio with sound wave %d"), TotalSounds);
		AudioImporter = nullptr;
		IsPlaying = false;
	}
}
