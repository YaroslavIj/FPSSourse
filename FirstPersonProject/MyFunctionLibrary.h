// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Animation/SkeletalMeshActor.h"
#include "Chaos/ChaosEngineInterface.h"
//
#include "MyFunctionLibrary.generated.h"

class ABaseWeapon;
class AFirstPersonCharacter;
class ABaseProjectile;
/**
 * 
 */
UCLASS()
class FIRSTPERSONPROJECT_API UMyFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
};

UENUM(BlueprintType)
enum class EAmmoTypes : uint8
{
	RifleType,
	ShotgunType,
	PistolType,
	SniperType,
};

UENUM(BlueprintType)
enum class ETuningTypes : uint8
{
	Sight,
};

UENUM(BlueprintType)
enum class EMovementState: uint8
{
	Running,
	Aiming,
};

UENUM(BlueprintType)
enum class EInventorySlots : uint8
{
	MainWeaponSlot,
	SecondaryWeaponSlot,
};

UENUM(BlueprintType)
enum class EGameModes : uint8
{
	EliminateAsManyAsPossible,
};


USTRUCT(BlueprintType)
struct FProjectileSettings
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Class")
	TSubclassOf<ABaseProjectile> ProjectileClass;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Characteristics")
	float Damage = 10.0f;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Characteristics")
	float InitSpeed = 10.0f;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Characteristics")
	float MaxSpeed = 10.0f;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Characteristics")
	float MaxStrikingDistance = 2000.0f;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mesh")
	USkeletalMesh* SkeletalMesh = nullptr;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mesh")
	UStaticMesh* StaticMesh = nullptr;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Visual")
	UParticleSystem* ProjectileTrace = nullptr;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Visual")
	TMap<TEnumAsByte<EPhysicalSurface>, USoundBase*> HitSounds;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Visual")
	TMap<TEnumAsByte<EPhysicalSurface>, UParticleSystem*> HitFXs;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Visual")
	TMap<TEnumAsByte<EPhysicalSurface>, UMaterialInterface*> HitDecals;
};

USTRUCT(BlueprintType)
struct FWeaponSettings : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Class")
	TSubclassOf<ABaseWeapon> WeaponClass;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Characteristics")
	int32 MaxAmmoAmount = 10;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Characteristics")
	float ReloadTime = 0.0f;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Characteristics")
	float FireRate = 0.1f;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Characteristics")
	int32 NumberProjectilesByShot = 1;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Characteristics")
	float HipRecoil = 0.f;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Characteristics")
	float AimRecoil = 0.f;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Characteristics")
	EAmmoTypes AmmoType = EAmmoTypes::RifleType;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Characteristics")
	bool bIsWeaponMain = true;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Projectile")
	FProjectileSettings ProjectileSettings;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mesh")
	USkeletalMesh* WeaponSkeletalMesh = nullptr;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mesh")
	UStaticMesh* WeaponStaticMesh = nullptr;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mesh")
	USkeletalMesh* ClipSkeletalMesh = nullptr;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mesh")
	UStaticMesh* ClipStaticMesh = nullptr;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mesh")
	USkeletalMesh* SleeveSkeletalMesh = nullptr;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mesh")
	UStaticMesh* SleeveStaticMesh = nullptr;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation")
	UAnimMontage* CharacterAimAnim = nullptr;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation")
	UAnimMontage* CharacterHipFireAnim = nullptr;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation")
	UAnimMontage* CharacterAimFireAnim = nullptr;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation")
	UAnimMontage* CharacterReloadAnim = nullptr;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation")
	UAnimMontage* CharacterSwithWeaponAnim = nullptr;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Locations")
	FTransform ShootLocation = FTransform();
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Locations")
	FTransform SleeveLocation = FTransform();
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Locations")
	FTransform DropClipLocation = FTransform();
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Locations")
	FVector AimCameraLocationWithoutSight = FVector(0);
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Locations")
	FVector WeaponLocationInAim = FVector(0);
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Locations")
	FTransform SightLocation = FTransform();
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dispersion")
	float MinStandDispersion = 0.0f;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dispersion")
	float CoefDispersionWhileAiming = 0.0f;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dispersion")
	float MaxStandDispersion = 0.0f;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dispersion")
	float DispersionReductionWhileStanding = 0.0f;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dispersion")
	float DispersionReductionWhileMoving = 0.0f;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dispersion")
	float ShootDispersion = 0.0f;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dispersion")
	float IncreaseDispersionInMove = 0.0f;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dispersion")
	float MoveDispersion = 0.0f;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dispersion")
	float MaxMoveDispersion = 0.0f;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dispersion")
	float MinMoveDispersion = 0.0f;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Sounds")
	USoundBase* FireSound = nullptr;	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Sounds")
	USoundBase* ReloadSound = nullptr;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "FXs")
	UParticleSystem* FireFX = nullptr;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Socket")
	FName SocketNameToAttach = NAME_None;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Image")
	UTexture2D* Image = nullptr;

};

USTRUCT(BlueprintType)
struct FWeaponSlot
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName WeaponName = NAME_None;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 AmmoAmount = 0;
};

USTRUCT(BlueprintType)
struct FWeaponTuningSlot : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FName> TuningNames;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName WeaponName;
};

USTRUCT(BlueprintType)
struct FAmmoSlot
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EAmmoTypes AmmoType = EAmmoTypes::RifleType;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 ClipAmount = 0;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 MaxClipAmount = 0;
};
USTRUCT(BlueprintType)
struct FCharacterMovementSpeedByRotation
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MinDegree = 0.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MaxDegree = 0.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float CoefSpeed = 0.f;
};

USTRUCT(BlueprintType)
struct FGameModes : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Parameters")
	FName GameModeName = FName("");
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Parameters")
	int32 AmountPlayersInTeam = 0;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Parameters")
	int32 SumPointsToWin = 0;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Parameters")
	bool bCanKillsAffectPoints = true;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Parameters")
	float PlayerRestartTime = 0.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Widget")
	UTexture2D* Image = nullptr;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Widget")
	FText Description = FText();
};

USTRUCT(BlueprintType)
struct FMaps : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Parameters")
	FName MapName = FName();
};

USTRUCT(BlueprintType)
struct FInventoryForPlayers
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName PlayerName;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FWeaponSlot> WeaponSlots;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FWeaponTuningSlot> WeaponTuningSlots;
};

USTRUCT(BlueprintType)
struct FSessionInfo
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite)
	int32 CurrentPlayers = 0;
	UPROPERTY(BlueprintReadWrite)
	FString GameModeName = FString();
	UPROPERTY(BlueprintReadWrite)
	FString MapName = FString();
	//UPROPERTY(BlueprintReadWrite)
	//FName SessionName = FName();
};

USTRUCT(BlueprintType)
struct FWeaponTuningInfo : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UStaticMesh* StaticMesh = nullptr;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Parameters")
	ETuningTypes TuningType = ETuningTypes::Sight;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Parameters")
	TArray<FName> SuitableWeapons;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Parameters")
	UTexture2D* Image = nullptr;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sight")
	bool bIsOptical = false;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sight")
	float FieldOfView = 70.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sight")
	FVector CameraRelativeLocation = FVector(0);
};
