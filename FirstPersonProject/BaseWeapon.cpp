// Fill out your copyright notice in the Description page of Project Settings.


#include "BaseWeapon.h"
#include "MyFunctionLibrary.h"
#include "BaseProjectile.h"
#include "Kismet/KismetMathLibrary.h"
#include "Particles/ParticleEmitter.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"
#include "Components/SceneComponent.h"
#include "Kismet/KismetSystemLibrary.h"
#include "FPS_PlayerController.h"
#include "FirstPersonCharacter.h"

// Sets default values
ABaseWeapon::ABaseWeapon()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	SceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("SceneComponent"));
	RootComponent = SceneComponent;
	WeaponSkeletalMeshComponent = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("SkeletalMesh"));
	WeaponSkeletalMeshComponent->SetCollisionProfileName(FName("OverlapAll"));
	WeaponSkeletalMeshComponent->SetupAttachment(RootComponent);
	WeaponStaticMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StaticMesh"));
	WeaponStaticMeshComponent->SetCollisionProfileName(FName("OverlapAll"));
	WeaponStaticMeshComponent->SetupAttachment(RootComponent);
	SightMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Sight"));
	SightMeshComponent->SetCollisionProfileName(FName("OverlapAll"));
	SightMeshComponent->SetupAttachment(RootComponent);

	SetReplicates(true);
}

void ABaseWeapon::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ABaseWeapon, WeaponStaticMesh);
	DOREPLIFETIME(ABaseWeapon, WeaponSkeletalMesh);
	DOREPLIFETIME(ABaseWeapon, ShootEndLocation);
	DOREPLIFETIME(ABaseWeapon, ShootLocation);
	DOREPLIFETIME(ABaseWeapon, AimCameraLocation);
	DOREPLIFETIME(ABaseWeapon, bIsWeaponReloading);
	DOREPLIFETIME(ABaseWeapon, SightMesh);
}

float ABaseWeapon::GetCurrentDispersion()
{
	return CurrentDispersion;
}

void ABaseWeapon::Reload()
{
	AmmoAmount = WeaponSettings.MaxAmmoAmount;
	bIsWeaponReloading = false;
	OnWeaponReloadEnd.Broadcast(WeaponSettings.AmmoType, -1);
}

void ABaseWeapon::WeaponReloadStart()
{
	if (!bIsWeaponReloading && !bIsAmmoSlotEmpty)
	{
		bIsWeaponReloading = true;	
		if(GetWorld())
		{
			OnWeaponReloadStart.Broadcast();
			GetWorld()->GetTimerManager().SetTimer(ReloadTimer, this, &ABaseWeapon::Reload, WeaponSettings.ReloadTime, false);
		}
	}
}

void ABaseWeapon::InitSightMesh_Multicast_Implementation(UStaticMesh* NewMesh, FTransform SightTransform)
{
	if (SightMeshComponent)
	{
		SightMeshComponent->SetRelativeTransform(SightTransform);
		SightMeshComponent->SetStaticMesh(NewMesh);
	}
}

void ABaseWeapon::InitMesh_Multicast_Implementation(USkeletalMesh* SkeletalMesh, UStaticMesh* StaticMesh)
{
	if (SkeletalMesh)
	{
		WeaponSkeletalMeshComponent->SetSkeletalMesh(SkeletalMesh);
		//WeaponSkeletalMesh = SkeletalMesh;
	}
	else
	{
		WeaponSkeletalMeshComponent->DestroyComponent();
	}
	if (StaticMesh)
	{
		WeaponStaticMeshComponent->SetStaticMesh(StaticMesh);
		//WeaponStaticMesh = StaticMesh;
	}
	else
	{
		WeaponStaticMeshComponent->DestroyComponent();
	}
}

void ABaseWeapon::SpawnFireFX_Multicast_Implementation(UParticleSystem* FX, FVector Location)
{
	UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), FX, Location);
}

