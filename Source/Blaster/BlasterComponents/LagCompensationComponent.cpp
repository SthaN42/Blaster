// Copyright Nathan Guerin


#include "LagCompensationComponent.h"

#include "Blaster/Character/BlasterCharacter.h"
#include "Blaster/Physics/BlasterCollisionChannels.h"
#include "Blaster/Weapon/Weapon.h"
#include "Components/CapsuleComponent.h"
#include "Kismet/GameplayStatics.h"

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

	SaveFramePackage();
}

void ULagCompensationComponent::SaveFramePackage()
{
	if (Character == nullptr || !Character->HasAuthority()) return;

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
		Package.Character = Character;
	}
}

void ULagCompensationComponent::ShowFramePackage(const FFramePackage& Package, const FColor& Color, const bool bPersistent) const
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

void ULagCompensationComponent::ServerScoreRequest_Implementation(ABlasterCharacter* HitCharacter, const FVector_NetQuantize& TraceStart, const FVector_NetQuantize& HitLocation, const float HitTime) const
{
	const FServerSideRewindResult Confirm = ServerSideRewind(HitCharacter, TraceStart, HitLocation, HitTime);

	if (Character && HitCharacter && Character->GetEquippedWeapon() && Confirm.bHitConfirmed)
	{
		UGameplayStatics::ApplyDamage(HitCharacter, Character->GetEquippedWeapon()->GetCalculatedDamage(Confirm.bWeakSpot), Character->Controller, Character->GetEquippedWeapon(), UDamageType::StaticClass());
	}
}

void ULagCompensationComponent::ServerShotgunScoreRequest_Implementation(const TArray<ABlasterCharacter*>& HitCharacters, const FVector_NetQuantize& TraceStart, const TArray<FVector_NetQuantize>& HitLocations, const float HitTime) const
{
	const FShotgunServerSideRewindResult Confirm = ServerSideRewind(HitCharacters, TraceStart, HitLocations, HitTime);

	for (auto& HitCharacter : HitCharacters)
	{
		if (HitCharacter == nullptr || Character == nullptr || Character->GetEquippedWeapon() == nullptr) continue;
		
		float TotalDamage = 0.f;
		if (Confirm.BodyShots.Contains(HitCharacter))
		{
			TotalDamage += Confirm.BodyShots[HitCharacter] * Character->GetEquippedWeapon()->GetCalculatedDamage(false);
		}
		if (Confirm.WeakSpotShots.Contains(HitCharacter))
		{
			TotalDamage += Confirm.WeakSpotShots[HitCharacter] * Character->GetEquippedWeapon()->GetCalculatedDamage(true);
		}
		UGameplayStatics::ApplyDamage(HitCharacter, TotalDamage, Character->Controller, Character->GetEquippedWeapon(), UDamageType::StaticClass());
	}
}

FServerSideRewindResult ULagCompensationComponent::ServerSideRewind(ABlasterCharacter* HitCharacter,
	const FVector_NetQuantize& TraceStart, const FVector_NetQuantize& HitLocation, const float HitTime) const
{
	const FFramePackage FrameToCheck = GetFrameToCheck(HitCharacter, HitTime);
	return ConfirmHit(FrameToCheck, HitCharacter, TraceStart, HitLocation);
}

FShotgunServerSideRewindResult ULagCompensationComponent::ServerSideRewind(
	const TArray<ABlasterCharacter*>& HitCharacters, const FVector_NetQuantize& TraceStart,
	const TArray<FVector_NetQuantize>& HitLocations, const float HitTime) const
{
	TArray<FFramePackage> FramesToCheck;
	for (ABlasterCharacter* HitCharacter : HitCharacters)
	{
		FramesToCheck.Add(GetFrameToCheck(HitCharacter, HitTime));
	}
	return ConfirmHit(FramesToCheck, TraceStart, HitLocations);
}

FFramePackage ULagCompensationComponent::GetFrameToCheck(ABlasterCharacter* HitCharacter, const float HitTime)
{
	if (HitCharacter == nullptr ||
		HitCharacter->GetLagCompensation() == nullptr ||
		HitCharacter->GetLagCompensation()->FrameHistory.GetHead() == nullptr ||
		HitCharacter->GetLagCompensation()->FrameHistory.GetTail() == nullptr)
		return FFramePackage();

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
		return FFramePackage();
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
		FrameToCheck = InterpBetweenFrames(OlderFrame->GetValue(), YoungerFrame->GetValue(), HitTime);
	}
	FrameToCheck.Character = HitCharacter;
	return FrameToCheck;
}

FFramePackage ULagCompensationComponent::InterpBetweenFrames(const FFramePackage& OlderFrame,
	const FFramePackage& YoungerFrame, const float HitTime)
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

		InterpCapsuleInfo.Location = FMath::Lerp(OlderInfo.Location, YoungerInfo.Location, InterpFraction);
		InterpCapsuleInfo.Rotation = FMath::Lerp(OlderInfo.Rotation, YoungerInfo.Rotation, InterpFraction);
		InterpCapsuleInfo.HalfHeight = YoungerInfo.HalfHeight;
		InterpCapsuleInfo.Radius = YoungerInfo.Radius;

		InterpFramePackage.HitBoxInfo.Add(YoungerPair.Key, InterpCapsuleInfo);
	}

	return InterpFramePackage;
}

