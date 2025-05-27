// Copyright Nathan Guerin

#pragma once

#include "CoreMinimal.h"
#include "Weapon.h"
#include "ProjectileWeapon.generated.h"

class AProjectile;
/**
 * 
 */
UCLASS()
class BLASTER_API AProjectileWeapon : public AWeapon
{
	GENERATED_BODY()

public:
	virtual void Fire(const FVector& HitTarget) override;

private:
	UPROPERTY(EditAnywhere, Category = "WeaponProperties|Projectile")
	TSubclassOf<AProjectile> ProjectileClass;

	/** The name of the socket on the weapon skeletal mesh from where the projectile should be spawned from */
	UPROPERTY(EditDefaultsOnly, Category = "WeaponProperties|Projectile")
	FName ProjectileSpawnSocketName;
};