void ABaseWeapon::SpawnFireSound_Multicast_Implementation(USoundBase* Sound, FVector Location)
{
	UGameplayStatics::PlaySoundAtLocation(GetWorld(), Sound, Location);
}

// Called when the game starts or when spawned
void ABaseWeapon::BeginPlay()
{
	Super::BeginPlay();
	
	if (GetLocalRole() == ROLE_SimulatedProxy)
	{
		if(WeaponSkeletalMesh || WeaponStaticMesh)
		{
			if(WeaponSkeletalMeshComponent && WeaponStaticMeshComponent)
			{
				if (WeaponSkeletalMeshComponent->SkeletalMesh == nullptr && WeaponStaticMeshComponent->GetStaticMesh() == nullptr)
				{
					if (WeaponSkeletalMesh)
					{
						WeaponSkeletalMeshComponent->SetSkeletalMesh(WeaponSkeletalMesh);
						WeaponStaticMeshComponent->DestroyComponent();
					}
					else if (WeaponStaticMesh)
					{
						WeaponStaticMeshComponent->SetStaticMesh(WeaponStaticMesh);
						WeaponSkeletalMeshComponent->DestroyComponent();
					}
				}
			}
		}
		if (SightMesh && SightMeshComponent->GetStaticMesh() == nullptr)
		{
			SightMeshComponent->SetStaticMesh(SightMesh);
		}
	}
}

// Called every frame
void ABaseWeapon::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	FireTick(DeltaTime);
	DispersionTick(DeltaTime);
}

void ABaseWeapon::SetWeaponStateFire(bool bIsFire)
{
	bIsFireButtonPressed = bIsFire;
}

void ABaseWeapon::InitWeapon(FWeaponSettings NewWeaponSettings)
{
	WeaponSettings = NewWeaponSettings;
	ShootLocation = NewWeaponSettings.ShootLocation;
	if (AmmoAmount <= 0)
	{
		WeaponReloadStart();
	}
	CoefDispersionWhileAiming = 1.f;
	CurrentDispersion = NewWeaponSettings.MinStandDispersion;

	//SightMesh->SetRelativeTransform(NewWeaponSettings.SightLocation);
	WeaponStaticMesh = WeaponSettings.WeaponStaticMesh;
	WeaponSkeletalMesh = WeaponSettings.WeaponSkeletalMesh;
	InitMesh_Multicast(WeaponSettings.WeaponSkeletalMesh, WeaponSettings.WeaponStaticMesh);
}

