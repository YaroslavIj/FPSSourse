// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "MyFunctionLibrary.h"
//
#include "BaseWeapon.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnWeaponFire);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnWeaponReloadStart);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnWeaponReloadEnd, EAmmoTypes, AmmoType, int32, ChangeValue);

UCLASS()
class FIRSTPERSONPROJECT_API ABaseWeapon : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ABaseWeapon();

	FOnWeaponFire OnWeaponFire;
	FOnWeaponReloadStart OnWeaponReloadStart;
	FOnWeaponReloadEnd OnWeaponReloadEnd;

	FTimerHandle ReloadTimer;

	float FireTimer = 0.0f;
	UPROPERTY(BlueprintReadOnly)
	FWeaponSettings WeaponSettings;
	UPROPERTY(BlueprintReadOnly)
	FWeaponTuningInfo SightInfo;
	UPROPERTY(Replicated)
	FVector ShootEndLocation = FVector();
	bool bShouldReduceDispersion = false;
	float MaxDispersion = 0.0f;
	float MinDispersion = 0.0f;
	UPROPERTY(Replicated)
	bool bIsWeaponReloading = false;
	int32 AmmoAmount = 0.0f;
	UPROPERTY(Replicated, BlueprintReadOnly)
	UStaticMesh* WeaponStaticMesh = nullptr;
	UPROPERTY(Replicated, BlueprintReadOnly)
	USkeletalMesh* WeaponSkeletalMesh = nullptr;
	UPROPERTY(Replicated, BlueprintReadOnly)
	UStaticMesh* SightMesh = nullptr;

	float CoefDispersionWhileAiming = 0.0f;

	UPROPERTY(Replicated)
	FTransform ShootLocation = FTransform();
	UPROPERTY(BlueprintReadWrite)
	bool bIsAmmoSlotEmpty = false;

	UPROPERTY()
	bool bHasSight = false;

	UPROPERTY(Replicated)
	FVector AimCameraLocation = FVector(0);
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Mesh")
	USkeletalMeshComponent* WeaponSkeletalMeshComponent = nullptr;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Mesh")
	UStaticMeshComponent* WeaponStaticMeshComponent = nullptr;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Mesh")
	UStaticMeshComponent* SightMeshComponent = nullptr;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Scene")
	USceneComponent* SceneComponent = nullptr;
	bool bIsFireButtonPressed = false;
	float CurrentDispersion = 0.0f;
public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UStaticMeshComponent* GetSightMesh() { return SightMeshComponent; }
	void SetWeaponStateFire(bool bIsFire);
	void InitWeapon(FWeaponSettings NewWeaponSettings);
	void Fire();
	void FireTick(float DeltaTime);
	void DispersionTick(float DeltaTime);
	float GetCurrentDispersion();
	UFUNCTION(NetMulticast, Reliable)
	void SpawnFireFX_Multicast(UParticleSystem* FX, FVector Location);
	UFUNCTION(NetMulticast, Reliable)
	void SpawnFireSound_Multicast(USoundBase* Sound, FVector Location);
	void Reload();
	void WeaponReloadStart();
	UFUNCTION(NetMulticast, Reliable)
	void InitMesh_Multicast(USkeletalMesh* SkeletalMesh, UStaticMesh* StaticMesh);

	UFUNCTION(NetMulticast, Reliable)
	void SpawnHitSound_Multicast(USoundBase* Sound, FHitResult Hit);
	UFUNCTION(NetMulticast, Reliable)
	void SpawnHitDecal_Multicast(UMaterialInterface* DecalMaterial, UPrimitiveComponent* OtherComp, FHitResult Hit);

	UFUNCTION(NetMulticast, Reliable)
	void SpawnHitFX_Multicast(UParticleSystem* FX, FHitResult Hit);
	UFUNCTION(NetMulticast, Reliable)
	void InitSightMesh_Multicast(UStaticMesh* NewMesh, FTransform SightTransform);
};
