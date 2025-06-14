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

private:
	UPROPERTY(EditDefaultsOnly, Category = "WeaponProperties")
	float Damage = 20.f;

	UPROPERTY(EditAnywhere, Category = "WeaponProperties|Sounds")
	TObjectPtr<USoundBase> HitSound;

	UPROPERTY(EditAnywhere, Category = "WeaponProperties|Effects")
	TObjectPtr<UParticleSystem> ImpactParticles;
	
	UPROPERTY(EditAnywhere, Category = "WeaponProperties|Effects")
	TObjectPtr<UParticleSystem> BeamParticles;
};
