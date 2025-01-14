// Fill out your copyright notice in the Description page of Project Settings.


#include "LootBox.h"
#include "MyGameInstance.h"
#include "PickUpItem.h"
#include "Kismet/KismetMathLibrary.h"
#include "Net/UnrealNetwork.h"

// Sets default values
ALootBox::ALootBox()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	MainMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MaunMesh"));
	RootComponent = MainMesh;
	ClosedMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("CloseMesh"));
	ClosedMesh->SetupAttachment(RootComponent);
	OpenedMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("OpenedMesh"));
	OpenedMesh->SetupAttachment(RootComponent);
	OpenedMesh->SetVisibility(false);
	LootSpawnLocation = CreateDefaultSubobject<UArrowComponent>(TEXT("LootSpawnLocation"));
	LootSpawnLocation->SetupAttachment(RootComponent);
}

// Called when the game starts or when spawned
void ALootBox::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ALootBox::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void ALootBox::OpenLootBox()
{
	if(bCanOpenLootBox)
	{
		OpenOrCloseLootBox_Multicast(true);
		if (GetWorld())
		{
			FActorSpawnParameters SpawnParams;
			SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
			APickUpWeapon* DropWeapon = Cast<APickUpWeapon>(GetWorld()->SpawnActor<APickUpWeapon>(PickUpWeaponClass, LootSpawnLocation->GetComponentLocation(), LootSpawnLocation->GetComponentRotation(), SpawnParams));
			if (GetGameInstance())
			{
				UMyGameInstance* GI = Cast<UMyGameInstance>(GetGameInstance());
				if (DropWeapon && GI && GI->WeaponSettingsTaple)
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
							float TuningChance = UKismetMathLibrary::RandomFloat();	
							FWeaponTuningSlot TuningSlot;
							if(TuningChance <= 30.f)
							{		
								TuningSlot.WeaponName = WeaponNames[RandomWeaponIndex];
								int32 SumTuningTypes = static_cast<int32>(ETuningTypes::Sight) + 1;
								TArray<FName> AllItems = GI->WeaponTuningInfoTaple->GetRowNames();
								for (int32 i = 0; i < SumTuningTypes; i++)
								{
									ETuningTypes TuningType = static_cast<ETuningTypes>(i);
									TArray<FName> SuitableItems;
									for (int32 j = 0; j < AllItems.Num(); j++)
									{
										FWeaponTuningInfo NewInfo;
										if (GI->GetWeaponTuningInfoByName(AllItems[j], NewInfo))
										{
											if (NewInfo.TuningType == TuningType)
											{
												if (NewInfo.SuitableWeapons.Contains(WeaponNames[RandomWeaponIndex]))
												{
													SuitableItems.Add(AllItems[j]);
												}

											}
										}
									}
									int32 RandomItemIndex = UKismetMathLibrary::RandomInteger(SuitableItems.Num());
									if (SuitableItems.IsValidIndex(RandomItemIndex))
									{
										TuningSlot.TuningNames.Add(SuitableItems[RandomItemIndex]);
									}
								}
							}
							DropWeapon->InitItem(WeaponSlot, TuningSlot);			
						}
					}
				}
			}
			APickUpAmmo* DropAmmo = Cast<APickUpAmmo>(GetWorld()->SpawnActor<APickUpAmmo>(PickUpAmmoClass, LootSpawnLocation->GetComponentLocation(), LootSpawnLocation->GetComponentRotation(), SpawnParams));
			if (DropAmmo)
			{
				int32 SumAmmoTypes = static_cast<int32>(EAmmoTypes::SniperType) + 1;
				int32 RandomAmmoTypeIndex = UKismetMathLibrary::RandomInteger(SumAmmoTypes);
				EAmmoTypes AmmoType = static_cast<EAmmoTypes>(RandomAmmoTypeIndex);
				FAmmoSlot AmmoSlot;
				AmmoSlot.AmmoType = AmmoType;
				AmmoSlot.ClipAmount = 2;
				AmmoSlot.MaxClipAmount = 2;
				DropAmmo->InitItem(AmmoSlot);
			}
			bCanOpenLootBox = false;
			GetWorld()->GetTimerManager().SetTimer(ReloadTimer, this, &ALootBox::EndReload, ReloadTime, false);
		}
	}

	
}

void ALootBox::EndReload()
{
	bCanOpenLootBox = true;
	OpenOrCloseLootBox_Multicast(false);
}

void ALootBox::OpenOrCloseLootBox_Multicast_Implementation(bool bIsOpen)
{	
	if (OpenedMesh && ClosedMesh)
	{
		if(bIsOpen)
		{
			OpenedMesh->SetVisibility(true);
			ClosedMesh->SetVisibility(false);
		}
		else
		{
			OpenedMesh->SetVisibility(false);
			ClosedMesh->SetVisibility(true);
		}
	}
}
