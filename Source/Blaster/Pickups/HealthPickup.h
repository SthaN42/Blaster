// Copyright Nathan Guerin

#pragma once

#include "CoreMinimal.h"
#include "Pickup.h"
#include "HealthPickup.generated.h"

class UNiagaraSystem;
class UNiagaraComponent;
/**
 * 
 */
UCLASS()
class BLASTER_API AHealthPickup : public APickup
{
	GENERATED_BODY()

public:
	AHealthPickup();

	virtual void Destroyed() override;

protected:
	virtual void OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult) override;

private:
	UPROPERTY(EditDefaultsOnly, Category = "Pickup|Health")
	float HealAmount = 100.f;

	UPROPERTY(EditDefaultsOnly, Category = "Pickup|Health")
	float HealingTime = 5.f;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UNiagaraComponent> PickupEffectComponent;

	UPROPERTY(EditDefaultsOnly, Category = "Pickup")
	TObjectPtr<UNiagaraSystem> PickupEffect;
};
