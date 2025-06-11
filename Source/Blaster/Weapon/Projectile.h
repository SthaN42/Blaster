// Copyright Nathan Guerin

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Projectile.generated.h"

class UNiagaraComponent;
class UNiagaraSystem;
class USoundCue;
class UProjectileMovementComponent;
class UBoxComponent;

UCLASS()
class BLASTER_API AProjectile : public AActor
{
	GENERATED_BODY()
	
public:	
	AProjectile();
	virtual void Tick(float DeltaTime) override;

	virtual void Destroyed() override;

protected:
	virtual void BeginPlay() override;

	UFUNCTION()
	virtual void OnHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);

	void SpawnTrailSystem();

	void StartDestroyTimer();

	void DestroyTimerFinished();

	/**
	 * Calls the UGameplayStatics function according the parameters on the projectile
	 * 
	 * @param OtherActor If the projectile is set to NOT apply radial damage, this parameter is needed.
	 */
	void ApplyProjectileDamage(AActor* OtherActor = nullptr);

	/** Sets whether this projectile does simple or radial damage */
	UPROPERTY(EditDefaultsOnly, Category = "Projectile|Damage")
	bool bIsRadialDamage = false;

	/** The default amount of damage dealt */
	UPROPERTY(EditDefaultsOnly, Category = "Projectile|Damage")
	float BaseDamage = 20.f;

	/** Minimum amount of damage dealt (at the end of the falloff) */
	UPROPERTY(EditDefaultsOnly, Category = "Projectile|Damage|Radial Damage", meta = (EditCondition = "bIsRadialDamage"))
	float MinimumDamage = 10.f;

	/** The area of the full damage (BaseDamage), from the origin */
	UPROPERTY(EditDefaultsOnly, Category = "Projectile|Damage|Radial Damage", meta = (EditCondition = "bIsRadialDamage"))
	float DamageInnerRadius = 200.f;
	
	/** The area of the minimum damage (MinimumDamage), from the origin */
	UPROPERTY(EditDefaultsOnly, Category = "Projectile|Damage|Radial Damage", meta = (EditCondition = "bIsRadialDamage"))
	float DamageOuterRadius = 500.f;

	/** If this projectile should ignore its owner when applying damage */
	UPROPERTY(EditDefaultsOnly, Category = "Projectile|Damage|Radial Damage", meta = (EditCondition = "bIsRadialDamage"))
	bool bInflictSelfDamage = false;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UStaticMeshComponent> ProjectileMesh;

	UPROPERTY(EditAnywhere, Category = "Projectile|Effects")
	TObjectPtr<UParticleSystem> ImpactParticles;

	UPROPERTY(EditAnywhere, Category = "Projectile|Sounds")
	TObjectPtr<USoundCue> ImpactSound;

	UPROPERTY(EditAnywhere)
	TObjectPtr<UBoxComponent> CollisionBox;

	UPROPERTY(EditDefaultsOnly, Category = "Projectile|Effects")
	TObjectPtr<UNiagaraSystem> TrailNiagaraSystem;
	
	UPROPERTY()
	TObjectPtr<UNiagaraComponent> TrailNiagaraComponent;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UProjectileMovementComponent> ProjectileMovementComponent;

private:
	UPROPERTY(EditAnywhere, Category = "Projectile|Effects")
	TObjectPtr<UParticleSystem> Tracer;

	UPROPERTY()
	TObjectPtr<UParticleSystemComponent> TracerComponent;

	FTimerHandle DestroyTimer;

	UPROPERTY(EditDefaultsOnly, Category = "Projectile")
	float DestroyTime = 3.f;
};
