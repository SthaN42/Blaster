// Copyright Nathan Guerin

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "LagCompensationComponent.generated.h"

class AWeapon;
class ABlasterCharacter;
class ABlasterPlayerController;

USTRUCT(BlueprintType)
struct FCapsuleInfo
{
	GENERATED_BODY()

	UPROPERTY()
	FVector Location = FVector::ZeroVector;

	UPROPERTY()
	FRotator Rotation = FRotator::ZeroRotator;

	UPROPERTY()
	float HalfHeight = 0.f;

	UPROPERTY()
	float Radius = 0.f;
};

// Only supports one capsule per bone in the physics asset
USTRUCT(BlueprintType)
struct FFramePackage
{
	GENERATED_BODY()

	UPROPERTY()
	float Time = 0.f;

	UPROPERTY()
	TMap<FName, FCapsuleInfo> HitBoxInfo;

	UPROPERTY()
	TObjectPtr<ABlasterCharacter> Character;
};

USTRUCT(BlueprintType)
struct FServerSideRewindResult
{
	GENERATED_BODY()

	UPROPERTY()
	bool bHitConfirmed = false;

	UPROPERTY()
	bool bWeakSpot = false;
};

USTRUCT(BlueprintType)
struct FShotgunServerSideRewindResult
{
	GENERATED_BODY()

	UPROPERTY()
	TMap<TObjectPtr<ABlasterCharacter>, uint32> BodyShots;

	UPROPERTY()
	TMap<TObjectPtr<ABlasterCharacter>, uint32> WeakSpotShots;
};

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class BLASTER_API ULagCompensationComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	ULagCompensationComponent();
	friend ABlasterCharacter;

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	void ShowFramePackage(const FFramePackage& Package, const FColor& Color, const bool bPersistent = false) const;

	/** HitScan version of Server-Side Rewind */
	FServerSideRewindResult ServerSideRewind(ABlasterCharacter* HitCharacter, const FVector_NetQuantize& TraceStart, const FVector_NetQuantize& HitLocation, const float HitTime) const;

	/** Shotgun version of Server-Side Rewind */
	FShotgunServerSideRewindResult ServerSideRewind(const TArray<ABlasterCharacter*>& HitCharacters, const FVector_NetQuantize& TraceStart, const TArray<FVector_NetQuantize>& HitLocations, const float HitTime) const;

	/** HitScan version of Score Request */
	UFUNCTION(Server, Reliable)
	void ServerScoreRequest(ABlasterCharacter* HitCharacter, const FVector_NetQuantize& TraceStart, const FVector_NetQuantize& HitLocation, const float HitTime) const;

	/** Shotgun version of Score Request */
	UFUNCTION(Server, Reliable)
	void ServerShotgunScoreRequest(const TArray<ABlasterCharacter*>& HitCharacters, const FVector_NetQuantize& TraceStart, const TArray<FVector_NetQuantize>& HitLocations, const float HitTime) const;

protected:
	virtual void BeginPlay() override;

	void SaveFramePackage();

	void SaveFramePackage(FFramePackage& Package);

	static FFramePackage InterpBetweenFrames(const FFramePackage& OlderFrame, const FFramePackage& YoungerFrame, const float HitTime);

	/** HitScan version of ConfirmHit, used in the HitScan version of SSR */
	FServerSideRewindResult ConfirmHit(const FFramePackage& Package, ABlasterCharacter* HitCharacter, const FVector_NetQuantize& TraceStart, const FVector_NetQuantize& HitLocation) const;

	/** Shotgun version of ConfirmHit, used in the Shotgun version of SSR */
	FShotgunServerSideRewindResult ConfirmHit(const TArray<FFramePackage>& Packages, const FVector_NetQuantize& TraceStart, const TArray<FVector_NetQuantize>& HitLocations) const;

	static void CacheCapsulePositions(const ABlasterCharacter* HitCharacter, FFramePackage& OutFramePackage);

	static void MoveBoxes(const ABlasterCharacter* HitCharacter, const FFramePackage& Package, const bool bEnableCollision);

	static FFramePackage GetFrameToCheck(ABlasterCharacter* HitCharacter, const float HitTime);

private:
	UPROPERTY()
	TObjectPtr<ABlasterCharacter> Character;

	UPROPERTY()
	TObjectPtr<ABlasterPlayerController> Controller;

	TDoubleLinkedList<FFramePackage> FrameHistory;

	UPROPERTY(EditDefaultsOnly, Category = "Lag Compensation|Server-Side Rewind")
	float MaxRecordTime = 2.f;
};
