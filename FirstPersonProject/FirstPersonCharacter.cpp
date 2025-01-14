// Fill out your copyright notice in the Description page of Project Settings.


#include "FirstPersonCharacter.h"
#include "MyGameInstance.h"
#include "MyFunctionLibrary.h"
#include "BaseWeapon.h"
#include "Net/UnrealNetwork.h"
#include "Camera/CameraComponent.h"
#include "Components/SceneComponent.h"
#include "DrawDebugHelpers.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Kismet/KismetMathLibrary.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Engine/EngineTypes.h"
#include "HealthComponent.h"
#include "Components/PrimitiveComponent.h"
#include "FPS_PlayerController.h"
#include "FPS_PlayerState.h"
#include "FPSGameMode.h"
#include "Kismet/GameplayStatics.h"
#include "PickUpItem.h"
#include "Components/CapsuleComponent.h"
#include "LootBox.h"

// Sets default values
AFirstPersonCharacter::AFirstPersonCharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	GetMesh()->SetupAttachment(RootComponent);
	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	Camera->SetupAttachment(RootComponent);
	AimCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("AimCamera"));
	AimCamera->SetupAttachment(RootComponent);
	HealthComponent = CreateDefaultSubobject<UHealthComponent>(TEXT("HealthComponent"));
	OnTakeAnyDamage.AddDynamic(this, &AFirstPersonCharacter::TakeAnyDamage);
	if (HealthComponent)
	{
		HealthComponent->OnDead.AddDynamic(this, &AFirstPersonCharacter::DeadEvent);
		HealthComponent->OnHealthChange.AddDynamic(this, &AFirstPersonCharacter::OnHealthChange);
	}

	SetReplicates(true);
}

void AFirstPersonCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AFirstPersonCharacter, CurrentWeapon);
	DOREPLIFETIME(AFirstPersonCharacter, bIsAiming);
	DOREPLIFETIME(AFirstPersonCharacter, bIsWeaponMain);
	DOREPLIFETIME(AFirstPersonCharacter, WeaponToPickUp);
	DOREPLIFETIME(AFirstPersonCharacter, AmmoToPickUp);
	DOREPLIFETIME(AFirstPersonCharacter, ChoosedLootBox);

}

// Called when the game starts or when spawned
void AFirstPersonCharacter::BeginPlay()
{
	Super::BeginPlay();

	//if(GetLocalRole() == ROLE_Authority)
	//{
	//	InitWeapon(WeaponSlots[0].WeaponName, WeaponSlots[0].AmmoAmount);
	//}
}

