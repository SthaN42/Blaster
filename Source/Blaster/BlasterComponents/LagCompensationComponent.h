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
	FName BoneName = FName();

	UPROPERTY()
	int32 BoneIndex = 0;

	UPROPERTY()
	FTransform BoneWorldTransform = FTransform();

	UPROPERTY()
	float HalfHeight = 0.f;

	UPROPERTY()
	float Radius = 0.f;
};

USTRUCT(BlueprintType)
struct FFramePackage
{
	GENERATED_BODY()

	UPROPERTY()
	float Time = 0.f;

	UPROPERTY()
	TArray<FCapsuleInfo> HitBoxInfo;
};

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class BLASTER_API ULagCompensationComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	ULagCompensationComponent();
	friend ABlasterCharacter;
	
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

protected:
	virtual void BeginPlay() override;

	void ConstructInitialCapsuleInfo();

private:
	UPROPERTY()
	TObjectPtr<ABlasterCharacter> Character;
	
	UPROPERTY()
	TObjectPtr<ABlasterPlayerController> Controller;

	UPROPERTY()
	TArray<FCapsuleInfo> InitialCapsuleInfo;
};
