// Fill out your copyright notice in the Description page of Project Settings.


#include "MyGameInstance.h"
#include "MyFunctionLibrary.h"
#include "OnlineSessionSettings.h"
#include "OnlineSubsystemUtils.h"
#include "OnlineSubsystem.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "Kismet/KismetStringLibrary.h"
#include "Net/UnrealNetwork.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerState.h"
#include "Templates/SharedPointer.h"
#include "GameFramework/PlayerController.h"

UMyGameInstance::UMyGameInstance()
{
	//OnFindSessionsCompleteDelegate = FOnFindSessionsCompleteDelegate::CreateUObject(this, &UMyGameInstance::OnFindSessionComplitedEvent);
	//OnCreateSessionCompleteDelegate = FOnCreateSessionCompleteDelegate::CreateUObject(this, &UMyGameInstance::OnCreateSessionComplitedEvent);
}

bool UMyGameInstance::GetWeaponSettingsByName(FName NameWeapon, FWeaponSettings& OutSettings)
{
	bool bIsFind = false;
	FWeaponSettings* WeaponSettingsRow;
	if (WeaponSettingsTaple)
	{
		WeaponSettingsRow = WeaponSettingsTaple->FindRow<FWeaponSettings>(NameWeapon, "", false);
		if (WeaponSettingsRow)
		{
			bIsFind = true;
			OutSettings = *WeaponSettingsRow;
		}
	}
	return bIsFind;
}

bool UMyGameInstance::GetWeaponTuningInfoByName(FName TuningName, FWeaponTuningInfo& OutInfo)
{
	bool bIsFind = false;
	FWeaponTuningInfo* TuningRow = nullptr;
	if (WeaponTuningInfoTaple)
	{
		TuningRow = WeaponTuningInfoTaple->FindRow<FWeaponTuningInfo>(TuningName, "", false);
		if(TuningRow != nullptr)
		{
			bIsFind = true;
			OutInfo = *TuningRow;
		}
	}
	return bIsFind;
}

void UMyGameInstance::SaveGameModeParameters(FGameModes NewGameMode)
{
	GameMode = NewGameMode;
}

void UMyGameInstance::FindSessionsToJoin(int32 MaxSearchResults)
{
	TMap<FName, FString> SessionSettings;
	IOnlineSubsystem* OnlineSubsystem = Online::GetSubsystem(GetWorld());
	IOnlineSessionPtr SessionInterface = OnlineSubsystem->GetSessionInterface();
	SessionSearch = MakeShareable(new FOnlineSessionSearch());
	//SessionSearch->QuerySettings.Set(SEARCH_LOBBIES, false, EOnlineComprisonOp::Equals);
	SessionSearch->MaxSearchResults = MaxSearchResults;
	APlayerState* PS = Cast<APlayerState>(UGameplayStatics::GetPlayerController(GetWorld(), 0)->PlayerState);
	//SessionSearch->AddOnFindSessionsCompleteDelegate_Handle( &UMyGameInstance::OnFindSessionComplited);
	//OnSearchSessionsComplited_Handle = SessionInterface->AddOnFindSessionsCompleteDelegate_Handle(OnFindSessionsCompleteDelegate);
	SessionInterface->OnFindSessionsCompleteDelegates.AddUObject(this, &UMyGameInstance::OnFindSessionComplitedEvent);
	SessionInterface->FindSessions(0, SessionSearch);
}

void UMyGameInstance::OnFindSessionComplitedEvent(bool bIsSuccess)
{	
	TArray<FSessionInfo> SessionsInfo;
	FString NameOfGameMode = UKismetStringLibrary::Conv_NameToString(GameMode.GameModeName);

	for(int32 i = 0; i < SessionSearch->SearchResults.Num(); i++)
	{	
		FString OutGameModeName;
		if (SessionSearch->SearchResults[i].Session.SessionSettings.Get("GameMode", OutGameModeName) && OutGameModeName == NameOfGameMode)
		{
			bool bIsSessionOpend = false;
			if(SessionSearch->SearchResults[i].Session.SessionSettings.Get("bIsSessionOpend", bIsSessionOpend) && bIsSessionOpend)
			{
				FSessionInfo SessionInfo;
				int32 MaxConnections = SessionSearch->SearchResults[i].Session.SessionSettings.NumPublicConnections;
				int32 OpenConnections = SessionSearch->SearchResults[i].Session.NumOpenPublicConnections;

				SessionInfo.CurrentPlayers = MaxConnections - OpenConnections;
				FString OutMapName;
				SessionSearch->SearchResults[i].Session.SessionSettings.Get("Map", OutMapName);
				SessionInfo.MapName = OutMapName;
				SessionsInfo.Add(SessionInfo);
			}
		}
	}
	OnFindSessionsCompleted.Broadcast(SessionsInfo);
	//OnFindSessionComplited_BP()
}

