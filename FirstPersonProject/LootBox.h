// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/ArrowComponent.h"
//
#include "LootBox.generated.h"

class APickUpAmmo;
class APickUpWeapon;
UCLASS()
class FIRSTPERSONPROJECT_API ALootBox : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ALootBox();

	FTimerHandle ReloadTimer;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	float ReloadTime = 5.f;

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<APickUpWeapon> PickUpWeaponClass;
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<APickUpAmmo> PickUpAmmoClass;

	UPROPERTY(BlueprintReadOnly)
	bool bCanOpenLootBox = true;
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Mesh")
	UStaticMeshComponent* MainMesh = nullptr;	
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Mesh")
	UStaticMeshComponent* ClosedMesh = nullptr;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Mesh")
	UStaticMeshComponent* OpenedMesh = nullptr;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Arrow")
	UArrowComponent* LootSpawnLocation = nullptr;
	
public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UFUNCTION(BlueprintCallable)
	void OpenLootBox();
	UFUNCTION()
	void EndReload();
	UFUNCTION(NetMulticast, Reliable)
	void OpenOrCloseLootBox_Multicast(bool bIsOpen);

};
