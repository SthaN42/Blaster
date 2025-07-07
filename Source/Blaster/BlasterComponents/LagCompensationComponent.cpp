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

	FFramePackage Package;
	SaveFramePackage(Package);
	ShowFramePackage(Package, FColor::Red, true);
}

void ULagCompensationComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	FFramePackage Package;
	SaveFramePackage(Package);
	ShowFramePackage(Package, FColor::Orange);
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

void ULagCompensationComponent::ShowFramePackage(const FFramePackage& Package, const FColor& Color, bool bPersistent)
{
	for (const FCapsuleInfo& CapsuleInfo : Package.HitBoxInfo)
	{
		DrawDebugCapsule(GetWorld(),
			CapsuleInfo.BoneWorldTransform.GetLocation(),
			CapsuleInfo.HalfHeight,
			CapsuleInfo.Radius,
			CapsuleInfo.BoneWorldTransform.GetRotation(),
			Color,
			bPersistent,
			MaxRecordTime);
	}
}

void ULagCompensationComponent::SaveFramePackage(FFramePackage& Package)
{
	checkf(!InitialCapsuleInfo.IsEmpty(), TEXT("InitialCapsuleInfo is empty! Call ConstructInitialCapsuleInfo() first, and make sure the skeletal mesh of your character has a valid physics assets with physics bodies (only Capsules are supported)."))
	
	if (!Character) Character = Cast<ABlasterCharacter>(GetOwner());
	if (Character && Character->GetMesh())
	{
		Package.Time = GetWorld()->GetTimeSeconds();
		for (const FCapsuleInfo& InitialCapsule : InitialCapsuleInfo)
		{
			FCapsuleInfo CapsuleInfo;
			CapsuleInfo.BoneName = InitialCapsule.BoneName;
			CapsuleInfo.BoneIndex = InitialCapsule.BoneIndex;
			// Local Bone Transform (physic body's transform) * World Bone Transform
			CapsuleInfo.BoneWorldTransform = InitialCapsule.BoneWorldTransform * Character->GetMesh()->GetBoneTransform(InitialCapsule.BoneIndex);
			CapsuleInfo.HalfHeight = InitialCapsule.HalfHeight;
			CapsuleInfo.Radius = InitialCapsule.Radius;
			
			Package.HitBoxInfo.Add(CapsuleInfo);
		}
	}
}
