// Fill out your copyright notice in the Description page of Project Settings.


#include "FPSGameMode.h"
#include "GameFramework/PlayerStart.h"
#include "Kismet/KismetStringLibrary.h"
#include "FPS_PlayerController.h"
#include "MyGameInstance.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "FirstPersonCharacter.h"
#include "FPS_PlayerState.h"
#include "HealthComponent.h"
#include "Engine/TargetPoint.h"
#include "PickUpItem.h"

void AFPSGameMode::BeginPlay()
{
	if (GetGameInstance())
	{
		UMyGameInstance* GI = Cast<UMyGameInstance>(GetGameInstance());
		if (GI)
		{
			GameMode = GI->GameMode;
			GI->UpdateSession_OnServer();

			//This Code Spawns Pick Up Items On Map
			TArray<AActor*> Actors;
			UGameplayStatics::GetAllActorsOfClass(GetWorld(), ATargetPoint::StaticClass(), Actors);
			for (int32 i = 0; i < Actors.Num(); i++)
			{
				ATargetPoint* TP = Cast<ATargetPoint>(Actors[i]);
				if (TP && TP->Tags[0] == FName("Item"))
				{
					float RandomItem = UKismetMathLibrary::RandomFloat();
					if (RandomItem < SpawnWeaponChance)
					{
						FActorSpawnParameters SpawnParams;
						SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
						APickUpWeapon* DropWeapon = Cast<APickUpWeapon>(GetWorld()->SpawnActor<APickUpWeapon>(PickUpWeaponClass, TP->GetActorLocation(), TP->GetActorRotation(), SpawnParams));
						if (DropWeapon && GI->WeaponSettingsTaple)
						{
							TArray<FName> WeaponNames = GI->WeaponSettingsTaple->GetRowNames();
							int32 RandomWeaponIndex = UKismetMathLibrary::RandomInteger(WeaponNames.Num());
							FWeaponSettings WeaponSettings;
							if (WeaponNames.IsValidIndex(RandomWeaponIndex))
							{
								if (GI->GetWeaponSettingsByName(WeaponNames[RandomWeaponIndex], WeaponSettings))
								{
									FWeaponSlot WeaponSlot;
									WeaponSlot.WeaponName = WeaponNames[RandomWeaponIndex];
									WeaponSlot.AmmoAmount = WeaponSettings.MaxAmmoAmount;
									FWeaponTuningSlot TuningSlot;
									TuningSlot.WeaponName = WeaponNames[RandomWeaponIndex];
									int32 SumTuningTypes = static_cast<int32>(ETuningTypes::Sight);
									TArray<FName> AllItems = GI->WeaponTuningInfoTaple->GetRowNames();
									for (int32 j = 0; j < SumTuningTypes; j++)
									{
										ETuningTypes TuningType = static_cast<ETuningTypes>(j);
										TArray<FName> SuitableItems;
										for (int32 k = 0; k < AllItems.Num(); k++)
										{
											FWeaponTuningInfo NewInfo;
											if (GI->GetWeaponTuningInfoByName(AllItems[k], NewInfo))
											{
												if (NewInfo.TuningType == TuningType)
												{
													SuitableItems.Add(AllItems[k]);
												}
											}
										}
										int32 RandomItemIndex = UKismetMathLibrary::RandomInteger(SuitableItems.Num());
										if (SuitableItems.IsValidIndex(RandomItemIndex))
										{
											TuningSlot.TuningNames.Add(SuitableItems[RandomItemIndex]);
										}
									}
									DropWeapon->InitItem(WeaponSlot, TuningSlot);
								}
							}

						}
					}
					else if (RandomItem < SpawnWeaponChance + SpawnAmmoChance)
					{
						FActorSpawnParameters SpawnParams;
						SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
						APickUpAmmo* DropAmmo = Cast<APickUpAmmo>(GetWorld()->SpawnActor<APickUpAmmo>(PickUpAmmoClass, TP->GetActorLocation(), TP->GetActorRotation(), SpawnParams));
						if (DropAmmo)
						{
							int32 SumAmmoTypes = static_cast<int32>(EAmmoTypes::SniperType);
							int32 RandomAmmoTypeIndex = UKismetMathLibrary::RandomInteger(SumAmmoTypes);
							EAmmoTypes AmmoType = static_cast<EAmmoTypes>(RandomAmmoTypeIndex);
							FAmmoSlot AmmoSlot;
							AmmoSlot.AmmoType = AmmoType;
							AmmoSlot.ClipAmount = 2;
							AmmoSlot.MaxClipAmount = 2;
							DropAmmo->InitItem(AmmoSlot);
						}
					}
				}
			}
		}
	}
}