void ABaseWeapon::Fire()
{
	//OnServer
	if(!bIsWeaponReloading)
	{	
		FVector RelativeLocation = GetActorForwardVector() * WeaponSettings.ShootLocation.GetLocation().X + GetActorRightVector() * WeaponSettings.ShootLocation.GetLocation().Y + GetActorUpVector() * WeaponSettings.ShootLocation.GetLocation().Z;
		FTransform SpawnTransform;
		SpawnTransform.SetLocation(GetActorLocation() + RelativeLocation);
		SpawnTransform.SetScale3D(WeaponSettings.ShootLocation.GetScale3D());
		for (int i = 0; i < WeaponSettings.NumberProjectilesByShot; i++)
		{
			FRotator SpawnRotation = UKismetMathLibrary::FindLookAtRotation(GetActorLocation() + RelativeLocation, ShootEndLocation);
			FVector DispersionDirection = FMath::VRandCone(SpawnRotation.Vector(), GetCurrentDispersion());
			SpawnTransform.SetRotation(DispersionDirection.Rotation().Quaternion());
			if (WeaponSettings.ProjectileSettings.ProjectileClass)
			{
				FActorSpawnParameters SpawnParams;
				SpawnParams.Instigator = GetInstigator();
				SpawnParams.Owner = GetOwner();
				SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

				//DrawDebugCone(GetWorld(), SpawnTransform.GetLocation(), SpawnRotation.Vector(), 10000, GetCurrentDispersion(), GetCurrentDispersion(), 100, FColor::Red, false, 1);


				ABaseProjectile* Projectile = GetWorld()->SpawnActor<ABaseProjectile>(WeaponSettings.ProjectileSettings.ProjectileClass, SpawnTransform, SpawnParams);
				if (Projectile)
				{
					//GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, FString::Printf(TEXT("%f")));
					Projectile->InitProjectile(WeaponSettings.ProjectileSettings);
				}

				//DrawDebugSphere(GetWorld(), SpawnTransform.GetLocation(), 10, 20, FColor::Red);
			}
			else
			{
				TArray<AActor*> IgnoreActors;
				FHitResult Hit;
				UKismetSystemLibrary::LineTraceSingle(GetWorld(), SpawnTransform.GetLocation(), SpawnTransform.GetLocation() + SpawnTransform.GetRotation().Vector() * 10000.f, ETraceTypeQuery::TraceTypeQuery1, false, IgnoreActors, EDrawDebugTrace::ForDuration, Hit, true, FColor::Red, FColor::Green, 1.f);
				if (Hit.bBlockingHit)
				{
					if (Hit.GetActor() && Hit.GetComponent() && Hit.GetComponent()->GetMaterial(0) && Hit.GetComponent()->GetMaterial(0)->GetPhysicalMaterial() && Hit.GetComponent()->GetMaterial(0)->GetPhysicalMaterial()->SurfaceType)
					{
						if(AFirstPersonCharacter* Char = Cast<AFirstPersonCharacter>(GetOwner()))
						{
							EPhysicalSurface Surface = Hit.Component->GetMaterial(0)->GetPhysicalMaterial()->SurfaceType;
							if (WeaponSettings.ProjectileSettings.HitSounds.Contains(Surface))
							{
								USoundBase* Sound = WeaponSettings.ProjectileSettings.HitSounds[Surface];
								if (Sound)
								{
									Char->SpawnSound_Multicast(Sound, Hit.Location);

								}
							}
							if (WeaponSettings.ProjectileSettings.HitDecals.Contains(Surface))
							{
								UMaterialInterface* Material = WeaponSettings.ProjectileSettings.HitDecals[Surface];
								if (Material && Hit.GetComponent())
								{
									Char->SpawnDecal_Multicast(Material, Hit.GetComponent(), Hit);
								}
							}
							if (WeaponSettings.ProjectileSettings.HitFXs.Contains(Surface))
							{
								UParticleSystem* FX = WeaponSettings.ProjectileSettings.HitFXs[Surface];
								if (FX)
								{
									Char->SpawnFX_Multicast(FX, Hit.Location, Hit.ImpactNormal.Rotation());
									//UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), FX, FTransform(Hit.ImpactNormal.Rotation(), Hit.ImpactPoint, FVector(0.5f)));
								}
							}
						}
					}
					if (Hit.GetActor())
					{
						ACharacter* Char = Cast<ACharacter>(Hit.GetActor());
						if (Char && Char->GetController())
						{
							AFPS_PlayerController* TargetPC = Cast<AFPS_PlayerController>(Char->GetController());
							AFPS_PlayerController* SelfPC = Cast<AFPS_PlayerController>(GetInstigator()->GetController());
							if (TargetPC && SelfPC && TargetPC->TeamNumber != SelfPC->TeamNumber)
							{
								if (GetInstigator() && GetInstigator()->GetController())
								{
									UGameplayStatics::ApplyDamage(Hit.GetActor(), WeaponSettings.ProjectileSettings.Damage, GetInstigator()->GetController(), this, NULL);
								}
								else
								{
									UGameplayStatics::ApplyDamage(Hit.GetActor(), WeaponSettings.ProjectileSettings.Damage, nullptr, this, NULL);
								}
							}
						}
						else 
						{
							if (GetInstigator() && GetInstigator()->GetController())
							{
								UGameplayStatics::ApplyDamage(Hit.GetActor(), WeaponSettings.ProjectileSettings.Damage, GetInstigator()->GetController(), this, NULL);
							}	
							else
							{
								UGameplayStatics::ApplyDamage(Hit.GetActor(), WeaponSettings.ProjectileSettings.Damage, nullptr, this, NULL);
							}
						}

					}
				}
			}
		}

		if (GetCurrentDispersion() + WeaponSettings.ShootDispersion <= MaxDispersion)
		{
			CurrentDispersion += WeaponSettings.ShootDispersion;
		}
		else
		{
			CurrentDispersion = MaxDispersion;
		}
		SpawnFireFX_Multicast(WeaponSettings.FireFX, SpawnTransform.GetLocation());
		SpawnFireSound_Multicast(WeaponSettings.FireSound, SpawnTransform.GetLocation());
		AmmoAmount--;

		if (AmmoAmount <= 0)
		{
			WeaponReloadStart();
		}

		OnWeaponFire.Broadcast();
	}
}

