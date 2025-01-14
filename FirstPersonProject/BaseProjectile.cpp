
#include "BaseProjectile.h"
#include "Kismet/GameplayStatics.h"
#include "MyFunctionLibrary.h"
#include "GameFramework/Character.h"
#include "FPS_PlayerController.h"



// Sets default values
ABaseProjectile::ABaseProjectile()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	ProjectileMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ProjectileMesh"));
	ProjectileMesh->SetupAttachment(RootComponent);
	ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovement"));
	ProjectileMovement->UpdatedComponent = RootComponent;
	ProjectileMovement->bRotationFollowsVelocity = true;
	ProjectileMovement->ProjectileGravityScale = 0.1f;

	ProjectileMesh->SetCollisionProfileName(FName("Projectile"));
	ProjectileMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_EngineTraceChannel1, ECollisionResponse::ECR_Ignore);
	ProjectileMesh->OnComponentHit.AddDynamic(this, &ABaseProjectile::HitReaction_OnServer);
	ProjectileMesh->MoveIgnoreActors.Add(GetOwner());
		
	SetReplicates(true);
}

// Called when the game starts or when spawned
void ABaseProjectile::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ABaseProjectile::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void ABaseProjectile::InitProjectile(FProjectileSettings NewProjectileSettings)
{
	ProjectileSettings = NewProjectileSettings;
	InitVelocity_Multicast(NewProjectileSettings.InitSpeed, NewProjectileSettings.MaxSpeed);
	SpawnParticleEffect_Multicast(ProjectileSettings.ProjectileTrace);
	ProjectileMesh->MoveIgnoreActors.Add(GetInstigator());
	InitMesh_Multicast(ProjectileSettings.StaticMesh);
}

void ABaseProjectile::SpawnHitSound_Multicast_Implementation(USoundBase* Sound, FHitResult Hit)
{
	UGameplayStatics::PlaySoundAtLocation(GetWorld(), Sound, Hit.ImpactPoint);
}

void ABaseProjectile::SpawnHitDecal_Multicast_Implementation(UMaterialInterface* DecalMaterial, UPrimitiveComponent* OtherComp, FHitResult Hit)
{
	UGameplayStatics::SpawnDecalAttached(DecalMaterial, FVector(10), OtherComp, NAME_None, Hit.ImpactPoint, Hit.ImpactNormal.Rotation(), EAttachLocation::KeepWorldPosition, 10.f);
}

void ABaseProjectile::SpawnHitFX_Multicast_Implementation(UParticleSystem* FX, FHitResult Hit)
{
	UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), FX, FTransform(Hit.ImpactNormal.Rotation(), Hit.ImpactPoint, FVector(0.5f)));

}

void ABaseProjectile::HitReaction_OnServer_Implementation(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	if(OtherActor && Hit.Component->GetMaterial(0) && Hit.Component->GetMaterial(0)->GetPhysicalMaterial() && Hit.Component->GetMaterial(0)->GetPhysicalMaterial()->SurfaceType)
	{
		EPhysicalSurface Surface = Hit.Component->GetMaterial(0)->GetPhysicalMaterial()->SurfaceType;
		if (ProjectileSettings.HitSounds.Contains(Surface))
		{
			USoundBase* Sound = ProjectileSettings.HitSounds[Surface];
			if (Sound)
			{
				SpawnHitSound_Multicast(Sound, Hit);
			}
		}
		if (ProjectileSettings.HitDecals.Contains(Surface))
		{
			UMaterialInterface* Material = ProjectileSettings.HitDecals[Surface];
			if (Material && OtherComp)
			{
				SpawnHitDecal_Multicast(Material, OtherComp, Hit);
			}
		}
		if (ProjectileSettings.HitFXs.Contains(Surface))
		{
			UParticleSystem* FX = ProjectileSettings.HitFXs[Surface];
			if (FX)
			{
				SpawnHitFX_Multicast(FX, Hit);
				//UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), FX, FTransform(Hit.ImpactNormal.Rotation(), Hit.ImpactPoint, FVector(0.5f)));
			}
		}
	}
	ACharacter* Char = Cast<ACharacter>(OtherActor);
	if (Char && Char->GetController())
	{
		AFPS_PlayerController* TargetPC = Cast<AFPS_PlayerController>(Char->GetController());
		AFPS_PlayerController* SelfPC = Cast<AFPS_PlayerController>(GetInstigator()->GetController());
		if (TargetPC && SelfPC && TargetPC->TeamNumber != SelfPC->TeamNumber)
		{
			if(GetInstigator() && GetInstigator()->GetController())
			{
				UGameplayStatics::ApplyDamage(OtherActor, ProjectileSettings.Damage, GetInstigator()->GetController(), this, NULL);
			}
			else
			{
				UGameplayStatics::ApplyDamage(OtherActor, ProjectileSettings.Damage, nullptr, this, NULL);
			}
		}
	}
	else
	{
		if (GetInstigator() && GetInstigator()->GetController())
		{
			UGameplayStatics::ApplyDamage(OtherActor, ProjectileSettings.Damage, GetInstigator()->GetController(), this, NULL);
		}
		else
		{
			UGameplayStatics::ApplyDamage(OtherActor, ProjectileSettings.Damage, nullptr, this, NULL);
		}
	}
	
	
	Destroy();
	//SetLifeSpan(1);
}

void ABaseProjectile::SpawnParticleEffect_Multicast_Implementation(UParticleSystem* FX)
{
	UGameplayStatics::SpawnEmitterAttached(FX, ProjectileMesh, FName("None"), FVector(0), FRotator(0, -90, 0), EAttachLocation::SnapToTarget, true);
}

void ABaseProjectile::InitMesh_Multicast_Implementation(UStaticMesh* Mesh)
{
	ProjectileMesh->SetStaticMesh(Mesh);
}

void ABaseProjectile::InitVelocity_Multicast_Implementation(float NewSpeed, float NewMaxSpeed)
{
	if (ProjectileMovement)
	{
		ProjectileMovement->Velocity = GetActorForwardVector() * NewSpeed;
		ProjectileMovement->MaxSpeed = NewMaxSpeed;
	}
}
