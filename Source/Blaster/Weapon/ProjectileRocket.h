// Copyright Nathan Guerin

#pragma once

#include "CoreMinimal.h"
#include "Projectile.h"
#include "ProjectileRocket.generated.h"

/**
 * 
 */
UCLASS()
class BLASTER_API AProjectileRocket : public AProjectile
{
	GENERATED_BODY()

public:
	AProjectileRocket();

protected:
	virtual void OnHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit) override;

	/** Minimum amount of damage dealt (at the end of the falloff) */
	UPROPERTY(EditDefaultsOnly, Category = "Damage|Radial")
	float MinimumDamage = 10.f;

	/** The area of the full damage (BaseDamage), from the origin  */
	UPROPERTY(EditDefaultsOnly, Category = "Damage|Radial")
	float DamageFalloffRadiusStart = 200.f;
	
	/** The area of the minimum damage (MinimumDamage), from the origin  */
	UPROPERTY(EditDefaultsOnly, Category = "Damage|Radial")
	float DamageFalloffRadiusEnd = 500.f;

private:
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UStaticMeshComponent> RocketMesh;
};
