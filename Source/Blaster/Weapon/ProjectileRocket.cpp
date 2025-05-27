// Copyright Nathan Guerin


#include "ProjectileRocket.h"

#include "NiagaraComponent.h"

#include "NiagaraFunctionLibrary.h"
#include "Components/AudioComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Components/BoxComponent.h"
#include "Sound/SoundCue.h"

AProjectileRocket::AProjectileRocket()
{
	RocketMesh = CreateDefaultSubobject<UStaticMeshComponent>("RocketMesh");
	RocketMesh->SetupAttachment(RootComponent);
	RocketMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void AProjectileRocket::BeginPlay()
{
	Super::BeginPlay();

	if (!HasAuthority())
	{
		// ReSharper disable once CppBoundToDelegateMethodIsNotMarkedAsUFunction
		CollisionBox->OnComponentHit.AddDynamic(this, &ThisClass::OnHit);
	}

	if (TrailNiagaraSystem)
	{
		TrailNiagaraComponent = UNiagaraFunctionLibrary::SpawnSystemAttached(
			TrailNiagaraSystem, RocketMesh, TrailSpawnSocketName, GetActorLocation(), GetActorRotation(),
			EAttachLocation::KeepWorldPosition, false);
	}
	if (ProjectileSoundLoop)
	{
		ProjectileLoopSoundComponent = UGameplayStatics::SpawnSoundAttached(
			ProjectileSoundLoop, GetRootComponent(), TrailSpawnSocketName, GetActorLocation(),
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
	const APawn* FiringPawn = GetInstigator();
	if (HasAuthority() && FiringPawn)
	{
		if (AController* FiringController = FiringPawn->GetController())
		{
			UGameplayStatics::ApplyRadialDamageWithFalloff(this, BaseDamage, MinimumDamage, GetActorLocation(),
			                                               DamageFalloffRadiusStart, DamageFalloffRadiusEnd, 1.f,
			                                               UDamageType::StaticClass(), TArray<AActor*>(), this,
			                                               FiringController);
		}
	}

	if (ImpactParticles)
	{
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ImpactParticles, GetActorTransform());
	}
	if (ImpactSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, ImpactSound, GetActorLocation());
	}
	if (RocketMesh)
	{
		RocketMesh->SetVisibility(false);
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

	GetWorldTimerManager().SetTimer(DestroyTimer, this, &ThisClass::DestroyTimerFinished, DestroyTime);
}

void AProjectileRocket::DestroyTimerFinished()
{
	Destroy();
}