void ABaseWeapon::FireTick(float DeltaTime)
{
	if (bIsFireButtonPressed && AmmoAmount > 0)
	{
		if (FireTimer <= 0)
		{
			Fire();
			FireTimer = WeaponSettings.FireRate;
		}
	}
	if (FireTimer > 0)
	{
		FireTimer -= DeltaTime;
	}
}

void ABaseWeapon::DispersionTick(float DeltaTime)
{
	if (bShouldReduceDispersion)
	{
		MaxDispersion = WeaponSettings.MaxStandDispersion* CoefDispersionWhileAiming;
		//MinDispersion = WeaponSettings.MinDispersion;
		//if(!bIsFireButtonPressed)
		if(GetCurrentDispersion() - WeaponSettings.DispersionReductionWhileStanding * DeltaTime >= WeaponSettings.MinStandDispersion * CoefDispersionWhileAiming)
		{
			CurrentDispersion -= WeaponSettings.DispersionReductionWhileStanding * DeltaTime + WeaponSettings.DispersionReductionWhileStanding * DeltaTime* CoefDispersionWhileAiming;
		}
		else
		{
			CurrentDispersion = WeaponSettings.MinStandDispersion* CoefDispersionWhileAiming;
		}
	}
	else
	{
		
		MaxDispersion = WeaponSettings.MaxMoveDispersion * CoefDispersionWhileAiming;
		//MinDispersion = WeaponSettings.MinMoveDispersion;
		
		//if(!bIsFireButtonPressed)
		{
			if (GetCurrentDispersion() + WeaponSettings.IncreaseDispersionInMove * DeltaTime <= WeaponSettings.MoveDispersion * CoefDispersionWhileAiming)
			{
				CurrentDispersion += WeaponSettings.IncreaseDispersionInMove * DeltaTime * CoefDispersionWhileAiming;
			}
			else
			{
				CurrentDispersion -= WeaponSettings.DispersionReductionWhileMoving * DeltaTime + WeaponSettings.DispersionReductionWhileMoving * DeltaTime * CoefDispersionWhileAiming;
			}

		}
	}
}

void ABaseWeapon::SpawnHitSound_Multicast_Implementation(USoundBase* Sound, FHitResult Hit)
{
	UGameplayStatics::PlaySoundAtLocation(GetWorld(), Sound, Hit.ImpactPoint);
}

void ABaseWeapon::SpawnHitDecal_Multicast_Implementation(UMaterialInterface* DecalMaterial, UPrimitiveComponent* OtherComp, FHitResult Hit)
{
	UGameplayStatics::SpawnDecalAttached(DecalMaterial, FVector(10), OtherComp, NAME_None, Hit.ImpactPoint, Hit.ImpactNormal.Rotation(), EAttachLocation::KeepWorldPosition, 10.f);
}

void ABaseWeapon::SpawnHitFX_Multicast_Implementation(UParticleSystem* FX, FHitResult Hit)
{
	UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), FX, FTransform(Hit.ImpactNormal.Rotation(), Hit.ImpactPoint, FVector(0.5f)));
}