// Called every frame
void AFirstPersonCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	AFPS_PlayerController* PC = Cast<AFPS_PlayerController>(UGameplayStatics::GetPlayerController(GetWorld(), 0));
	if (GetController() && GetController()->IsLocalPlayerController() || PC && PC->SpectatingTarget == this)
	{
		if (GetController() && GetController()->IsLocalPlayerController())
		{
			//GEngine->AddOnScreenDebugMessage(-1, 0, FColor::Yellow, TEXT("IsLOCALController: true"));
		}
		if (CurrentWeapon)
		{
			FVector RelativeLocation = CurrentWeapon->GetActorForwardVector() * CurrentWeapon->AimCameraLocation.X + CurrentWeapon->GetActorRightVector() * CurrentWeapon->AimCameraLocation.Y + CurrentWeapon->GetActorUpVector() * CurrentWeapon->AimCameraLocation.Z;
			//UKismetMathLibrary::VInterpTo(AimCamera->GetComponentLocation(), CurrentWeapon->GetActorLocation() + RelativeLocation, 10)
			AimCamera->SetWorldLocation(CurrentWeapon->GetActorLocation() + RelativeLocation);
		}
		
		if (bIsAiming)
		{
			//FVector TargetRelativeLocation = GetActorForwardVector() * CurrentWeapon->WeaponSettings.WeaponLocationInAim.X + GetActorRightVector() * CurrentWeapon->WeaponSettings.WeaponLocationInAim.Y + GetActorUpVector() * CurrentWeapon->WeaponSettings.WeaponLocationInAim.Z;
			//FVector NewWeaponLocation = UKismetMathLibrary::VInterpTo(CurrentWeapon->GetActorLocation(), GetMesh()->GetComponentLocation() + TargetRelativeLocation, DeltaTime, 10);
			////FRotator NewWeaponRotation = UKismetMathLibrary::RInterpTo(CurrentWeapon->GetActorRotation(), GetActorRotation(), DeltaTime, 10);
			//CurrentWeapon->SetActorLocation(NewWeaponLocation);
			//CurrentWeapon->SetActorRotation(NewWeaponRotation);
			if (CurrentWeapon)
			{
				if (PC && PC->SpectatingTarget == this)
				{
					//GEngine->AddOnScreenDebugMessage(-1, 0, FColor::Yellow, TEXT("SpectatingTarget: true"));
				}
				FVector ViewDirection;
				FVector Location;
				ViewDirection = AimCamera->GetComponentRotation().Vector();
				Location = AimCamera->GetComponentLocation();

				FHitResult HitResult;
				FCollisionQueryParams Params;
				Params.AddIgnoredActor(this);
				GetWorld()->LineTraceSingleByChannel(HitResult, Location, Location + ViewDirection * 10000, ECollisionChannel::ECC_GameTraceChannel1, Params);
				//FVector ShootEndLocation = FVector(0);

				//if (HitResult.bBlockingHit)
				//{
				//	FVector RelativeLocation = GetActorForwardVector() * CurrentWeapon->WeaponSettings.ShootLocation.GetLocation().X + GetActorRightVector() * CurrentWeapon->WeaponSettings.ShootLocation.GetLocation().Y + GetActorUpVector() * CurrentWeapon->WeaponSettings.ShootLocation.GetLocation().Z;
				//	float DistanceToHit = (HitResult.Location - (CurrentWeapon->GetActorLocation() + RelativeLocation)).Size();
				//	if (DistanceToHit > 30)
				//	{
				//		ShootEndLocation = CurrentWeapon->ShootEndLocation;

				//	}
				//	else
				//	{
				//		ShootEndLocation = Location + ViewDirection * 10000.f;

				//	}
				//DrawDebugSphere(GetWorld(), HitResult.ImpactPoint, 10, 20, FColor::Red);
				//}
				//else
				//{
				//	ShootEndLocation = Location + ViewDirection * 10000;
				//}
				// 

				FVector ShootEndLocation;
				if (HitResult.bBlockingHit)
				{				
					FVector RelativeLocation = GetActorForwardVector() * CurrentWeapon->ShootLocation.GetLocation().X + GetActorRightVector() * CurrentWeapon->ShootLocation.GetLocation().Y + GetActorUpVector() * CurrentWeapon->ShootLocation.GetLocation().Z;
					float DistanceToHit = (HitResult.Location - (CurrentWeapon->GetActorLocation() + RelativeLocation)).Size();
					if (DistanceToHit > 100)
					{
						ShootEndLocation = HitResult.Location;
					}
					else
					{
						ShootEndLocation = Location + ViewDirection * 10000.f;
					}
				}
				else
				{
					ShootEndLocation = Location + ViewDirection * 10000.f;
				}

				////GEngine->AddOnScreenDebugMessage(-1, 3, FColor::Yellow, FString::Printf(TEXT("Location: %f - %f - %f"), ShootEndLocation.X, ShootEndLocation.Y, ShootEndLocation.Z));
				//FRotator NeedRotation = UKismetMathLibrary::FindLookAtRotation(CurrentWeapon->GetActorLocation(), ShootEndLocation);
				FRotator NeedRotation = UKismetMathLibrary::FindLookAtRotation(CurrentWeapon->GetSightMesh()->GetComponentLocation(), ShootEndLocation);
				//GEngine->AddOnScreenDebugMessage(-1, 0.0f, FColor::Yellow, FString::Printf(TEXT("Rotation: %f - %f - %f"), NewRotation.Pitch, NewRotation.Yaw, NewRotation.Roll));
				FRotator NewRotation = UKismetMathLibrary::RInterpTo(CurrentWeapon->GetSightMesh()->GetComponentRotation(), NeedRotation, DeltaTime, 5);
				CurrentWeapon->SetActorRotation(NeedRotation);
			}
		}
		if (GetMesh() && CurrentWeapon)
		{
			//FVector NewWeaponLocation = UKismetMathLibrary::VInterpTo(CurrentWeapon->GetActorLocation(), GetMesh()->GetSocketLocation(CurrentWeapon->WeaponSettings.SocketNameToAttach), DeltaTime, 10);
			//FRotator NewWeaponRotation = UKismetMathLibrary::RInterpTo(CurrentWeapon->GetActorRotation(), GetMesh()->GetSocketRotation(CurrentWeapon->WeaponSettings.SocketNameToAttach), DeltaTime, 10);
			//CurrentWeapon->SetActorLocation(NewWeaponLocation);
			//CurrentWeapon->SetActorRotation(NewWeaponRotation);
			///*if((CurrentWeapon->GetActorLocation() + FVector(2.f, 2.f, 2.f) > GetMesh()->GetSocketLocation(CurrentWeapon->WeaponSettings.SocketNameToAttach). || CurrentWeapon->GetActorLocation().X - 2.f > GetMesh()->GetSocketLocation(CurrentWeapon->WeaponSettings.SocketNameToAttach).X)
			//	&& (CurrentWeapon->GetActorLocation().Y + 2.f > GetMesh()->GetSocketLocation(CurrentWeapon->WeaponSettings.SocketNameToAttach).X || CurrentWeapon->GetActorLocation().X - 2.f > GetMesh()->GetSocketLocation(CurrentWeapon->WeaponSettings.SocketNameToAttach).Y)*/
			//if ((CurrentWeapon->GetActorLocation() - GetMesh()->GetSocketLocation(CurrentWeapon->WeaponSettings.SocketNameToAttach)).Size() < 2.f)
			//{
			//	FAttachmentTransformRules Rule(EAttachmentRule::SnapToTarget, false);
			//	CurrentWeapon->AttachToComponent(GetMesh(), Rule, CurrentWeapon->WeaponSettings.SocketNameToAttach);

		}

		if (Camera->IsActive())
		{
			SetCameraRotation_OnServer(Camera->GetComponentRotation());
		}
		else if (AimCamera->IsActive())
		{
			SetCameraRotation_OnServer(AimCamera->GetComponentRotation());
		}
	}
	

	if(GetLocalRole() == ROLE_Authority)
	{
		if(CurrentWeapon)
		{		
			//FVector RelativeLocation = FVector(0);
			//FVector MainLocation = FVector(0);
			//if (CurrentWeapon->bHasSight)
			//{
			//	RelativeLocation = CurrentWeapon->SightInfo.CameraRelativeLocation;
			//	MainLocation = CurrentWeapon->GetSightMesh()->GetComponentLocation();
			//}	
			//else
			//{
			//	RelativeLocation = CurrentWeapon->WeaponSettings.AimCameraLocationWithoutSight;
			//	MainLocation = CurrentWeapon->GetActorLocation();
			//}

			//FVector NewRelativeLocation = CurrentWeapon->GetActorForwardVector() * RelativeLocation.X +
			//CurrentWeapon->GetActorRightVector() * RelativeLocation.Y +
			//CurrentWeapon->GetActorUpVector() * RelativeLocation.Z;
	

			//AimCameraLocation = MainLocation + NewRelativeLocation;
			//SetAimCameraLocation_Multicast(CurrentWeapon->WeaponSettings.AimCameraLocation);
			if (CurrentWeapon)
			{
				if (GetVelocity().IsNearlyZero())
				{
					CurrentWeapon->bShouldReduceDispersion = true;
				}
				else
				{
					CurrentWeapon->bShouldReduceDispersion = false;
				}
			}
		}
		if(GetMesh() && GetMesh()->GetAnimInstance())
		{
			float DirectionDegree = GetMesh()->GetAnimInstance()->CalculateDirection(GetVelocity(), GetActorRotation());
			int8 i = 0;
			bool bIsFind = false;
			while (i < CharacterMovementSpeedByRotation.Num() && !bIsFind)
			{
				if (DirectionDegree > CharacterMovementSpeedByRotation[i].MinDegree && DirectionDegree < CharacterMovementSpeedByRotation[i].MaxDegree
				|| DirectionDegree < CharacterMovementSpeedByRotation[i].MinDegree * -1.f && DirectionDegree > CharacterMovementSpeedByRotation[i].MaxDegree * -1.f)
				{
					bIsFind = true;
					//if(GetCharacterMovement() && GetCharacterMovement()->MaxWalkSpeed != CharacterMovementSpeedByRotation[i].MovementSpeed)
					float CoefSpeedByAiming = 1.0f;
					if(bIsAiming)
					{
						CoefSpeedByAiming = 0.5f;
					}
					else
					{
						CoefSpeedByAiming = 1.f;
					}
					SetCharacterMovementSpeed_Multicast(CharacterMovementSpeedByRotation[i].CoefSpeed * ForwardSpeed* CoefSpeedByAiming);
				}
				i++;
			}
		}
	}

}

