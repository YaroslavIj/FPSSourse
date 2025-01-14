// Fill out your copyright notice in the Description page of Project Settings.

#include "PickUpItem.h"
#include "Net/UnrealNetwork.h"
#include "MyGameInstance.h"

// Sets default values

APickUpItem::APickUpItem()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	//GetStaticMeshComponent()->SetSimulatePhysics(true);
	////GetStaticMeshComponent()->SetMassOverrideInKg(FName(), 100.f, true);
	//bStaticMeshReplicateMovement = true;
	//GetStaticMeshComponent()->SetMobility(EComponentMobility::Movable);

}

// Called when the game starts or when spawned
void APickUpItem::BeginPlay()
{
	Super::BeginPlay();

}

// Called every frame
void APickUpItem::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void APickUpItem::InitMesh_Multicast_Implementation(UStaticMesh* NewMesh)
{
	if(NewMesh && GetStaticMeshComponent())
	{
		GetStaticMeshComponent()->SetStaticMesh(NewMesh);
	}
}

APickUpWeapon::APickUpWeapon()
{	
	PrimaryActorTick.bCanEverTick = true;
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.

	SightMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("SightMesh"));
	SightMesh->SetupAttachment(RootComponent);
}

// Called when the game starts or when spawned
void APickUpWeapon::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void APickUpWeapon::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void APickUpWeapon::InitItem(FWeaponSlot NewWeaponSlot, FWeaponTuningSlot NewTuningSlot)
{
	WeaponSlot = NewWeaponSlot;
	TuningSlot = NewTuningSlot;
	if (GetGameInstance())
	{
		if(UMyGameInstance* GI = Cast<UMyGameInstance>(GetGameInstance()))
		{		
			FWeaponSettings WeaponSettings;
			if (GI->GetWeaponSettingsByName(NewWeaponSlot.WeaponName, WeaponSettings))
			{
				if (WeaponSettings.WeaponStaticMesh)
				{
					InitMesh_Multicast(WeaponSettings.WeaponStaticMesh);
				}	
				FWeaponTuningInfo TuningInfo;
				for(int32 i = 0; i < NewTuningSlot.TuningNames.Num(); i++)
				{
					if (GI->GetWeaponTuningInfoByName(NewTuningSlot.TuningNames[i], TuningInfo))
					{
						if (TuningInfo.StaticMesh)
						{
							FTransform TuningTransform;
							switch (TuningInfo.TuningType)
							{
							case ETuningTypes::Sight: 
								TuningTransform = WeaponSettings.SightLocation;
								break;
							default:
								break;
							}
							InitTuningMesh_Multicast(TuningInfo.StaticMesh, TuningTransform);
						}
					}
				}
			}

	
		}
	}
}

void APickUpWeapon::InitTuningMesh_Multicast_Implementation(UStaticMesh* NewMesh, FTransform NewTransform)
{
	if (NewMesh && SightMesh)
	{
		SightMesh->SetStaticMesh(NewMesh);
		SightMesh->SetRelativeTransform(NewTransform);
	}
}

APickUpAmmo::APickUpAmmo()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void APickUpAmmo::BeginPlay()
{
	Super::BeginPlay();

}

// Called every frame
void APickUpAmmo::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void APickUpAmmo::InitItem(FAmmoSlot NewAmmoSlot)
{
	AmmoSlot = NewAmmoSlot;
	//if (StaticMeshesByAmmoTypes.Contains(NewAmmoSlot.AmmoType) && StaticMeshesByAmmoTypes[NewAmmoSlot.AmmoType])
	//{
	//	InitMesh_Multicast(StaticMeshesByAmmoTypes[NewAmmoSlot.AmmoType]);
	//}
}