void AFPSGameMode::HandleStartingNewPlayer_Implementation(APlayerController* NewPlayer)
{
	Super::HandleStartingNewPlayer_Implementation(NewPlayer);

	if(NewPlayer)
	{
		PlayerControllers.Add(NewPlayer);

		AFPS_PlayerController* Controller = Cast<AFPS_PlayerController>(NewPlayer);
		if (Controller)
		{
			int32 NewTeamNumber = 0;
			for (int32 i = 0; i < SumPlayersInTeams.Num(); i++)
			{
				if(SumPlayersInTeams[i] < SumPlayersInTeams[NewTeamNumber])
				{
					NewTeamNumber = i;
				}		
			}	
			if(SumPlayersInTeams.IsValidIndex(NewTeamNumber))
			{
				SumPlayersInTeams[NewTeamNumber]++;
			}
			Controller->TeamNumber = NewTeamNumber;
			
			//Controller->UnPossess();


			if (GetGameInstance() && Controller->PlayerState)
			{
				UMyGameInstance* GI = Cast<UMyGameInstance>(GetGameInstance());
				AFPS_PlayerState* PS = Cast<AFPS_PlayerState>(Controller->PlayerState);
				PS->TeamNumber = NewTeamNumber;
				if (GI && PS)
				{
					bool bIsFind = false;
					int32 i = 0;
					while (i < GI->InventoryForPlayers.Num() && !bIsFind)
					{
						if (GI->InventoryForPlayers[i].PlayerName == PS->GetPlayerName())
						{
							bIsFind = true;
							PS->WeaponSlots = GI->InventoryForPlayers[i].WeaponSlots;
							PS->TuningWeaponSlots = GI->InventoryForPlayers[i].WeaponTuningSlots;
							//PlayerRestart(Controller);
							//Controller->bCanRespawn = false;
							//Controller->SetCanRespawn_OnServer();
							//Char->InitInventory_OnServer(GI->InventoryForPlayers[i].WeaponSlots);
						}
						i++;
					}
					if (!bIsFind)
					{
						Controller->EventAfterJoin_Multicast();
					}	
					SpawnSpectatorPawn(Controller, nullptr);
				}
			}
		}
		UpdatePlayersListForPlayers();
	}
}

void AFPSGameMode::PlayerRestart(AFPS_PlayerController* Controller)
{
	if(Controller)
	{
		TArray<AActor*> PlayerStarts;
		UGameplayStatics::GetAllActorsOfClass(GetWorld(), APlayerStart::StaticClass(), PlayerStarts);
		FString StringNumber = UKismetStringLibrary::Conv_IntToString(Controller->TeamNumber);
		FName NameNumber = UKismetStringLibrary::Conv_StringToName(StringNumber);
		for (int32 i = 0; i < PlayerStarts.Num(); i++)
		{
			if (PlayerStarts[i])
			{
				APlayerStart* PlayerStart = Cast<APlayerStart>(PlayerStarts[i]);
				if (PlayerStart)
				{
					FName Tag = PlayerStart->PlayerStartTag;
					if (NameNumber == Tag)
					{
						//Controller->UnPossess();
						//RestartPlayerAtPlayerStart(Controller, PlayerStart);
						if(GetWorld() && CharacterClass && Controller->PlayerState)
						{
							FActorSpawnParameters SpawnParams;
							SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
							AFirstPersonCharacter* Char = Cast<AFirstPersonCharacter>(GetWorld()->SpawnActor<AFirstPersonCharacter>(CharacterClass, PlayerStart->GetActorLocation(), PlayerStart->GetActorRotation(), SpawnParams));
							AFPS_PlayerState* PS = Cast<AFPS_PlayerState>(Controller->PlayerState);
							if (Char && PS)
							{
								Controller->Respawn_Multicast(Controller->SpectatingTarget);
								Controller->SpectatingTarget = nullptr;
								Controller->Possess(Char);
								Char->InitInventory_OnServer(PS->WeaponSlots, PS->TuningWeaponSlots);
								//Char->InitWeapon(PS->WeaponSlots[0].WeaponName, WeaponSettings.MaxAmmoAmount);
							}
							
						}
					}
				}
			}
		}
	}
}