// Called to bind functionality to input
void AFirstPersonCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	PlayerInputComponent->BindAxis(TEXT("WeaponFire"), this, &AFirstPersonCharacter::WeaponFire_OnServer);
	PlayerInputComponent->BindAxis(TEXT("MoveForward"), this, &AFirstPersonCharacter::MoveForward);
	PlayerInputComponent->BindAxis(TEXT("MoveRight"), this, &AFirstPersonCharacter::MoveRight); 
	PlayerInputComponent->BindAction(TEXT("SwitchWeapon"),EInputEvent::IE_Pressed, this, &AFirstPersonCharacter::SwitchWeapon_OnServer);
	PlayerInputComponent->BindAction(TEXT("WeaponReload"), EInputEvent::IE_Pressed, this, &AFirstPersonCharacter::WeaponReload_OnServer);
	PlayerInputComponent->BindAction(TEXT("PickUpItem"), EInputEvent::IE_Pressed, this, &AFirstPersonCharacter::PickUpItem_OnServer);
	PlayerInputComponent->BindAxis(TEXT("WeaponAiming"), this, &AFirstPersonCharacter::WeaponAiming_OnServer);
}

void AFirstPersonCharacter::SetCameraRotation_OnServer_Implementation(FRotator NewRotation)
{
	//Camera->SetWorldRotation(NewRotation);
	FVector ViewDirection = NewRotation.Vector();
	FVector Location = FVector::ZeroVector;
	SetCameraRotation_Multicast(NewRotation);
	if (Camera && !bIsAiming)
	{
		Location = Camera->GetComponentLocation();
		//Camera->SetWorldRotation(NewRotation);

	}
	else if (AimCamera && bIsAiming)
	{
		Location = AimCamera->GetComponentLocation();
		//AimCamera->SetWorldRotation(NewRotation);
		//GEngine->AddOnScreenDebugMessage(-1, 0, FColor::Yellow, TEXT("AimCameraIsActive"));
	}
	FHitResult HitResult;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(this);
	GetWorld()->LineTraceSingleByChannel(HitResult, Location, Location + ViewDirection * 10000, ECollisionChannel::ECC_GameTraceChannel1, Params);
	//DrawDebugLine(GetWorld(), Location, Location + ViewDirection * 10000, FColor::Red, false, 0);

	if(CurrentWeapon)
	{
		//GEngine->AddOnScreenDebugMessage(-1, 0, FColor::Yellow, FString::Printf(TEXT("%s"), HitResult.Component->Name));
		if (HitResult.bBlockingHit)
		{
			FVector RelativeLocation = GetActorForwardVector() * CurrentWeapon->WeaponSettings.ShootLocation.GetLocation().X + GetActorRightVector() * CurrentWeapon->WeaponSettings.ShootLocation.GetLocation().Y + GetActorUpVector() * CurrentWeapon->WeaponSettings.ShootLocation.GetLocation().Z;
			float DistanceToHit = (HitResult.Location - (CurrentWeapon->GetActorLocation() + RelativeLocation)).Size();
			if(bIsAiming)
			{
				
				if (DistanceToHit > 100)
				{
					CurrentWeapon->ShootEndLocation = HitResult.Location;
					
				}
				else
				{
					CurrentWeapon->ShootEndLocation = Location + ViewDirection * 10000.f;
					
				}
			}
			else
			{
				CurrentWeapon->ShootEndLocation = HitResult.Location;
			}
			if (HitResult.GetActor() && HitResult.GetActor()->StaticClass())
			{
 
				//GEngine->AddOnScreenDebugMessage(-1, 0, FColor::Yellow,  HitResult.GetActor()->GetName());
			//	if(HitResult.GetActor()->StaticClass()->IsChildOf(APickUpWeapon::StaticClass()))
				if(DistanceToHit <= 150)
				{
					APickUpWeapon* Weapon = Cast<APickUpWeapon>(HitResult.GetActor());
					if (Weapon)
					{
						WeaponToPickUp = Weapon;
					}
					else
					{
						WeaponToPickUp = nullptr;
					}

					//	if (HitResult.GetActor()->StaticClass()->IsChildOf(APickUpAmmo::StaticClass()))

					APickUpAmmo* Ammo = Cast<APickUpAmmo>(HitResult.GetActor());
					if (Ammo)
					{
						AmmoToPickUp = Ammo;
					}
					else
					{
						AmmoToPickUp = nullptr;
					}
					ALootBox* LootBox = Cast<ALootBox>(HitResult.GetActor());
					if (LootBox)
					{
						ChoosedLootBox = LootBox;
					}
					else
					{
						ChoosedLootBox = nullptr;
					}
				}
				else
				{
					ChoosedLootBox = nullptr;
					AmmoToPickUp = nullptr;
					WeaponToPickUp = nullptr;
				}
				
			}
		}
		else
		{
			CurrentWeapon->ShootEndLocation = Location + ViewDirection * 10000.f;
		}
		//DrawDebugSphere(GetWorld(), CurrentWeapon->ShootEndLocation, 10, 20, FColor::Red, false, 0);
	}
	
}

