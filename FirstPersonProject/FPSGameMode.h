// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "MyFunctionLibrary.h"
//
#include "FPSGameMode.generated.h"

class AFPS_PlayerController;
class APickUpAmmo;
class APickUpWeapon;
UCLASS()
class FIRSTPERSONPROJECT_API AFPSGameMode : public AGameModeBase
{
	GENERATED_BODY()
public:

	//FTimerHandle PlayerRestartTimer;

	UPROPERTY(BlueprintReadOnly)
	TArray<APlayerController*> PlayerControllers;

	TArray<FTimerHandle> PlayerRestartTimers;

	UPROPERTY(BlueprintReadOnly)
	int32 SumTeams = 2;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TArray<int32> SumPlayersInTeams;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TArray<int32> SumTeamKills;
	UPROPERTY(BlueprintReadWrite)
	FName GameModeName;
	UPROPERTY(BlueprintReadWrite)
	int32 SumPointsToWin = 0;
	UPROPERTY(BlueprintReadOnly)
	FGameModes GameMode;

	TArray<AFPS_PlayerController*> PlayersWaitingRestart;

	UPROPERTY(EditDefaultsOnly)
	float SpawnWeaponChance = 0.4f;
	UPROPERTY(EditDefaultsOnly)
	float SpawnAmmoChance = 0.3f;

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<APickUpWeapon> PickUpWeaponClass;
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<APickUpAmmo> PickUpAmmoClass;
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<AFirstPersonCharacter> CharacterClass;
protected:

public:

	void IncrementTeamKills(int32 TeamNumber, AFPS_PlayerController* Killer);
	virtual void BeginPlay() override;
	virtual void HandleStartingNewPlayer_Implementation(APlayerController* NewPlayer) override;

	void PlayerRestart(AFPS_PlayerController* Controller);
	void PlayerDeath(AFPS_PlayerController* Controller);
	//void RestartPlayerByTimer();

	UFUNCTION(BlueprintNativeEvent)
	void SpawnSpectatorPawn(APlayerController* Controller, APawn* SpectatingTargetPawn);
	UFUNCTION()
	void UpdatePlayersListForPlayers();
};
