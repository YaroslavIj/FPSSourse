// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/StaticMeshActor.h"
#include "MyFunctionLibrary.h"
//
#include "PickUpItem.generated.h"


UCLASS()
class FIRSTPERSONPROJECT_API APickUpItem : public AStaticMeshActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	APickUpItem();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UFUNCTION(NetMulticast, Reliable)
	virtual void InitMesh_Multicast(UStaticMesh* NewMesh);

};

UCLASS()
class FIRSTPERSONPROJECT_API APickUpWeapon : public APickUpItem
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	APickUpWeapon();

	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	FWeaponSlot WeaponSlot;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	FWeaponTuningSlot TuningSlot;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Mesh")
	class UStaticMeshComponent* SightMesh = nullptr;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;
	UFUNCTION(BlueprintCallable)
	void InitItem(FWeaponSlot NewWeaponSlot, FWeaponTuningSlot NewTuningSlot);
	UFUNCTION(NetMulticast, Reliable)
	void InitTuningMesh_Multicast(UStaticMesh* NewMesh, FTransform NewTransform);

};

UCLASS()
class FIRSTPERSONPROJECT_API APickUpAmmo : public APickUpItem
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	APickUpAmmo();
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	FAmmoSlot AmmoSlot;
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	//UPROPERTY(EditDefaultsOnly)
	//TMap<EAmmoTypes, UStaticMesh*> StaticMeshesByAmmoTypes;

	UFUNCTION(BlueprintCallable)
	void InitItem(FAmmoSlot NewAmmoSlot);
};