void AFirstPersonCharacter::InitWeapon(FName WeaponName, int32 AmmoAmount)
{
	//OnServer
	UMyGameInstance* GI = Cast<UMyGameInstance>(GetGameInstance());
	if (GI)
	{
		FWeaponSettings WeaponSettings;
		if (GI->GetWeaponSettingsByName(WeaponName, WeaponSettings))
		{
			if (WeaponSettings.WeaponClass)
			{
				if (CurrentWeapon)
				{
					CurrentWeapon->Destroy();
				}
				FActorSpawnParameters SpawnParams;
				SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
				SpawnParams.Owner = this;
				SpawnParams.Instigator = GetInstigator();
				ABaseWeapon* NewWeapon = GetWorld()->SpawnActor<ABaseWeapon>(WeaponSettings.WeaponClass, SpawnParams);
				if (NewWeapon)
				{
					bIsWeaponMain = WeaponSettings.bIsWeaponMain;
					CurrentWeapon = NewWeapon;
					CurrentWeapon->AmmoAmount = AmmoAmount;
					NewWeapon->InitWeapon(WeaponSettings);
					FAttachmentTransformRules Rule(EAttachmentRule::SnapToTarget, false);
					NewWeapon->AttachToComponent(GetMesh(), Rule, WeaponSettings.SocketNameToAttach);
					NewWeapon->OnWeaponFire.AddDynamic(this, &AFirstPersonCharacter::OnWeaponFire_OnServer);
					NewWeapon->OnWeaponReloadEnd.AddDynamic(this, &AFirstPersonCharacter::ChangeAmmoAmount);
					NewWeapon->OnWeaponReloadStart.AddDynamic(this, &AFirstPersonCharacter::OnWeaponReloadStart);
					NewWeapon->AimCameraLocation = WeaponSettings.AimCameraLocationWithoutSight;
					//AimCameraLocation = WeaponSettings.AimCameraLocationWithoutSight;
					{
						int32 i = 0;
						bool bIsFind = false;
						while (i < WeaponTuningSlots.Num() && !bIsFind)
						{
							if (WeaponTuningSlots[i].WeaponName == WeaponName)
							{
								for (int32 j = 0; j < WeaponTuningSlots[i].TuningNames.Num(); j++)
								{
									TuningWeapon(WeaponTuningSlots[i].TuningNames[j]);
									
								}
								bIsFind = true;
							}
							i++;
						}
						if (!bIsFind)
						{
							AimCamera->FieldOfView = 70.f;
						}
					}
					if (GetController())
					{
						AFPS_PlayerController* PC = Cast<AFPS_PlayerController>(GetController());
						if (PC)
						{
							bool bIsFind = false;
							int32 i = 0;
							int32 MaxClipAmount = 0;
							int32 ClipAmount = 0;
							while(!bIsFind && i < AmmoSlots.Num())
							{
								if (AmmoSlots[i].AmmoType == WeaponSettings.AmmoType)
								{
									MaxClipAmount = AmmoSlots[i].MaxClipAmount;
									ClipAmount = AmmoSlots[i].ClipAmount;
								}
								i++;
							}
							if (ClipAmount > 0)
							{
								NewWeapon->bIsAmmoSlotEmpty = false;
							}
							else
							{
								NewWeapon->bIsAmmoSlotEmpty = true;
							}
							
							PC->InitInventorySlotsWidget_Multicast(AmmoAmount, WeaponSettings.MaxAmmoAmount, ClipAmount, MaxClipAmount, WeaponSettings.Image);
						}
					}
					
					//FAttachmentTransformRules CameraRule(EAttachmentRule::SnapToTarget, EAttachmentRule::KeepWorld, EAttachmentRule::KeepRelative, false);
					//AimCamera->AttachToComponent(NewWeapon->GetRootComponent(), CameraRule);
					/*FVector RelativeLocation = NewWeapon->GetActorForwardVector() * WeaponSettings.AimCameraLocation.X + NewWeapon->GetActorRightVector() * WeaponSettings.AimCameraLocation.Y + NewWeapon->GetActorUpVector() * WeaponSettings.AimCameraLocation.Z;
					AimCamera->SetWorldLocation(NewWeapon->GetActorLocation() + RelativeLocation);*/
					//AimCamera->SetWorldRotation(NewWeapon->GetActorRotation());

					//FVector SocketLocation = GetMesh()->GetSocketLocation(FName("WeaponSocketRightHand"));
					//FVector SpawnRelativeLocation = GetActorForwardVector() * WeaponSettings.WeaponSpawnRelativeLocation.X + GetActorRightVector() * WeaponSettings.WeaponSpawnRelativeLocation.Y + GetActorUpVector() * WeaponSettings.WeaponSpawnRelativeLocation.Z;
					//NewWeapon->SetActorLocation(SocketLocation + SpawnRelativeLocation);
				}
			}

		}
		
	}
}

