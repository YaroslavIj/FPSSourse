// Fill out your copyright notice in the Description page of Project Settings.


#include "FPS_PlayerController.h"
#include "Net/UnrealNetwork.h"
#include "FPSGameMode.h"
#include "FPS_PlayerState.h"
#include "MyGameInstance.h"
#include "FirstPersonCharacter.h"
#include "HealthComponent.h"
#include "BaseWeapon.h"

void AFPS_PlayerController::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AFPS_PlayerController, TeamNumber);
	DOREPLIFETIME(AFPS_PlayerController, SpectatingTarget);
}

void AFPS_PlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();
	//InputComponent = NewObject<UInputComponent>(this, TEXT("PCInputComponent"));
	//InputComponent->RegisterComponent();
	InputComponent->BindAction(TEXT("Respawn"), IE_Pressed, this, &AFPS_PlayerController::TryToRepawn_OnServer);
	InputComponent->BindAxis(TEXT("SwitchSpectatingTarget"), this, &AFPS_PlayerController::SwitchSpectatingTarget_OnServer);
}

void AFPS_PlayerController::UpdateTeamKills_Multicast_Implementation(const TArray<int32> &SumTeamKills, int32 MaxTeamKills)
{
	UpdateTeamKills_BP(SumTeamKills, MaxTeamKills);
}

void AFPS_PlayerController::UpdateTeamKills_BP_Implementation(const TArray<int32> &SumTeamKills, int32 MaxTeamKills)
{
	//BP
}

void AFPS_PlayerController::InitInventorySlotsWidget_Multicast_Implementation(int32 AmmoAmount, int32 MaxAmmoAmount, int32 ClipsAmount, int32 MaxClipsAmount, UTexture2D* Image)
{
	InitInventorySlotsWidget_BP(AmmoAmount, MaxAmmoAmount, ClipsAmount, MaxClipsAmount, Image);
}

void AFPS_PlayerController::InitInventorySlotsWidget_BP_Implementation(int32 AmmoAmount, int32 MaxAmmoAmount, int32 ClipsAmount, int32 MaxClipsAmount, UTexture2D* Image)
{
	//BP
}

void AFPS_PlayerController::InitHealthBarWidget_BP_Implementation(float NewHealth, float ChangeValue)
{
	//BP
}

void AFPS_PlayerController::InitHealthBarWidget_Multicast_Implementation(float NewHealth, float ChangeValue)
{
	InitHealthBarWidget_BP(NewHealth, ChangeValue);
}

void AFPS_PlayerController::PlayerDeath(float RespawnTime)
{
	if (GetWorld())
	{
		GetWorld()->GetTimerManager().SetTimer(PlayerRespawnTimer, this, &AFPS_PlayerController::SetCanRespawn_OnServer, RespawnTime, false);
		PlayerDeath_BP_OnServer(RespawnTime);
		bIsSpectating = true;
	}
}

void AFPS_PlayerController::SetSpectatingTarget_BP_Implementation(APawn* NewTarget)
{
	//BP
}

void AFPS_PlayerController::TryToRepawn_OnServer_Implementation()
{
	if(bCanRespawn)
	{
		if (GetWorld() && GetWorld()->GetAuthGameMode())
		{
			AFPSGameMode* GM = Cast<AFPSGameMode>(GetWorld()->GetAuthGameMode());
			if (GM)
			{
				if(GetPawn())
				{
					GetPawn()->Destroy();
				}
				GM->PlayerRestart(this);
				bCanRespawn = false;
				bIsSpectating = false;
			}
		}
	}
}

void AFPS_PlayerController::SetCanRespawn_OnServer_Implementation()
{
	bCanRespawn = true;
}

void AFPS_PlayerController::EventAfterJoin_Multicast_Implementation()
{
	if (this->IsLocalPlayerController() && GetGameInstance())
	{
		UMyGameInstance* GI = Cast<UMyGameInstance>(GetGameInstance());
		AFPS_PlayerState* PS = Cast<AFPS_PlayerState>(PlayerState);
		if (GI && PS)
		{
			bool bIsFind = false;
			int32 i = 0;
			while (i < GI->InventoryForPlayers.Num() && !bIsFind)
			{
				if (GI->InventoryForPlayers[i].PlayerName == PS->GetPlayerName())
				{
					bIsFind = true;
					PS->InitInventorySlots_OnServer(GI->InventoryForPlayers[i].WeaponSlots, GI->InventoryForPlayers[i].WeaponTuningSlots);
				}
				i++;
			}
			//SetCanRespawn_OnServer();
			//TryToRepawn_OnServer();
		}
	}
}

void AFPS_PlayerController::IncrementPlayerKills_Multicast_Implementation(int32 NewKills)
{

	IncrementPlayerKills_BP(NewKills);
	
}

void AFPS_PlayerController::IncrementPlayerKills_BP_Implementation(int32 NewKills)
{
}

void AFPS_PlayerController::BeginPlay()
{
	Super::BeginPlay();
}

void AFPS_PlayerController::PlayerDeath_BP_OnServer_Implementation(float RespawnTime)
{
	//BP
}

