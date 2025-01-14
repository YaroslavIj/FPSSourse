// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "MyFunctionLibrary.h"
//
#include "FPS_PlayerState.generated.h"

/**
 * 
 */
UCLASS()
class FIRSTPERSONPROJECT_API AFPS_PlayerState : public APlayerState
{
	GENERATED_BODY()
	
public:

	UPROPERTY(BlueprintReadWrite, Replicated)
	int32 Kills = 0;

	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	TArray<FWeaponSlot> WeaponSlots;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	TArray<FWeaponTuningSlot> TuningWeaponSlots;
	UPROPERTY(BlueprintReadOnly, Replicated)
	int32 TeamNumber = 0;

	UFUNCTION(Server, Reliable)
	void InitInventorySlots_OnServer(const TArray<FWeaponSlot> &NewWeaponSlots, const TArray<FWeaponTuningSlot> &NewTuningSlots);
};