void AFirstPersonCharacter::WeaponFire_OnServer_Implementation(float Value)
{
	bool bIsFire = false;
	if (Value > 0.0f)
	{
		bIsFire = true;
	}
	if (CurrentWeapon && HealthComponent && HealthComponent->GetIsAlive())
	{
		CurrentWeapon->SetWeaponStateFire(bIsFire);
	}

}

void AFirstPersonCharacter::MoveForward(float Value)
{
	AddMovementInput(GetActorForwardVector(), Value);

}

void AFirstPersonCharacter::MoveRight(float Value)
{
	AddMovementInput(GetActorRightVector(), Value);
}

void AFirstPersonCharacter::OnWeaponFire_OnServer_Implementation()
{
	if(CurrentWeapon)
	{
		if (!bIsAiming && CurrentWeapon->WeaponSettings.CharacterHipFireAnim && GetController() && !GetController()->IsLocalPlayerController())
		{
			PlayMontage_Multicast(CurrentWeapon->WeaponSettings.CharacterHipFireAnim);
		}
		if (bIsAiming && CurrentWeapon->WeaponSettings.CharacterAimFireAnim && GetController() && !GetController()->IsLocalPlayerController())
		{
			PlayMontage_Multicast(CurrentWeapon->WeaponSettings.CharacterAimFireAnim);		
		}
		float Recoil = 0.f;
		if (bIsAiming)
		{
			Recoil = CurrentWeapon->WeaponSettings.AimRecoil;
		}
		else
		{
			Recoil = CurrentWeapon->WeaponSettings.HipRecoil;
		}
		OnWeaponFire_Multicast(Recoil);
		if (GetController())
		{
			AFPS_PlayerController* PC = Cast<AFPS_PlayerController>(GetController());
			if (PC)
			{
				bool bIsFind = false;
				int32 i = 0;
				int32 ClipsAmount = 0;
				int32 MaxClipsAmount = 0;
				while (!bIsFind && i < AmmoSlots.Num())
				{
					if(AmmoSlots[i].AmmoType == CurrentWeapon->WeaponSettings.AmmoType)
					{
						bIsFind = true;
						ClipsAmount = AmmoSlots[i].ClipAmount;
						MaxClipsAmount = AmmoSlots[i].MaxClipAmount;
					}
					i++;
				}
				PC->InitInventorySlotsWidget_Multicast(CurrentWeapon->AmmoAmount, CurrentWeapon->WeaponSettings.MaxAmmoAmount, ClipsAmount, MaxClipsAmount, CurrentWeapon->WeaponSettings.Image);
			}
		}
	}

}

void AFirstPersonCharacter::OnWeaponFire_Multicast_Implementation(float Recoil)
{
	AddControllerPitchInput(-Recoil);
}
void AFirstPersonCharacter::SetCharacterMovementSpeed_Multicast_Implementation(float NewSpeed)
{
	if(GetCharacterMovement())
	{
		GetCharacterMovement()->MaxWalkSpeed = NewSpeed;
	}
}

void AFirstPersonCharacter::WeaponReload_OnServer_Implementation()
{
	if (CurrentWeapon && !CurrentWeapon->bIsWeaponReloading && !CurrentWeapon->bIsAmmoSlotEmpty)
	{
		CurrentWeapon->WeaponReloadStart();
	}
}

void AFirstPersonCharacter::SwitchWeapon_OnServer_Implementation()
{
	if(CurrentWeapon)
	{
		if (bIsWeaponMain)
		{
			WeaponSlots[0].AmmoAmount = CurrentWeapon->AmmoAmount;
			InitWeapon(WeaponSlots[1].WeaponName, WeaponSlots[1].AmmoAmount);
		}
		else
		{
			WeaponSlots[1].AmmoAmount = CurrentWeapon->AmmoAmount;
			InitWeapon(WeaponSlots[0].WeaponName, WeaponSlots[0].AmmoAmount);
		}
	}
	else
	{
		InitWeapon(WeaponSlots[0].WeaponName, WeaponSlots[0].AmmoAmount);
	}
	if(CurrentWeapon)
	{
		SetStateAiming_Multicast(false, nullptr, FVector(), CurrentWeapon->WeaponSettings.SocketNameToAttach);
	}
}

