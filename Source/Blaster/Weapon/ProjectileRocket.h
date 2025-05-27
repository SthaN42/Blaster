// Copyright Nathan Guerin

#pragma once

#include "CoreMinimal.h"
#include "Projectile.h"
#include "ProjectileRocket.generated.h"

class UNiagaraSystem;
class UNiagaraComponent;
/**
 * 
 */
UCLASS()
class BLASTER_API AProjectileRocket : public AProjectile
{
	GENERATED_BODY()

public:
	AProjectileRocket();

	virtual void Destroyed() override;

protected:
	virtual void BeginPlay() override;

	virtual void OnHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit) override;

	void DestroyTimerFinished();

	UPROPERTY(EditDefaultsOnly, Category = "Projectile|Sounds")
	TObjectPtr<USoundBase> ProjectileSoundLoop;

	UPROPERTY()
	TObjectPtr<UAudioComponent> ProjectileLoopSoundComponent;

	UPROPERTY(EditDefaultsOnly, Category = "Projectile|Effects")
	TObjectPtr<UNiagaraSystem> TrailNiagaraSystem;
	
	UPROPERTY()
	TObjectPtr<UNiagaraComponent> TrailNiagaraComponent;

	/** The name of the socket on the projectile mesh from where the niagara trail component should be spawned from */
	UPROPERTY(EditDefaultsOnly, Category = "Projectile|Effects")
	FName TrailSpawnSocketName;

	/** Minimum amount of damage dealt (at the end of the falloff) */
	UPROPERTY(EditDefaultsOnly, Category = "Projectile|Damage|Radial")
	float MinimumDamage = 10.f;

	/** The area of the full damage (BaseDamage), from the origin  */
	UPROPERTY(EditDefaultsOnly, Category = "Projectile|Damage|Radial")
	float DamageFalloffRadiusStart = 200.f;
	
	/** The area of the minimum damage (MinimumDamage), from the origin  */
	UPROPERTY(EditDefaultsOnly, Category = "Projectile|Damage|Radial")
	float DamageFalloffRadiusEnd = 500.f;

private:
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UStaticMeshComponent> RocketMesh;

	FTimerHandle DestroyTimer;

	UPROPERTY(EditDefaultsOnly)
	float DestroyTime = 3.f;
};
