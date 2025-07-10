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
	void WeaponTraceHit(const FVector& TraceStart, const FVector& HitTarget, FHitResult& OutHit) const;

	UPROPERTY(EditAnywhere, Category = "WeaponProperties|Sounds")
	TObjectPtr<USoundBase> HitSound;

	UPROPERTY(EditAnywhere, Category = "WeaponProperties|Effects")
	TObjectPtr<UParticleSystem> ImpactParticles;
	
private:
	UPROPERTY(EditAnywhere, Category = "WeaponProperties|Effects")
	TObjectPtr<UParticleSystem> BeamParticles;
};