void AFirstPersonCharacter::WeaponAiming_OnServer_Implementation(float Value)
{
	if (Value > 0)
	{
		if(bIsAiming != true)
		{
			bIsAiming = true;
			if(CurrentWeapon && !CurrentWeapon->bIsWeaponReloading)
			{
				CurrentWeapon->CoefDispersionWhileAiming = CurrentWeapon->WeaponSettings.CoefDispersionWhileAiming;
				//SetStateAiming_OnClient(CurrentWeapon->WeaponSettings.LocationInAiming);
	
				SetStateAiming_Multicast(true, CurrentWeapon->WeaponSettings.CharacterAimAnim, CurrentWeapon->WeaponSettings.WeaponLocationInAim, CurrentWeapon->WeaponSettings.SocketNameToAttach);;
				
			}
		}
	}
	else
	{
		if(bIsAiming != false)
		{
			if(CurrentWeapon)
			{
				CurrentWeapon->CoefDispersionWhileAiming = 1.f;
			}
			bIsAiming = false;
			//SetStateAiming_OnClient(GetMesh()->GetSocketLocation(CurrentWeapon->WeaponSettings.SocketNameToAttach))
			SetStateAiming_Multicast(false, CurrentWeapon->WeaponSettings.CharacterAimAnim, CurrentWeapon->WeaponSettings.WeaponLocationInAim, CurrentWeapon->WeaponSettings.SocketNameToAttach);
		}
	}

}


void AFirstPersonCharacter::SetMovementState_Multicast_Implementation(EMovementState NewMovementState)
{
	MovementState = NewMovementState;
	switch (NewMovementState)
	{
	case EMovementState::Running:
		ForwardSpeed = 600;
		break;
	case EMovementState::Aiming:
		ForwardSpeed = 200;
		break;
	default:
		break;
	}
}

void AFirstPersonCharacter::SetStateAiming_Multicast_Implementation(bool IsAiming, UAnimMontage* AimAnim, FVector WeaponLocationInAim, FName SocketNameToAttachWeapon)
{
	if (IsAiming && !CurrentWeapon->bIsWeaponReloading)
	{
		AFPS_PlayerController* PC = Cast<AFPS_PlayerController>(UGameplayStatics::GetPlayerController(GetWorld(), 0));
		if (GetController() && GetController()->IsLocalPlayerController() || PC && PC->SpectatingTarget == this)
		{
			if (GetLocalRole() == ROLE_SimulatedProxy)
			{
				//GEngine->AddOnScreenDebugMessage(-1, 5, FColor::Green, TEXT("Client"));
			}
			if (GetMesh())
			{
				SetStateAiming_BP(true);
				GetMesh()->HideBoneByName(FName("LeftArm"), EPhysBodyOp::PBO_None);
				GetMesh()->HideBoneByName(FName("RightArm"), EPhysBodyOp::PBO_None);
				//GetMesh()->SetVisibility(false);
				if (CurrentWeapon)
				{
					if (GetLocalRole() == ROLE_Authority)
					{
						CurrentWeapon->SetReplicates(false);
					}
					CurrentWeapon->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
					FAttachmentTransformRules Rule(EAttachmentRule::SnapToTarget, false);
					CurrentWeapon->AttachToComponent(GetMesh(), Rule);
					FVector TargetRelativeLocation = GetActorForwardVector() * WeaponLocationInAim.X + GetActorRightVector() * WeaponLocationInAim.Y + GetActorUpVector() * WeaponLocationInAim.Z;
					FVector NewWeaponLocation = GetActorLocation() + TargetRelativeLocation;
					CurrentWeapon->SetActorLocation(NewWeaponLocation);
					
				}
			}
			Camera->Deactivate();
			AimCamera->Activate();
			
		}
		if(GetMesh() && GetMesh()->GetAnimInstance() && AimAnim)
		{
			GetMesh()->GetAnimInstance()->Montage_Play(AimAnim);
		}
	}
	else
	{
		AFPS_PlayerController* PC = Cast<AFPS_PlayerController>(UGameplayStatics::GetPlayerController(GetWorld(), 0));
		if (GetController() && GetController()->IsLocalPlayerController() || PC && PC->SpectatingTarget == this)
		{
			SetStateAiming_BP(false);
			if (GetMesh())
			{
				GetMesh()->UnHideBoneByName(FName("LeftArm"));
				GetMesh()->UnHideBoneByName(FName("RightArm"));
				//GetMesh()->SetVisibility(true);
				if (CurrentWeapon)
				{
					if (GetLocalRole() == ROLE_Authority)
					{
						CurrentWeapon->SetReplicates(true);
					}
					FAttachmentTransformRules Rule(EAttachmentRule::SnapToTarget, false);
					CurrentWeapon->AttachToComponent(GetMesh(), Rule, SocketNameToAttachWeapon);
				}
			}
			AimCamera->Deactivate();
			Camera->Activate();
			
		}
		if (GetMesh() && GetMesh()->GetAnimInstance() && CurrentWeapon && !CurrentWeapon->bIsWeaponReloading)
		{
			GetMesh()->GetAnimInstance()->StopAllMontages(0.25f);
		}
	}
}

void AFirstPersonCharacter::SetStateAiming_OnClient_Implementation(FVector LocationInAiming)
{
	if (CurrentWeapon)
	{
		FVector RelativeLocation = GetActorForwardVector() * LocationInAiming.X + GetActorRightVector() * LocationInAiming.Y + GetActorUpVector() * LocationInAiming.Z;
		CurrentWeapon->SetActorLocation(GetActorLocation() + RelativeLocation);
	}

}