FServerSideRewindResult ULagCompensationComponent::ConfirmHit(const FFramePackage& Package,
	ABlasterCharacter* HitCharacter, const FVector_NetQuantize& TraceStart, const FVector_NetQuantize& HitLocation) const
{
	if (HitCharacter == nullptr) return FServerSideRewindResult();

	FFramePackage CurrentFrame;
	CacheCapsulePositions(HitCharacter, CurrentFrame);
	MoveBoxes(HitCharacter, Package, true);

	bool bSuccessfulHit = false, bWeakSpotHit = false;

	if (const UWorld* World = GetWorld())
	{
		FHitResult ConfirmHitResult;
		const FVector TraceEnd = TraceStart + (HitLocation - TraceStart) * 1.25f;

		FCollisionQueryParams CollisionParams;
		CollisionParams.bReturnPhysicalMaterial = true;
		World->LineTraceSingleByChannel(ConfirmHitResult, TraceStart, TraceEnd, ECC_HitBox, CollisionParams);

		bSuccessfulHit = ConfirmHitResult.bBlockingHit;
		if (ConfirmHitResult.PhysMaterial.Get())
		{
			bWeakSpotHit = ConfirmHitResult.PhysMaterial.Get()->SurfaceType == EPS_Player_WeakSpot;
		}
	}
	MoveBoxes(HitCharacter, CurrentFrame, false);

	return FServerSideRewindResult(bSuccessfulHit, bWeakSpotHit);
}

FShotgunServerSideRewindResult ULagCompensationComponent::ConfirmHit(const TArray<FFramePackage>& Packages,
	const FVector_NetQuantize& TraceStart, const TArray<FVector_NetQuantize>& HitLocations) const
{
	FShotgunServerSideRewindResult ShotgunResult;

	// Cache current positions and move hit characters capsules
	TArray<FFramePackage> CurrentFrames;
	for (const FFramePackage& Package : Packages)
	{
		FFramePackage CurrentFrame;
		CurrentFrame.Character = Package.Character;
		CacheCapsulePositions(Package.Character, CurrentFrame);
		MoveBoxes(Package.Character, Package, true);
		CurrentFrames.Add(CurrentFrame);
	}

	// Confirm hit for each hit location
	const UWorld* World = GetWorld();
	for (const FVector_NetQuantize& HitLocation : HitLocations)
	{
		if (World)
		{
			FHitResult ConfirmHitResult;
			const FVector TraceEnd = TraceStart + (HitLocation - TraceStart) * 1.25f;

			FCollisionQueryParams CollisionParams;
			CollisionParams.bReturnPhysicalMaterial = true;
			World->LineTraceSingleByChannel(ConfirmHitResult, TraceStart, TraceEnd, ECC_HitBox, CollisionParams);

			if (!ConfirmHitResult.bBlockingHit) continue;

			// Check is this hit was on a weak spot
			bool bWeakSpotHit = false;
			if (ConfirmHitResult.PhysMaterial.Get())
			{
				bWeakSpotHit = ConfirmHitResult.PhysMaterial.Get()->SurfaceType == EPS_Player_WeakSpot;
			}

			// Add the hit to the correct TMap on the result
			if (ABlasterCharacter* HitBlasterCharacter = Cast<ABlasterCharacter>(ConfirmHitResult.GetActor()))
			{
				if (bWeakSpotHit)
				{
					if (ShotgunResult.WeakSpotShots.Contains(HitBlasterCharacter))
					{
						ShotgunResult.WeakSpotShots[HitBlasterCharacter]++;
					}
					else
					{
						ShotgunResult.WeakSpotShots.Emplace(HitBlasterCharacter, 1);
					}
				}
				else
				{
					if (ShotgunResult.BodyShots.Contains(HitBlasterCharacter))
					{
						ShotgunResult.BodyShots[HitBlasterCharacter]++;
					}
					else
					{
						ShotgunResult.BodyShots.Emplace(HitBlasterCharacter, 1);
					}
				}
			}
		}
	}
	// Move back capsules to current positions
	for (const FFramePackage& CurrentFrame : CurrentFrames)
	{
		MoveBoxes(CurrentFrame.Character, CurrentFrame, false);
	}

	return ShotgunResult;
}

void ULagCompensationComponent::CacheCapsulePositions(const ABlasterCharacter* HitCharacter,
	FFramePackage& OutFramePackage)
{
	if (HitCharacter == nullptr) return;

	for (const TPair<FName, TObjectPtr<UCapsuleComponent>>& HitPair : HitCharacter->HitCollisionCapsules)
	{
		if (HitPair.Value != nullptr)
		{
			FCapsuleInfo CapsuleInfo;
			CapsuleInfo.Location = HitPair.Value->GetComponentLocation();
			CapsuleInfo.Rotation = HitPair.Value->GetComponentRotation();
			OutFramePackage.HitBoxInfo.Add(HitPair.Key, CapsuleInfo);
		}
	}
}

void ULagCompensationComponent::MoveBoxes(const ABlasterCharacter* HitCharacter, const FFramePackage& Package, const bool bEnableCollision)
{
	if (HitCharacter == nullptr) return;

	for (const TPair<FName, TObjectPtr<UCapsuleComponent>>& HitPair : HitCharacter->HitCollisionCapsules)
	{
		if (HitPair.Value != nullptr)
		{
			HitPair.Value->SetWorldLocation(Package.HitBoxInfo[HitPair.Key].Location);
			HitPair.Value->SetWorldRotation(Package.HitBoxInfo[HitPair.Key].Rotation);
			if (bEnableCollision)
			{
				HitPair.Value->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
				HitPair.Value->SetCollisionResponseToChannel(ECC_HitBox, ECR_Block);
			}
			else
			{
				HitPair.Value->SetCollisionEnabled(ECollisionEnabled::NoCollision);
			}
		}
	}
}
