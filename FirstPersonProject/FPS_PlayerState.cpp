// Fill out your copyright notice in the Description page of Project Settings.


#include "FPS_PlayerState.h"
#include "Net/UnrealNetwork.h"

void AFPS_PlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AFPS_PlayerState, Kills);
	DOREPLIFETIME(AFPS_PlayerState, TeamNumber);
}

void AFPS_PlayerState::InitInventorySlots_OnServer_Implementation(const TArray<FWeaponSlot>& NewWeaponSlots, const TArray< FWeaponTuningSlot> &NewTuningSlots)
{
	WeaponSlots = NewWeaponSlots;
	TuningWeaponSlots = NewTuningSlots;

}
