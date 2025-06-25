// Copyright Nathan Guerin

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Pickup.generated.h"

class UNiagaraSystem;
class UNiagaraComponent;
class USphereComponent;

UCLASS()
class BLASTER_API APickup : public AActor
{
	GENERATED_BODY()
	
public:	
	APickup();

	virtual void Tick(float DeltaSeconds) override;

	virtual void Destroyed() override;
	
protected:
	virtual void BeginPlay() override;

	UFUNCTION(BlueprintNativeEvent, Category = "Pickup")
	void OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UPROPERTY(EditDefaultsOnly, Category = "Pickup")
	float BaseTurnRate = 45.f;

private:
	UPROPERTY(EditAnywhere)
	TObjectPtr<USphereComponent> OverlapSphere;

	UPROPERTY(EditAnywhere)
	TObjectPtr<UStaticMeshComponent> PickupMesh;

	UPROPERTY(EditDefaultsOnly, Category = "Pickup")
	TObjectPtr<USoundBase> PickupSound;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UNiagaraComponent> PickupEffectComponent;

	UPROPERTY(EditDefaultsOnly, Category = "Pickup")
	TObjectPtr<UNiagaraSystem> PickupEffect;
};
