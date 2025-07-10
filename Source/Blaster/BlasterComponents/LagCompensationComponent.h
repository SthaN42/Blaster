// Copyright Nathan Guerin

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "LagCompensationComponent.generated.h"

class ABlasterCharacter;
class ABlasterPlayerController;

USTRUCT(BlueprintType)
struct FCapsuleInfo
{
	GENERATED_BODY()

	UPROPERTY()
	FVector Location = FVector();

	UPROPERTY()
	FRotator Rotation = FRotator();

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

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class BLASTER_API ULagCompensationComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	ULagCompensationComponent();
	friend ABlasterCharacter;
	
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	void ShowFramePackage(const FFramePackage& Package, const FColor& Color, const bool bPersistent = false) const;

	FServerSideRewindResult ServerSideRewind(ABlasterCharacter* HitCharacter, const FVector_NetQuantize& TraceStart, const FVector_NetQuantize& HitLocation, const float HitTime) const;

protected:
	virtual void BeginPlay() override;

	void SaveFramePackage(FFramePackage& Package);

	static FFramePackage InterpBetweenFrames(const FFramePackage& OlderFrame, const FFramePackage& YoungerFrame, const float HitTime);

	FServerSideRewindResult ConfirmHit(const FFramePackage& Package, ABlasterCharacter* HitCharacter, const FVector_NetQuantize& TraceStart, const FVector_NetQuantize& HitLocation) const;

	void CacheCapsulePositions(const ABlasterCharacter* HitCharacter, FFramePackage& OutFramePackage) const;

	void MoveBoxes(const ABlasterCharacter* HitCharacter, const FFramePackage& Package, const bool bEnableCollision) const;
	
private:
	UPROPERTY()
	TObjectPtr<ABlasterCharacter> Character;
	
	UPROPERTY()
	TObjectPtr<ABlasterPlayerController> Controller;

	TDoubleLinkedList<FFramePackage> FrameHistory;

	UPROPERTY(EditDefaultsOnly, Category = "Lag Compensation|Server-Side Rewind")
	float MaxRecordTime = 2.f;
};
