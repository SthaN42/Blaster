// Copyright Nathan Guerin

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PickupSpawnPoint.generated.h"

class APickup;

UCLASS()
class BLASTER_API APickupSpawnPoint : public AActor
{
	GENERATED_BODY()
	
public:	
	APickupSpawnPoint();

protected:
	virtual void BeginPlay() override;
	
	void SpawnPickup();

	UFUNCTION()
	void StartSpawnPickupTimer(AActor* DestroyedActor);

	void SpawnTimerFinished();

	UPROPERTY(EditAnywhere, Category = "PickupSpawnPoint")
	TArray<TSubclassOf<APickup>> PickupClasses;

	UPROPERTY()
	TObjectPtr<APickup> SpawnedPickup;

private:
	FTimerHandle SpawnPickupTimer;

	UPROPERTY(EditAnywhere, Category = "PickupSpawnPoint")
	float SpawnPickupTimeMin = 2.f;
	
	UPROPERTY(EditAnywhere, Category = "PickupSpawnPoint")
	float SpawnPickupTimeMax = 5.f;
};
