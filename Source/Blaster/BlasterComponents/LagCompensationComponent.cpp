// Copyright Nathan Guerin


#include "LagCompensationComponent.h"

#include "Blaster/Character/BlasterCharacter.h"
#include "Components/CapsuleComponent.h"

namespace LagCompensationCVars
{
	static bool ShowFrameHistory = false;
	static FAutoConsoleVariableRef CVarShowFrameHistory(
		TEXT("LagCompensation.ShowFrameHistory"),
		ShowFrameHistory,
		TEXT("When true, will display recorded frames for server-side rewind."),
		ECVF_Default | ECVF_Cheat);
}

ULagCompensationComponent::ULagCompensationComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void ULagCompensationComponent::BeginPlay()
{
	Super::BeginPlay();
}

void ULagCompensationComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (FrameHistory.Num() <= 1)
	{
		FFramePackage ThisFrame;
		SaveFramePackage(ThisFrame);
		FrameHistory.AddHead(ThisFrame);
	}
	else
	{
		float HistoryLength = FrameHistory.GetHead()->GetValue().Time - FrameHistory.GetTail()->GetValue().Time;
		while (HistoryLength > MaxRecordTime)
		{
			FrameHistory.RemoveNode(FrameHistory.GetTail());
			HistoryLength = FrameHistory.GetHead()->GetValue().Time - FrameHistory.GetTail()->GetValue().Time;
		}
		FFramePackage ThisFrame;
		SaveFramePackage(ThisFrame);
		FrameHistory.AddHead(ThisFrame);

		if (LagCompensationCVars::ShowFrameHistory) ShowFramePackage(ThisFrame, FColor::Red);
	}
}

void ULagCompensationComponent::SaveFramePackage(FFramePackage& Package)
{
	if (!Character) Character = Cast<ABlasterCharacter>(GetOwner());
	if (Character && Character->GetMesh())
	{
		Package.Time = GetWorld()->GetTimeSeconds();
		for (const TPair<FName, TObjectPtr<UCapsuleComponent>>& CapsulePair : Character->HitCollisionCapsules)
		{
			FCapsuleInfo CapsuleInfo;
			CapsuleInfo.Location = CapsulePair.Value->GetComponentLocation();
			CapsuleInfo.Rotation = CapsulePair.Value->GetComponentRotation();
			CapsuleInfo.HalfHeight = CapsulePair.Value->GetScaledCapsuleHalfHeight();
			CapsuleInfo.Radius = CapsulePair.Value->GetScaledCapsuleRadius();
			
			Package.HitBoxInfo.Add(CapsulePair.Key, CapsuleInfo);
		}
	}
}

void ULagCompensationComponent::ShowFramePackage(const FFramePackage& Package, const FColor& Color, bool bPersistent)
{
	for (const TPair<FName, FCapsuleInfo>& CapsuleInfo : Package.HitBoxInfo)
	{
		DrawDebugCapsule(GetWorld(),
			CapsuleInfo.Value.Location,
			CapsuleInfo.Value.HalfHeight,
			CapsuleInfo.Value.Radius,
			CapsuleInfo.Value.Rotation.Quaternion(),
			Color,
			bPersistent,
			MaxRecordTime);
	}
}

void ULagCompensationComponent::ServerSideRewind(ABlasterCharacter* HitCharacter,
	const FVector_NetQuantize& TraceStart, const FVector_NetQuantize& HitLocation, float HitTime)
{
	if (HitCharacter == nullptr ||
		HitCharacter->GetLagCompensation() == nullptr ||
		HitCharacter->GetLagCompensation()->FrameHistory.GetHead() == nullptr ||
		HitCharacter->GetLagCompensation()->FrameHistory.GetTail() == nullptr)
		return;

	// Frame Package that we check to verify a hit
	FFramePackage FrameToCheck;

	bool bShouldInterpolate = true;

	// Frame history of the HitCharacter
	const TDoubleLinkedList<FFramePackage>& History = HitCharacter->GetLagCompensation()->FrameHistory;

	const float OldestHistoryTime = History.GetTail()->GetValue().Time;
	const float NewestHistoryTime = History.GetHead()->GetValue().Time;

	if (OldestHistoryTime > HitTime)
	{
		// Too far back - too laggy to perform SSR
		return;
	}
	if (OldestHistoryTime == HitTime)
	{
		FrameToCheck = History.GetTail()->GetValue();
		bShouldInterpolate = false;
	}
	if (NewestHistoryTime <= HitTime)
	{
		FrameToCheck = History.GetHead()->GetValue();
		bShouldInterpolate = false;
	}

	TDoubleLinkedList<FFramePackage>::TDoubleLinkedListNode* YoungerFrame = History.GetHead();
	TDoubleLinkedList<FFramePackage>::TDoubleLinkedListNode* OlderFrame = YoungerFrame;

	while (OlderFrame->GetValue().Time > HitTime) // Is Older still younger than HitTime?
	{
		// March back until OlderFrame.Time < HitTime < YoungerFrame.Time
		if (OlderFrame->GetNextNode() == nullptr) break;
		OlderFrame = OlderFrame->GetNextNode();
		if (OlderFrame->GetValue().Time > HitTime)
		{
			YoungerFrame = OlderFrame;
		}
	}
	
	if (OlderFrame->GetValue().Time == HitTime) // Highly unlikely, but we found our frame to check
	{
		FrameToCheck = OlderFrame->GetValue();
		bShouldInterpolate = false;
	}

	if (bShouldInterpolate)
	{
		// Interpolate between YoungerFrame and OlderFrame
	}
}

FFramePackage ULagCompensationComponent::InterpBetweenFrames(const FFramePackage& OlderFrame,
	const FFramePackage& YoungerFrame, float HitTime)
{
	const float Distance = YoungerFrame.Time - OlderFrame.Time;
	const float InterpFraction = FMath::Clamp((HitTime - OlderFrame.Time) / Distance, 0.f, 1.f);

	FFramePackage InterpFramePackage;
	InterpFramePackage.Time = HitTime;

	for (const TPair<FName, FCapsuleInfo>& YoungerPair : YoungerFrame.HitBoxInfo)
	{
		const FCapsuleInfo& OlderInfo = OlderFrame.HitBoxInfo[YoungerPair.Key];
		const FCapsuleInfo& YoungerInfo = YoungerFrame.HitBoxInfo[YoungerPair.Key];

		FCapsuleInfo InterpCapsuleInfo;

		InterpCapsuleInfo.Location = FMath::Lerp(OlderInfo.Location, YoungerInfo.Location, HitTime);
		InterpCapsuleInfo.Rotation = FMath::Lerp(OlderInfo.Rotation, YoungerInfo.Rotation, HitTime);
		InterpCapsuleInfo.HalfHeight = YoungerInfo.HalfHeight;
		InterpCapsuleInfo.Radius = YoungerInfo.Radius;

		InterpFramePackage.HitBoxInfo.Add(YoungerPair.Key, InterpCapsuleInfo);
	}
	
	return InterpFramePackage;
}
