// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "Engine/DataTable.h"
#include "MyFunctionLibrary.h"
#include "OnlineSessionSettings.h"
#include "Templates/SharedPointer.h"
#include "Net/UnrealNetwork.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerState.h"
#include "Templates/SharedPointer.h"
#include "OnlineSessionSettings.h"
#include "OnlineSubsystemUtils.h"
#include "OnlineSubsystem.h"
#include "Interfaces/OnlineSessionInterface.h"
//
#include "MyGameInstance.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnFindSessionsCompleted, const TArray<FSessionInfo>&, SessionsInfo);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnCreateSessionCompleted);


/**
 * 
 */
UCLASS()
class FIRSTPERSONPROJECT_API UMyGameInstance : public UGameInstance
{
	GENERATED_BODY()

public:

	UMyGameInstance();

	UPROPERTY(BlueprintReadWrite, BlueprintAssignable)
	FOnFindSessionsCompleted OnFindSessionsCompleted;
	UPROPERTY(BlueprintReadWrite, BlueprintAssignable)
	FOnCreateSessionCompleted OnCreateSessionCompleted;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "DataTable")
	UDataTable* WeaponSettingsTaple = nullptr;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "DataTable")
	UDataTable* WeaponTuningInfoTaple = nullptr;
	
	FName GameModeName = FName();
	int32 SumPointsToWin = 0;

	UPROPERTY(EditDefaultsOnly)
	FGameModes GameMode;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	TArray<FInventoryForPlayers> InventoryForPlayers;

	TSharedRef<FOnlineSessionSearch> SessionSearch = MakeShareable(new FOnlineSessionSearch());

	FOnFindSessionsCompleteDelegate OnFindSessionsCompleteDelegate;
	FDelegateHandle OnSearchSessionsComplited_Handle;

protected:


public:

	UFUNCTION(BlueprintCallable)
	bool GetWeaponSettingsByName(FName WeaponName, FWeaponSettings &OutSettings);
	UFUNCTION(BlueprintCallable)
	bool GetWeaponTuningInfoByName(FName TuningName, FWeaponTuningInfo& OutInfo);


	UFUNCTION(BlueprintCallable)
	void SaveGameModeParameters(FGameModes NewGameMode);

	UFUNCTION(BlueprintCallable, Server, Reliable)
	void UpdateSession_OnServer();
	UFUNCTION(BlueprintCallable)
	void FindSessionsToJoin(int32 MaxSearchResults);

	UFUNCTION()
	void OnFindSessionComplitedEvent(bool bIsSuccess);
	UFUNCTION(BlueprintCallable)
	void JoinToSession(int32 SessionNumber);
	UFUNCTION(BlueprintCallable)
	void HostSession(bool bIsLan, int32 NumPublicConnections);
	UFUNCTION()
	void OnCreateSessionCompleteEvent(FName ServerName, bool bIsSuccess);

	void OnJoinSessionCompleteEvent(FName SessionName, EOnJoinSessionCompleteResult::Type Result);
	UFUNCTION()
	void SetIsSessionOpend(bool bIsOpend);
	UFUNCTION(BlueprintCallable)
	void ClearSessionSearch();
};
