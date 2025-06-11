// Copyright Nathan Guerin


#include "ProjectileRocket.h"

#include "NiagaraComponent.h"

#include "Blaster/BlasterComponents/RocketMovementComponent.h"
#include "Components/AudioComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Components/BoxComponent.h"
#include "Sound/SoundCue.h"

AProjectileRocket::AProjectileRocket()
{
	ProjectileMesh = CreateDefaultSubobject<UStaticMeshComponent>("RocketMesh");
	ProjectileMesh->SetupAttachment(RootComponent);
	ProjectileMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	ProjectileMovementComponent = CreateDefaultSubobject<URocketMovementComponent>("RocketMovementComponent");
	ProjectileMovementComponent->bRotationFollowsVelocity = true;
	ProjectileMovementComponent->SetIsReplicated(true);
}

void AProjectileRocket::BeginPlay()
{
	Super::BeginPlay();

	if (!HasAuthority())
	{
		// ReSharper disable once CppBoundToDelegateMethodIsNotMarkedAsUFunction
		CollisionBox->OnComponentHit.AddDynamic(this, &ThisClass::OnHit);
	}

	SpawnTrailSystem();
	
	if (ProjectileSoundLoop)
	{
		ProjectileLoopSoundComponent = UGameplayStatics::SpawnSoundAttached(
			ProjectileSoundLoop, GetRootComponent(), FName(), GetActorLocation(),
			EAttachLocation::KeepWorldPosition, false, 1.f, 1.f, 0.f, nullptr, nullptr, false);
	}
}

void AProjectileRocket::Destroyed()
{
	// Empty on purpose
}

void AProjectileRocket::OnHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp,
                              FVector NormalImpulse, const FHitResult& Hit)
{
	if (OtherActor == GetOwner()) return;
	
	ApplyProjectileDamage();
	
	StartDestroyTimer();

	if (ImpactParticles)
	{
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ImpactParticles, GetActorTransform());
	}
	if (ImpactSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, ImpactSound, GetActorLocation());
	}
	if (ProjectileMesh)
	{
		ProjectileMesh->SetVisibility(false);
	}
	if (CollisionBox)
	{
		CollisionBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}
	if (TrailNiagaraComponent && TrailNiagaraComponent->IsActive())
	{
		TrailNiagaraComponent->Deactivate();
	}
	if (ProjectileLoopSoundComponent && ProjectileLoopSoundComponent->IsPlaying())
	{
		ProjectileLoopSoundComponent->Stop();
	}
}