void AFPS_PlayerController::SwitchSpectatingTarget_OnServer_Implementation(float Value)
{
	if(bIsSpectating)
	{
		if (Value != 0)
		{
			if (!bIsSwitchSpectatingTargetComplete)
			{
				bIsSwitchSpectatingTargetComplete = true;
				if (GetWorld() && GetWorld()->GetAuthGameMode())
				{
					if (AFPSGameMode* GM = Cast<AFPSGameMode>(GetWorld()->GetAuthGameMode()))
					{
						if (SpectatingTarget)
						{
							bool bIsFind = false;
							int32 i = 0;
							while (i < GM->PlayerControllers.Num() && !bIsFind)
							{
								if (GM->PlayerControllers.IsValidIndex(i) && GM->PlayerControllers[i] && GM->PlayerControllers[i]->GetPawn())
								{
									if (SpectatingTarget == GM->PlayerControllers[i]->GetPawn())
									{
										if (Value > 0)
										{
											//bool bIsFind = false;
											int32 j = i + 1;
											while (j < GM->PlayerControllers.Num() && !bIsFind)
											{
												if (GM->PlayerControllers.IsValidIndex(j) && GM->PlayerControllers[j])
												{
													bIsFind = TryGetNewSpectatingTarget(GM->PlayerControllers[j]);
												}
												j++;
											}
											if (!bIsFind)
											{
												SpectatingTarget = nullptr;
												SetSpectatingTarget_BP(nullptr);
												/*j = 0;
												while (j < i && !bIsFind)
												{
													if (GM->PlayerControllers.IsValidIndex(j) && GM->PlayerControllers[j])
													{
														bIsFind = TryGetNewSpectatingTarget(GM->PlayerControllers[j]);
													}
													j++;
												}*/
											}
										}
										else if (Value < 0)
										{
											//bool bIsFind = false;
											int32 j = i - 1;
											while (j >= 0 && !bIsFind)
											{
												if (GM->PlayerControllers.IsValidIndex(j) && GM->PlayerControllers[j])
												{
													bIsFind = TryGetNewSpectatingTarget(GM->PlayerControllers[j]);
												}
												j--;
											}
											if (!bIsFind)
											{
												SpectatingTarget = nullptr;
												SetSpectatingTarget_BP(nullptr);
												/*j = GM->PlayerControllers.Num() - 1;
												while (j > i && !bIsFind)
												{
													if (GM->PlayerControllers.IsValidIndex(j) && GM->PlayerControllers[j])
													{
														bIsFind = TryGetNewSpectatingTarget(GM->PlayerControllers[j]);
													}
													j--;
												}*/
											}

										}
									}
								}
								i++;
							}
						}
						else
						{
							if(Value > 0)
							{
								int32 i = 0;
								bool bIsFind = false;
								while (i < GM->PlayerControllers.Num() && !bIsFind)
								{
									if (GM->PlayerControllers.IsValidIndex(i) && GM->PlayerControllers[i])
									{
										bIsFind = TryGetNewSpectatingTarget(GM->PlayerControllers[i]);
									}
									i++;
								}
							}
							else if (Value < 0)
							{
								int32 i = GM->PlayerControllers.Num() - 1;
								bool bIsFind = false;
								while (i >= 0 && !bIsFind)
								{
									if (GM->PlayerControllers.IsValidIndex(i) && GM->PlayerControllers[i])
									{
										bIsFind = TryGetNewSpectatingTarget(GM->PlayerControllers[i]);
									}
									i--;
								}
							}
						}
					}
				}
			}
		}
		else if (bIsSwitchSpectatingTargetComplete)
		{
			bIsSwitchSpectatingTargetComplete = false;
		}
	}
}

bool AFPS_PlayerController::TryGetNewSpectatingTarget(APlayerController* Controller)
{
	bool bIsSuccess = false;
	if (AFPS_PlayerController* PC = Cast<AFPS_PlayerController>(Controller))
	{
		if (PC->TeamNumber == TeamNumber && PC->GetPawn())
		{
			if (AFirstPersonCharacter* Char = Cast<AFirstPersonCharacter>(PC->GetPawn()))
			{
				if (Char->GetHealthComponent()->GetIsAlive())
				{
					SpectatingTarget = Char;
					SetSpectatingTarget_BP(Char);
					bIsSuccess = true;
					if (Char->bIsAiming && Char->CurrentWeapon)
					{
						Char->SetStateAiming_Multicast(true, Char->CurrentWeapon->WeaponSettings.CharacterAimAnim, Char->CurrentWeapon->WeaponSettings.WeaponLocationInAim, Char->CurrentWeapon->WeaponSettings.SocketNameToAttach);
					}
				}
			}
		}
	}
	return bIsSuccess;
}

void AFPS_PlayerController::UpdatePlayersList_Multicast_Implementation(const TArray<AFPS_PlayerState*> &PlayerStates)
{
	if(IsLocalPlayerController())
	{
		UpdatePlayersList_BP(PlayerStates);
	}
}
void AFPS_PlayerController::UpdatePlayersList_BP_Implementation(const TArray<AFPS_PlayerState*> &Players)
{
	//BP
}

void AFPS_PlayerController::Respawn_Multicast_Implementation(APawn* LastSpectatingTarget)
{
	if (LastSpectatingTarget)
	{
		if (AFirstPersonCharacter* Char = Cast<AFirstPersonCharacter>(LastSpectatingTarget))
		{
			Char->GetMesh()->UnHideBoneByName(FName("Head"));
		}

	}
}
