// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"

//
#include "FPS_PlayerController.generated.h"


/**
 * 
 */
UCLASS()
class FIRSTPERSONPROJECT_API AFPS_PlayerController : public APlayerController
{
	GENERATED_BODY()
	

public:

	virtual void BeginPlay() override;
	UPROPERTY(Replicated, BlueprintReadOnly)
	int32 TeamNumber = 0;

	FTimerHandle PlayerRespawnTimer;
	UPROPERTY(Replicated, BlueprintReadWrite)
	APawn* SpectatingTarget = nullptr;
	UPROPERTY(BlueprintReadWrite)
	bool bCanRespawn = true;
	UPROPERTY(BlueprintReadOnly)
	bool bIsSwitchSpectatingTargetComplete = false;

	bool bIsSpectating = true;
protected:



public:

	virtual void SetupInputComponent() override;
	UFUNCTION(NetMulticast, Reliable)
	void UpdateTeamKills_Multicast(const TArray<int32> &SumTeamKills, int32 MaxTeamKills);

	UFUNCTION(BlueprintNativeEvent)
	void UpdateTeamKills_BP(const TArray<int32> &SumTeamKills, int32 MaxTeamKills);

	UFUNCTION(NetMulticast, Reliable)
	void InitInventorySlotsWidget_Multicast(int32 AmmoAmount, int32 MaxAmmoAmount, int32 ClipsAmount, int32 MaxClipsAmount, UTexture2D* Image);

	UFUNCTION(BlueprintNativeEvent)
	void InitInventorySlotsWidget_BP(int32 AmmoAmount, int32 MaxAmmoAmount, int32 ClipsAmount, int32 MaxClipsAmount, UTexture2D* Image);

	UFUNCTION(BlueprintNativeEvent)
	void InitHealthBarWidget_BP(float NewHealth, float ChangeValue);
	UFUNCTION(NetMulticast, Reliable)
	void InitHealthBarWidget_Multicast(float NewHealth, float ChangeValue);

	void PlayerDeath(float RespawnTime);
	UFUNCTION(Server, Reliable)
	void SetCanRespawn_OnServer();

	UFUNCTION(NetMulticast, Reliable)
	void IncrementPlayerKills_Multicast(int32 NewKills);
	UFUNCTION(BlueprintNativeEvent)
	void IncrementPlayerKills_BP(int32 NewKills);

	UFUNCTION(NetMulticast, Reliable, BlueprintCallable)
	void EventAfterJoin_Multicast();
	UFUNCTION(Server, Reliable)
	void TryToRepawn_OnServer();
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void SetSpectatingTarget_BP(APawn* NewTarget);

	UFUNCTION(BlueprintNativeEvent)
	void PlayerDeath_BP_OnServer(float RespawnTime);
	UFUNCTION(Server, Reliable)
	void SwitchSpectatingTarget_OnServer(float Value);
	UFUNCTION(BlueprintCallable)
	bool TryGetNewSpectatingTarget(APlayerController* Controller);
	UFUNCTION(NetMulticast, Reliable)
	void Respawn_Multicast(APawn* LastSpectatingTarget);
	UFUNCTION(NetMulticast, Reliable)
	void UpdatePlayersList_Multicast(const TArray<AFPS_PlayerState*>& PlayerStates);
	UFUNCTION(BlueprintNativeEvent)
	void UpdatePlayersList_BP(const TArray<AFPS_PlayerState*>& PlayerStates);
};