void AFirstPersonCharacter::DeadEvent()
{
	//OnServer

	if(HealthComponent && HealthComponent->LastDamageInstigator)
	{
		AFPS_PlayerController* PC = Cast<AFPS_PlayerController>(HealthComponent->LastDamageInstigator);
		if (PC)
		{
			if(GetWorld() && GetWorld()->GetAuthGameMode())
			{
				AFPSGameMode* GM = Cast<AFPSGameMode>(GetWorld()->GetAuthGameMode());
				if (GM)
				{
					if(GetController() && PC != GetController())
					{
						AFPS_PlayerState* PS = Cast<AFPS_PlayerState>(PC->PlayerState);
						if (PS)
						{
							PS->Kills++;
							PC->IncrementPlayerKills_Multicast(PS->Kills);
						}

						GM->IncrementTeamKills(PC->TeamNumber, PC);

					}

					if (GetController() && GetController()->PlayerState)
					{
						AFPS_PlayerController* OwningPC = Cast<AFPS_PlayerController>(GetController());
						//AFPS_PlayerState* PS = Cast<AFPS_PlayerState>(GetController()->PlayerState);
						if (OwningPC)
						{
							//PS->WeaponSlots = WeaponSlots;
							GM->PlayerDeath(OwningPC);

						}
					}
				}
			}

			/*if(PC->PlayerState)
			{
				AFPS_PlayerState* PS = Cast<AFPS_PlayerState>(PC->PlayerState);
				if (PS)
				{
					PS->Kills++;
				}
			}*/
		}
	}
	if (GetController())
	{
		GetController()->UnPossess();
		if (GetCapsuleComponent())
		{
			GetCapsuleComponent()->SetCollisionProfileName(FName("OverlapAll"));
		}
		if (CurrentWeapon)
		{
			CurrentWeapon->SetWeaponStateFire(false);
		}
	}
	if(DeadAnim)
	{
		PlayMontage_Multicast(DeadAnim);
		float Time = DeadAnim->GetSectionLength(0);
		if (GetWorld())
		{
			GetWorld()->GetTimerManager().SetTimer(RagdollTimer, this, &AFirstPersonCharacter::MakeRagdoll_Multicast, Time, false);
		}
	}
}

void AFirstPersonCharacter::TakeAnyDamage(AActor* DamagedActor, float Damage, const UDamageType* DamageType, AController* InstigateBy, AActor* DamageCauser)
{
	//Super::TakeDamage(DamagedActor, Damage, DamageType, InstigateBy, DamageCauser);
	if (HealthComponent)
	{	
		if(InstigateBy)
		{
			HealthComponent->LastDamageInstigator = InstigateBy;
		}	
		HealthComponent->ChangeCurrentHealth_OnServer(-Damage);
	}
}

void AFirstPersonCharacter::ChangeAmmoAmount(EAmmoTypes AmmoType, int32 ChangeValue)
{
	bool bIsFind = false;
	int32 i = 0;
	while (!bIsFind && i < AmmoSlots.Num())
	{
		if (AmmoSlots.IsValidIndex(i) && AmmoSlots[i].AmmoType == AmmoType)
		{
			bIsFind = true;
			if(AmmoSlots[i].ClipAmount + ChangeValue <= AmmoSlots[i].MaxClipAmount)
			{
				AmmoSlots[i].ClipAmount += ChangeValue;
			}
			if(CurrentWeapon)
			{
				if (AmmoSlots[i].ClipAmount <= 0)
				{
					CurrentWeapon->bIsAmmoSlotEmpty = true;
				}
				else
				{
					CurrentWeapon->bIsAmmoSlotEmpty = false;
				}
			}
			if(CurrentWeapon->WeaponSettings.AmmoType == AmmoType)
			{
				if (GetController())
				{
					AFPS_PlayerController* PC = Cast<AFPS_PlayerController>(GetController());
					if (PC)
					{
						PC->InitInventorySlotsWidget_Multicast(CurrentWeapon->AmmoAmount, CurrentWeapon->WeaponSettings.MaxAmmoAmount, AmmoSlots[i].ClipAmount, AmmoSlots[i].MaxClipAmount, CurrentWeapon->WeaponSettings.Image);
					}
				}
			}
		}
		i++;
	}

}

void AFirstPersonCharacter::OnHealthChange(float NewHealth, float ChangeValue)
{
	if (GetController())
	{
		AFPS_PlayerController* PC = Cast<AFPS_PlayerController>(GetController());
		if (PC)
		{
			PC->InitHealthBarWidget_Multicast(NewHealth, ChangeValue);
		}
	}
}

void AFirstPersonCharacter::OnWeaponReloadStart()
{
	if (CurrentWeapon)
	{
		SetStateAiming_Multicast(false, nullptr, FVector(0), CurrentWeapon->WeaponSettings.SocketNameToAttach);
		PlayMontage_Multicast(CurrentWeapon->WeaponSettings.CharacterReloadAnim);
	}
}

void AFirstPersonCharacter::TuningWeapon(FName TuningName)
{
	FWeaponTuningInfo TuningInfo;
	if (UMyGameInstance* GI = Cast<UMyGameInstance>(GetGameInstance()))
	{
		if (GI->GetWeaponTuningInfoByName(TuningName, TuningInfo) && CurrentWeapon)
		{
			switch (TuningInfo.TuningType)
			{
			case ETuningTypes::Sight:
				if(CurrentWeapon)
				{
					CurrentWeapon->SightInfo = TuningInfo;
					CurrentWeapon->InitSightMesh_Multicast(TuningInfo.StaticMesh, CurrentWeapon->WeaponSettings.SightLocation);
					CurrentWeapon->SightMesh = TuningInfo.StaticMesh;
					//AimCameraLocation = CurrentWeapon->WeaponSettings.AimCameraLocationInSight;
					CurrentWeapon->bHasSight = true;
					CurrentWeapon->AimCameraLocation = TuningInfo.CameraRelativeLocation + CurrentWeapon->GetSightMesh()->GetRelativeLocation();
				}
				AimCamera->FieldOfView = TuningInfo.FieldOfView;
				break;
			default:
				break;
			}
		
		}
	}
}

