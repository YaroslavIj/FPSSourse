// Fill out your copyright notice in the Description page of Project Settings.


#include "HealthComponent.h"
#include "Net/UnrealNetwork.h"

// Sets default values for this component's properties
UHealthComponent::UHealthComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}

void UHealthComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(UHealthComponent, Health);
}

// Called when the game starts
void UHealthComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...
	
}


// Called every frame
void UHealthComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

float UHealthComponent::GetCurrentHealth()
{
	return Health;
}

bool UHealthComponent::GetIsAlive()
{
	return bIsAlive;
}


void UHealthComponent::ChangeCurrentHealth_OnServer_Implementation(float Value)
{
	Value *= CoefDamage;
	if(Health + Value <= 100.f && Health + Value >= 0)
	{
		Health += Value;
		GEngine->AddOnScreenDebugMessage(-1, 3, FColor::Yellow, FString::Printf(TEXT("Damage: %f"), Value));
	}
	else
	{
		if (Health + Value > 100)
		{
			Health = 100;
		}
		else if(Health + Value < 0)
		{
			Health = 0;
		}
	}
	OnHealthChange.Broadcast(GetCurrentHealth(), Value);
	if (Health <= 0 && bIsAlive)
	{
		bIsAlive = false;
		OnDead.Broadcast();
	}
}
