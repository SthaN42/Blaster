// Copyright Nathan Guerin

#pragma once

#include "CoreMinimal.h"
#include "Weapon.h"
#include "HitScanWeapon.generated.h"

/**
 * 
 */
UCLASS()
class BLASTER_API AHitScanWeapon : public AWeapon
{
	GENERATED_BODY()

public:
	virtual void Fire(const FVector& HitTarget) override;

protected:
	FVector TraceEndWithScatter(const FVector& TraceStart, const FVector& HitTarget) const;

private:
	UPROPERTY(EditDefaultsOnly, Category = "WeaponProperties")
	float Damage = 20.f;

	UPROPERTY(EditAnywhere, Category = "WeaponProperties|Sounds")
	TObjectPtr<USoundBase> HitSound;

	UPROPERTY(EditAnywhere, Category = "WeaponProperties|Effects")
	TObjectPtr<UParticleSystem> ImpactParticles;
	
	UPROPERTY(EditAnywhere, Category = "WeaponProperties|Effects")
	TObjectPtr<UParticleSystem> BeamParticles;

	/* Trace end with scatter */

	UPROPERTY(EditDefaultsOnly, Category = "WeaponProperties|Scatter")
	bool bUseScatter = false;
	
	UPROPERTY(EditDefaultsOnly, Category = "WeaponProperties|Scatter", meta = (EditCondition = "bUseScatter"))
	float DistanceToSphere = 800.f;

	UPROPERTY(EditDefaultsOnly, Category = "WeaponProperties|Scatter", meta = (EditCondition = "bUseScatter"))
	float SphereRadius = 75.f;
};