void AFPSGameMode::PlayerDeath(AFPS_PlayerController* Controller)
{
	if(Controller)
	{
		//IncrementTeamKills(Controller->TeamNumber);
		if (GetWorld())
		{			
			//FTimerHandle PlayerRestartTimer;
			//PlayerRestartTimers.Add(PlayerRestartTimer);
			//GetWorld()->GetTimerManager().SetTimer(PlayerRestartTimers[PlayerRestartTimers.Num() - 1], this, &AFPSGameMode::RestartPlayerByTimer, GameMode.PlayerRestartTime, false);
			//PlayersWaitingRestart.Add(Controller);
			//int32 Random = UKismetMathLibrary::RandomInteger(PlayerControllers.Num() - 1);
			int i = 0;
			bool bIsFind = false;
			APawn* NewSpectatingPawn = nullptr;
			while (i < PlayerControllers.Num() && !bIsFind)
			{
				if (PlayerControllers.IsValidIndex(i) && PlayerControllers[i])
				{
					AFPS_PlayerController* PC = Cast<AFPS_PlayerController>(PlayerControllers[i]);
					if (PC && PC->TeamNumber == Controller->TeamNumber)
					{
						if(PC->GetPawn())
						{
							AFirstPersonCharacter* Char = Cast<AFirstPersonCharacter>(PlayerControllers[i]->GetPawn());
							if (Char && Char->GetHealthComponent() && Char->GetHealthComponent()->GetIsAlive())
							{
								bIsFind = true;
								NewSpectatingPawn = Char;

							}
						}
					}
				}
				i++;
			}		

			for (int32 j = 0; j < PlayerControllers.Num(); j++)
			{
				if (PlayerControllers.IsValidIndex(j) && PlayerControllers[j])
				{
					AFPS_PlayerController* PC = Cast<AFPS_PlayerController>(PlayerControllers[j]);
					if (PC && PC->SpectatingTarget && PC->SpectatingTarget == Controller->SpectatingTarget)
					{
						PC->SpectatingTarget = NewSpectatingPawn;
						PC->SetSpectatingTarget_BP(NewSpectatingPawn);
					}
				}
			}
			SpawnSpectatorPawn(Controller, NewSpectatingPawn);
			Controller->SpectatingTarget = NewSpectatingPawn;
			Controller->PlayerDeath(GameMode.PlayerRestartTime);
		}
	}
}

void AFPSGameMode::UpdatePlayersListForPlayers()
{
	TArray<AFPS_PlayerState*> PSArray;
	for (int32 i = 0; i < PlayerControllers.Num(); i++)
	{
		if (PlayerControllers.IsValidIndex(i) && PlayerControllers[i] && PlayerControllers[i]->PlayerState)
		{
			if (AFPS_PlayerState* PS = Cast<AFPS_PlayerState>(PlayerControllers[i]->PlayerState))
			{
				PSArray.Add(PS);
			}
		}
	}
	for (int32 i = 0; i < PlayerControllers.Num(); i++)
	{
		if (PlayerControllers.IsValidIndex(i) && PlayerControllers[i])
		{
			if (AFPS_PlayerController* PC = Cast<AFPS_PlayerController>(PlayerControllers[i]))
			{
				PC->UpdatePlayersList_Multicast(PSArray);
			}
		}
	}
}

//void AFPSGameMode::RestartPlayerByTimer()
//{
//	if(PlayersWaitingRestart[0])
//	{
//	//	PlayersWaitingRestart[0]->UnPossess();
//		PlayerRestartTimers.RemoveAt(0);
//		PlayerRestart(PlayersWaitingRestart[0]);
//		PlayersWaitingRestart.RemoveAt(0);
//	}
//}

void AFPSGameMode::SpawnSpectatorPawn_Implementation(APlayerController* Controller, APawn* SpectatingTargetPawn)
{
	//BP
}

void AFPSGameMode::IncrementTeamKills(int32 TeamNumber, AFPS_PlayerController* Killer)
{
	//On Server
	if(GameMode.bCanKillsAffectPoints)
	{
		if (SumTeamKills.IsValidIndex(TeamNumber))
		{
			SumTeamKills[TeamNumber]++;
			bool bIsEndGame = false;
			if (SumTeamKills[TeamNumber] >= GameMode.SumPointsToWin)
			{
				if (UMyGameInstance* GI = Cast<UMyGameInstance>(GetGameInstance()))
				{
					GI->SetIsSessionOpend(false);
				}
				bIsEndGame = true;
				UGameplayStatics::SetGamePaused(GetWorld(), true);
			}
		}
		for (int32 i = 0; i < PlayerControllers.Num(); i++)
		{
			if (PlayerControllers[i])
			{
				AFPS_PlayerController* PC = Cast<AFPS_PlayerController>(PlayerControllers[i]);
				if (PC)
				{
					PC->UpdateTeamKills_Multicast(SumTeamKills, GameMode.SumPointsToWin);
				}
			}
		}
		if(Killer)
		{
			if (AFPS_PlayerState* KillerPS = Cast<AFPS_PlayerState>(Killer->PlayerState))
			{
				int32 i = 0; 
				bool bIsFind = false;
				while (!bIsFind && i < PlayerControllers.Num())
				{
					if (PlayerControllers[i] == Killer)
					{
						bIsFind = true;
						for (int32 j = i - 1; j > 0; j--)
						{
							if (AFPS_PlayerController* PC = Cast<AFPS_PlayerController>(PlayerControllers[j]))
							{
								if (PC->TeamNumber == Killer->TeamNumber)
								{						
									if (AFPS_PlayerState* PS = Cast<AFPS_PlayerState>(PC->PlayerState))
									{
										if (KillerPS->Kills > PS->Kills)
										{
											APlayerController* LastPC = PlayerControllers[j];
											PlayerControllers[j] = Killer;
											PlayerControllers[i] = LastPC;
										}
									}
			
								}
							}
						}
					}
					i++;
				}			
				UpdatePlayersListForPlayers();
			}
		}
	}
}
