// Copyright Nathan Guerin


#include "PickupSpawnPoint.h"

#include "Pickup.h"

APickupSpawnPoint::APickupSpawnPoint()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;
}

void APickupSpawnPoint::BeginPlay()
{
	Super::BeginPlay();

	StartSpawnPickupTimer(nullptr);
}

void APickupSpawnPoint::SpawnPickup()
{
	if (PickupClasses.Num() > 0)
	{
		const int32 Selection = FMath::RandRange(0, PickupClasses.Num() - 1);
		SpawnedPickup = GetWorld()->SpawnActor<APickup>(PickupClasses[Selection], GetActorTransform());

		if (SpawnedPickup && HasAuthority())
		{
			SpawnedPickup->OnDestroyed.AddDynamic(this, &ThisClass::StartSpawnPickupTimer);
		}
	}
}

void APickupSpawnPoint::StartSpawnPickupTimer(AActor* DestroyedActor)
{
	const float SpawnTime = FMath::FRandRange(SpawnPickupTimeMin, SpawnPickupTimeMax);
	GetWorldTimerManager().SetTimer(SpawnPickupTimer, this, &ThisClass::SpawnTimerFinished, SpawnTime);
}

void APickupSpawnPoint::SpawnTimerFinished()
{
	if (HasAuthority())
	{
		SpawnPickup();
	}
}
