// Copyright Nathan Guerin


#include "LagCompensationComponent.h"

#include "Blaster/Character/BlasterCharacter.h"
#include "PhysicsEngine/PhysicsAsset.h"

ULagCompensationComponent::ULagCompensationComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void ULagCompensationComponent::BeginPlay()
{
	Super::BeginPlay();

	ConstructInitialCapsuleInfo();
}

void ULagCompensationComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

void ULagCompensationComponent::ConstructInitialCapsuleInfo()
{
	if (Character == nullptr || Character->GetMesh() == nullptr || Character->GetMesh()->GetPhysicsAsset() == nullptr) return;
	
	for (const TObjectPtr<USkeletalBodySetup>& BodySetup : Character->GetMesh()->GetPhysicsAsset()->SkeletalBodySetups)
	{
		const FName BoneName = BodySetup->BoneName;
		// const FTransform BoneWorldTransform = Character->GetMesh()->GetBoneTransform(Character->GetMesh()->GetBoneIndex(BoneName));
		for (const auto& SphylElem : BodySetup->AggGeom.SphylElems)
		{
			const FTransform LocTransform = SphylElem.GetTransform();
			// const FTransform WorldTransform = LocTransform * BoneWorldTransform;

			FCapsuleInfo Capsule;
			Capsule.BoneName = BoneName;
			Capsule.BoneIndex = Character->GetMesh()->GetBoneIndex(BoneName);
			// For the initial info, we need the local transform (physic body's transform)
			Capsule.BoneWorldTransform = LocTransform;
			Capsule.HalfHeight = SphylElem.Length / 2.f + SphylElem.Radius;
			Capsule.Radius = SphylElem.Radius;
			
			InitialCapsuleInfo.Add(Capsule);
		}
	}
}