void AFirstPersonCharacter::InitInventory_OnServer_Implementation(const TArray<FWeaponSlot> &NewWeaponSlots, const TArray<FWeaponTuningSlot> &NewWeaponTuningSlots)
{
	WeaponSlots = NewWeaponSlots;
	WeaponTuningSlots = NewWeaponTuningSlots;
	InitWeapon(NewWeaponSlots[0].WeaponName, NewWeaponSlots[0].AmmoAmount);
}

void AFirstPersonCharacter::PickUpItem_OnServer_Implementation()
{
	if (WeaponToPickUp)
	{
		if(GetGameInstance())
		{
			UMyGameInstance* GI = Cast<UMyGameInstance>(GetGameInstance());
			if (GI)
			{
				FWeaponSettings NewWeaponSettings;
				if(GI->GetWeaponSettingsByName(WeaponToPickUp->WeaponSlot.WeaponName, NewWeaponSettings))
				{
					int32 SlotNumber = 0;
					if (!NewWeaponSettings.bIsWeaponMain)
					{
						SlotNumber = 1;
					}
					if(GetWorld() && GetWorld()->GetAuthGameMode())
					{	
						AFPSGameMode* GM = Cast<AFPSGameMode>(GetWorld()->GetAuthGameMode());
						if (GM && GM->PickUpWeaponClass)
						{		
							FActorSpawnParameters SpawnParams;
							SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
							APickUpWeapon* DropWeapon = GetWorld()->SpawnActor<APickUpWeapon>(GM->PickUpWeaponClass, GetActorLocation() + GetActorForwardVector() * 70, GetActorRotation(), SpawnParams);
							if (DropWeapon && CurrentWeapon)
							{
								FWeaponSlot OldWeaponSlot;
								OldWeaponSlot.WeaponName = WeaponSlots[SlotNumber].WeaponName;
								OldWeaponSlot.AmmoAmount = CurrentWeapon->AmmoAmount;
								FWeaponTuningSlot OldTuningSlot;
								int32 i = 0;
								bool bIsFind = false;
								while (i < WeaponTuningSlots.Num() && !bIsFind)
								{
									if (WeaponTuningSlots[i].WeaponName == WeaponSlots[SlotNumber].WeaponName)
									{
										bIsFind = true;
										OldTuningSlot = WeaponTuningSlots[i];
										WeaponTuningSlots.RemoveAt(i);
									}
									i++;
								}
								DropWeapon->InitItem(OldWeaponSlot, OldTuningSlot);
								//DropWeapon->GetStaticMeshComponent()->AddImpulse(GetActorForwardVector() * 100);
							}
						}
					}		
					FWeaponTuningSlot NewTuningSlot = WeaponToPickUp->TuningSlot;
					WeaponTuningSlots.Add(NewTuningSlot);
					if (NewWeaponSettings.bIsWeaponMain == bIsWeaponMain)
					{
						InitWeapon(WeaponToPickUp->WeaponSlot.WeaponName, WeaponToPickUp->WeaponSlot.AmmoAmount);
					}
					
					WeaponSlots[SlotNumber].AmmoAmount = WeaponToPickUp->WeaponSlot.AmmoAmount;
					WeaponSlots[SlotNumber].WeaponName = WeaponToPickUp->WeaponSlot.WeaponName;
					

					WeaponToPickUp->Destroy();
				}
			}
		}
	}
	if (AmmoToPickUp)
	{
		ChangeAmmoAmount(AmmoToPickUp->AmmoSlot.AmmoType, AmmoToPickUp->AmmoSlot.ClipAmount);
		AmmoToPickUp->Destroy();
	}
	if (ChoosedLootBox)
	{
		ChoosedLootBox->OpenLootBox();
	}
}

void AFirstPersonCharacter::SetCameraRotation_Multicast_Implementation(FRotator NewRotation)
{
	if (GetController() && GetController()->IsLocalPlayerController())
	{

	}
	else
	{
		if (Camera && !bIsAiming)
		{
			Camera->SetWorldRotation(NewRotation);

		}
		else if (AimCamera && bIsAiming)
		{
			AimCamera->SetWorldRotation(NewRotation);
		}
	}
}


void AFirstPersonCharacter::SetStateAiming_BP_Implementation(bool IsAiming)
{
	//BP
}

void AFirstPersonCharacter::MakeRagdoll_Multicast_Implementation()
{
	if (GetMesh())
	{
		GetMesh()->SetSimulatePhysics(true);
	}
}

void AFirstPersonCharacter::PlayMontage_Multicast_Implementation(UAnimMontage* AnimMontage)
{
	if (GetMesh() && GetMesh()->GetAnimInstance() && AnimMontage)
	{
		GetMesh()->GetAnimInstance()->Montage_Play(AnimMontage);
	}
}

void AFirstPersonCharacter::SpawnFX_Multicast_Implementation(UParticleSystem* FX, FVector Location, FRotator Rotation)
{
	if (GetWorld() && FX)
	{
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), FX, FTransform(Rotation, Location, FVector(0.5f)));
	}
}

void AFirstPersonCharacter::SpawnSound_Multicast_Implementation(USoundBase* Sound, FVector Location)
{
	if (Sound && GetWorld())
	{
		UGameplayStatics::PlaySoundAtLocation(GetWorld(), Sound, Location);
	}
}

void AFirstPersonCharacter::SpawnDecal_Multicast_Implementation(UMaterialInterface* DecalMaterial, UPrimitiveComponent* OtherComp, FHitResult Hit)
{
	if (OtherComp && DecalMaterial)
	{
		UGameplayStatics::SpawnDecalAttached(DecalMaterial, FVector(10), OtherComp, NAME_None, Hit.ImpactPoint, Hit.ImpactNormal.Rotation(), EAttachLocation::KeepWorldPosition, 10.f);
	}
}