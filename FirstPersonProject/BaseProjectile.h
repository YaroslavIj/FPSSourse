// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "MyFunctionLibrary.h"
//
#include "BaseProjectile.generated.h"

UCLASS()
class FIRSTPERSONPROJECT_API ABaseProjectile : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ABaseProjectile();

	FProjectileSettings ProjectileSettings;

	//int32 TeamNumber = 0;
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Mesh")
	class UStaticMeshComponent* ProjectileMesh = nullptr;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Movement")
	class UProjectileMovementComponent* ProjectileMovement = nullptr;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	void InitProjectile(FProjectileSettings NewProjectileSettings);
	UFUNCTION(NetMulticast, Reliable)
	void InitVelocity_Multicast(float NewSpeed, float NewMaxSpeed);
	UFUNCTION(NetMulticast, Reliable)
	void SpawnParticleEffect_Multicast(UParticleSystem* FX);
	UFUNCTION(NetMulticast, Reliable)
	void InitMesh_Multicast(UStaticMesh* Mesh);
	UFUNCTION(Server, Reliable)
	void HitReaction_OnServer(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);
	UFUNCTION(NetMulticast, Reliable)
	void SpawnHitSound_Multicast(USoundBase* Sound, FHitResult Hit);
	UFUNCTION(NetMulticast, Reliable)
	void SpawnHitDecal_Multicast(UMaterialInterface* DecalMaterial, UPrimitiveComponent* OtherComp, FHitResult Hit);
	
	UFUNCTION(NetMulticast, Reliable)
	void SpawnHitFX_Multicast(UParticleSystem* FX, FHitResult Hit);
};
