// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Camera/CameraComponent.h"
#include "MyFunctionLibrary.h"
//
#include "FirstPersonCharacter.generated.h"

class UHealthComponent;
class ABaseWeapon;
class APickUpWeapon;
class APickUpAmmo;
class ALootBox;

UCLASS()
class FIRSTPERSONPROJECT_API AFirstPersonCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AFirstPersonCharacter();

	UPROPERTY(EditDefaultsOnly)
	float RagdollTime = 1.f;
	FTimerHandle RagdollTimer;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Mesh")
	UAnimMontage* DeadAnim = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	TArray<FWeaponSlot> WeaponSlots;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	TArray<FAmmoSlot> AmmoSlots;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	TArray<FWeaponTuningSlot> WeaponTuningSlots;
	UPROPERTY(BlueprintReadOnly, Replicated)
	ABaseWeapon* CurrentWeapon = nullptr;

	UPROPERTY(Replicated, BlueprintReadOnly)
	bool bIsWeaponMain = true;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	TArray <FCharacterMovementSpeedByRotation> CharacterMovementSpeedByRotation;

	UPROPERTY(BlueprintReadWrite)
	EMovementState MovementState = EMovementState::Running;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	float ForwardSpeed = 1.f;
	UPROPERTY(BlueprintReadWrite, Replicated)
	bool bIsAiming = false;

	UPROPERTY(BlueprintReadOnly, Replicated)
	APickUpWeapon* WeaponToPickUp = nullptr;

	UPROPERTY(BlueprintReadOnly, Replicated)
	APickUpAmmo* AmmoToPickUp = nullptr;
	UPROPERTY(Replicated)
	FVector AimCameraLocation = FVector();

	UPROPERTY(BlueprintReadOnly, Replicated)
	ALootBox* ChoosedLootBox = nullptr;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Mesh")
	class USkeletalMesh* FirstPersonSkeletalMesh = nullptr;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Mesh")
	class USkeletalMesh* ThirdPersonSkeletalMesh = nullptr;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Camera")
	class UCameraComponent* Camera = nullptr;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Camera")
	class UCameraComponent* AimCamera = nullptr;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "ActorComponent")
	class UHealthComponent* HealthComponent = nullptr;
public:	

	UHealthComponent* GetHealthComponent() { return HealthComponent; }
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	UFUNCTION(BlueprintCallable)
	void InitWeapon(FName WeaponName, int32 AmmoAmount);

	UFUNCTION(Server, Unreliable)
	void WeaponFire_OnServer(float Value);
	void MoveForward(float Value);
	void MoveRight(float Value);
	UFUNCTION(Server, Unreliable)
	void SwitchWeapon_OnServer();
	UFUNCTION(Server, Reliable)
	void WeaponReload_OnServer();
	UFUNCTION(NetMulticast, Unreliable)
	void SetCharacterMovementSpeed_Multicast(float NewSpeed);
	UFUNCTION(Server, Reliable)
	void OnWeaponFire_OnServer();
	UFUNCTION(NetMulticast, Reliable)
	void OnWeaponFire_Multicast(float Recoil);
	UFUNCTION(Server, Unreliable)
	void SetCameraRotation_OnServer(FRotator NewRotation);
	UFUNCTION(Server, Unreliable)
	void WeaponAiming_OnServer(float Value);
	UFUNCTION(NetMulticast, Reliable)
	void SetStateAiming_Multicast(bool IsAiming, UAnimMontage* AimAnim, FVector WeaponLocationInAim, FName SocketNameToAttachWeapon);
	UFUNCTION(NetMulticast, Reliable)
	void SetMovementState_Multicast(EMovementState NewMovementState);
	UFUNCTION(Client, Reliable)
	void SetStateAiming_OnClient(FVector LocationInAiming);
	UFUNCTION()
	void DeadEvent();
	UFUNCTION(NetMulticast, Reliable)
	void PlayMontage_Multicast(UAnimMontage* AnimMontage);
	UFUNCTION()
	void TakeAnyDamage(AActor* DamagedActor, float Damage, const UDamageType* DamageType, AController* InstigateBy, AActor* DamageCauser);
	UFUNCTION(NetMulticast, Reliable)
	void MakeRagdoll_Multicast();
	UFUNCTION(BlueprintNativeEvent)
	void SetStateAiming_BP(bool IsAiming);
	
	UFUNCTION()
	void ChangeAmmoAmount(EAmmoTypes AmmoType, int32 ChangeValue);

	UFUNCTION()
	void OnHealthChange(float NewHealth, float ChangeValue);

	UFUNCTION()
	void OnWeaponReloadStart();

	UFUNCTION(NetMulticast, Unreliable)
	void SetCameraRotation_Multicast(FRotator NewRotation);

	UFUNCTION(Server, Reliable)
	void PickUpItem_OnServer();

	UFUNCTION(Server, Reliable)
	void InitInventory_OnServer(const TArray<FWeaponSlot> &NewWeaponSlots, const TArray<FWeaponTuningSlot> &NewWeaponTuningSlots);
	UFUNCTION(BlueprintCallable)
	void TuningWeapon(FName TuningName);

	UFUNCTION(NetMulticast, Reliable)
	void SpawnFX_Multicast(UParticleSystem* FX, FVector Location, FRotator Rotation);
	UFUNCTION(NetMulticast, Reliable)
	void SpawnSound_Multicast(USoundBase* Sound, FVector Location);
	UFUNCTION(NetMulticast, Reliable)
	void SpawnDecal_Multicast(UMaterialInterface* DecalMaterial, UPrimitiveComponent* OtherComp, FHitResult Hit);
};
