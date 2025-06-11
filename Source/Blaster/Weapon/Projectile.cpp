// Copyright Nathan Guerin


#include "Projectile.h"

#include "NiagaraFunctionLibrary.h"
#include "Blaster/Blaster.h"
#include "Blaster/BlasterLogChannels.h"
#include "Components/BoxComponent.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundCue.h"

AProjectile::AProjectile()
{
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;

	CollisionBox = CreateDefaultSubobject<UBoxComponent>("CollisionBox");
	SetRootComponent(CollisionBox);
	CollisionBox->SetCollisionObjectType(ECC_WorldDynamic);
	CollisionBox->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	CollisionBox->SetCollisionResponseToAllChannels(ECR_Ignore);
	CollisionBox->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);
	CollisionBox->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Block);
	CollisionBox->SetCollisionResponseToChannel(ECC_SkeletalMesh, ECR_Block);
}

void AProjectile::BeginPlay()
{
	Super::BeginPlay();

	if (Tracer)
	{
		TracerComponent = UGameplayStatics::SpawnEmitterAttached(Tracer,
			CollisionBox,
			NAME_None,
			GetActorLocation(),
			GetActorRotation(),
			EAttachLocation::KeepWorldPosition
		);
	}

	if (HasAuthority())
	{
		CollisionBox->OnComponentHit.AddDynamic(this, &ThisClass::OnHit);
		CollisionBox->IgnoreActorWhenMoving(Owner, true);
	}
}

void AProjectile::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AProjectile::StartDestroyTimer()
{
	GetWorldTimerManager().SetTimer(DestroyTimer, this, &ThisClass::DestroyTimerFinished, DestroyTime);
}

void AProjectile::DestroyTimerFinished()
{
	Destroy();
}

void AProjectile::Destroyed()
{
	Super::Destroyed();

	if (ImpactParticles)
	{
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ImpactParticles, GetActorTransform());
	}
	if (ImpactSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, ImpactSound, GetActorLocation());
	}
}

void AProjectile::OnHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{	
	Destroy();
}

void AProjectile::ApplyProjectileDamage(AActor* OtherActor)
{
	if (bIsRadialDamage)
	{
		const APawn* FiringPawn = GetInstigator();
		if (HasAuthority() && FiringPawn)
		{
			if (AController* FiringController = FiringPawn->GetController())
			{
				TArray<AActor*> IgnoreActors;
				if (!bInflictSelfDamage && GetOwner())
				{
					IgnoreActors.Add(GetOwner());
				}
				UGameplayStatics::ApplyRadialDamageWithFalloff(this, BaseDamage, MinimumDamage, GetActorLocation(),
															   DamageInnerRadius, DamageOuterRadius, 1.f,
															   UDamageType::StaticClass(), IgnoreActors, this,
															   FiringController);
			}
		}
	}
	else
	{
		if (OtherActor == nullptr)
		{
			UE_LOG(LogBlaster, Warning, TEXT("ApplyProjectileDamage was called on a non-radial damage projectile without a valid OtherActor, exiting without applying any damage."));
			return;
		}
		
		if (const ACharacter* OwnerCharacter = Cast<ACharacter>(GetOwner()))
		{
			if (AController* OwnerController = OwnerCharacter->GetController())
			{
				UGameplayStatics::ApplyDamage(OtherActor, BaseDamage, OwnerController, this, UDamageType::StaticClass());
			}
		}
	}
}

void AProjectile::SpawnTrailSystem()
{
	if (TrailNiagaraSystem && ProjectileMesh)
	{
		TrailNiagaraComponent = UNiagaraFunctionLibrary::SpawnSystemAttached(
			TrailNiagaraSystem, ProjectileMesh, NAME_None, GetActorLocation(), GetActorRotation(),
			EAttachLocation::KeepWorldPosition, false);
	}
}
