// Copyright Nathan Guerin

#pragma once

#include "CoreMinimal.h"
#include "Pickup.h"
#include "AmmoPickup.generated.h"

enum class EWeaponType;
/**
 * 
 */
UCLASS()
class BLASTER_API AAmmoPickup : public APickup
{
	GENERATED_BODY()

protected:
	virtual void OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult) override;

private:
	UPROPERTY(EditDefaultsOnly, Category = "Pickup|Ammo")
	EWeaponType WeaponType;

	UPROPERTY(EditDefaultsOnly, Category = "Pickup|Ammo")
	int32 AmmoAmount = 60;
};