void UMyGameInstance::JoinToSession(int32 SessionNumber)
{
	if(IOnlineSubsystem* OnlineSubsystem = Online::GetSubsystem(GetWorld()))
	{
		if(IOnlineSessionPtr SessionInterface = OnlineSubsystem->GetSessionInterface())
		{
			APlayerState* PS = Cast<APlayerState>(UGameplayStatics::GetPlayerController(GetWorld(), 0)->PlayerState);
			if (PS)
			{
				SessionInterface->OnJoinSessionCompleteDelegates.AddUObject(this, &UMyGameInstance::OnJoinSessionCompleteEvent);
				SessionInterface->JoinSession(0, GameSessionName, SessionSearch->SearchResults[SessionNumber]);
			}
		}
	}
}

void UMyGameInstance::HostSession(bool bIsLan, int32 NumPublicConnections)
{
	if (IOnlineSubsystem* OnlineSubsystem = Online::GetSubsystem(GetWorld()))
	{
		if (IOnlineSessionPtr SessionInterface = OnlineSubsystem->GetSessionInterface())
		{
			if (APlayerState* PS = Cast<APlayerState>(UGameplayStatics::GetPlayerController(GetWorld(), 0)->PlayerState))
			{
				//TSharedPtr<FOnlineSessionSettings> SessionSettings = MakeShareable(new FOnlineSessionSettings());
				FOnlineSessionSettings SessionSettings;
				SessionSettings.NumPublicConnections = NumPublicConnections;
				SessionSettings.bIsLANMatch = bIsLan;
				SessionSettings.bShouldAdvertise = true;
				SessionSettings.bUsesPresence = true;
				SessionSettings.bAllowJoinInProgress = true;
				SessionSettings.bIsDedicated = false;
				SessionInterface->OnCreateSessionCompleteDelegates.AddUObject(this, &UMyGameInstance::OnCreateSessionCompleteEvent);
				SessionInterface->CreateSession(0, GameSessionName, SessionSettings);
			}
		}
	}
}

void UMyGameInstance::OnCreateSessionCompleteEvent(FName ServerName, bool bIsSuccess)
{
	if(bIsSuccess)
	{
		OnCreateSessionCompleted.Broadcast();
	}
}

void UMyGameInstance::OnJoinSessionCompleteEvent(FName SessionName, EOnJoinSessionCompleteResult::Type Result)
{
	if (Result == ONLINE_SUCCESS)
	{	
		if (UGameplayStatics::GetPlayerController(GetWorld(), 0))
		{
			if (IOnlineSubsystem* OnlineSubsystem = Online::GetSubsystem(GetWorld()))
			{
				if (IOnlineSessionPtr SessionInterface = OnlineSubsystem->GetSessionInterface())
				{
					if (APlayerController* PC = Cast<APlayerController>(UGameplayStatics::GetPlayerController(GetWorld(), 0)))
					{

						FString URL;
						if (SessionInterface->GetResolvedConnectString(SessionName, URL))
						{
							PC->ClientTravel(URL, ETravelType::TRAVEL_Absolute);
						}
					}
				}
			}
		}
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, TEXT("JoinSuccess"));

	}
	else
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("JoinFailed"));
	}
}

void UMyGameInstance::SetIsSessionOpend(bool bIsOpend)
{
	IOnlineSessionPtr SessionInterface = Online::GetSessionInterfaceChecked(GetWorld());
	FNamedOnlineSession* Session = SessionInterface->GetNamedSession(GameSessionName);
	if (Session) 
	{
		Session->SessionSettings.Set("bIsSessionOpend", bIsOpend, EOnlineDataAdvertisementType::ViaOnlineService);
		SessionInterface->UpdateSession(GameSessionName, Session->SessionSettings);
	}
}

void UMyGameInstance::ClearSessionSearch()
{
	SessionSearch->SearchResults.Empty();
}

void UMyGameInstance::UpdateSession_OnServer_Implementation()
{
	IOnlineSessionPtr SessionInterface = Online::GetSessionInterfaceChecked(GetWorld());
	FNamedOnlineSession* Session = SessionInterface->GetNamedSession(GameSessionName);
	if(Session)
	{
		Session->SessionSettings.Set("Map",GetWorld()->GetMapName(), EOnlineDataAdvertisementType::ViaOnlineService);
		FString NameOfGameMode = UKismetStringLibrary::Conv_NameToString(GameMode.GameModeName);
		Session->SessionSettings.Set("GameMode", NameOfGameMode, EOnlineDataAdvertisementType::ViaOnlineService);
		Session->SessionSettings.Set("bIsSessionOpend", true, EOnlineDataAdvertisementType::ViaOnlineService);
		SessionInterface->UpdateSession(GameSessionName, Session->SessionSettings);
	}
}

