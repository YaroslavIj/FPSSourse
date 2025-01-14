// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "HealthComponent.generated.h"


DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnDead);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnHealthChange, float, NewHealth, float, ChangeValue);

UCLASS(Blueprintable, ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class FIRSTPERSONPROJECT_API UHealthComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UHealthComponent();

	UPROPERTY(BlueprintReadWrite, BlueprintAssignable)
	FOnDead OnDead;
	UPROPERTY(BlueprintAssignable, BlueprintReadWrite)
	FOnHealthChange OnHealthChange;

	UPROPERTY(BlueprintReadWrite)
	AController* LastDamageInstigator;

	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	float CoefDamage = 1.f;


protected:
	// Called when the game starts
	virtual void BeginPlay() override;

	UPROPERTY(Replicated, BlueprintReadOnly)
	float Health = 100.f;
	UPROPERTY(Replicated)
	bool bIsAlive = true;
public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	UFUNCTION(BlueprintCallable)
	float GetCurrentHealth();
	UFUNCTION(Server, Reliable, BlueprintCallable)
	void ChangeCurrentHealth_OnServer(float Value);
	UFUNCTION(BlueprintCallable)
	bool GetIsAlive();
};
