// Copyright Nathan Guerin


#include "HitScanWeapon.h"

#include "Blaster/BlasterComponents/LagCompensationComponent.h"
#include "Blaster/Character/BlasterCharacter.h"
#include "Blaster/Player/BlasterPlayerController.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystemComponent.h"

namespace WeaponCVars
{
	static bool ShowDebugHits = false;
	static FAutoConsoleVariableRef CVarShowDebugHits(
		TEXT("Weapon.ShowDebugHits"),
		ShowDebugHits,
		TEXT("When true, firing weapons will leave spheres indicating hit locations."),
		ECVF_Default | ECVF_Cheat);
}

void AHitScanWeapon::Fire(const FVector& HitTarget)
{
	Super::Fire(HitTarget);

	APawn* OwnerPawn = Cast<APawn>(GetOwner());
	if (!OwnerPawn) return;
	
	AController* InstigatorController = OwnerPawn->GetController();

	if (const USkeletalMeshSocket* SpawnSocket = GetWeaponMesh()->GetSocketByName("MuzzleFlash"))
	{
		const FTransform SocketTransform = SpawnSocket->GetSocketTransform(GetWeaponMesh());
		const FVector Start = SocketTransform.GetLocation();
		FHitResult FireHit;
		
		WeaponTraceHit(Start, HitTarget, FireHit);

		ABlasterCharacter* BlasterCharacter = Cast<ABlasterCharacter>(FireHit.GetActor());
		if (BlasterCharacter  && InstigatorController)
		{
			if (HasAuthority() && !bUseServerSideRewind)
			{
				UGameplayStatics::ApplyDamage(BlasterCharacter, Damage, InstigatorController, this, UDamageType::StaticClass());
			}
			if (!HasAuthority() && bUseServerSideRewind)
			{
				if (GetOwnerCharacter() && GetOwnerController() && GetOwnerCharacter()->GetLagCompensation())
				{
					GetOwnerCharacter()->GetLagCompensation()->ServerScoreRequest(BlasterCharacter,
						Start,
						HitTarget,
						GetOwnerController()->GetServerTime() - GetOwnerController()->SingleTripTime,
						this);
				}
			}
		}
		
		if (ImpactParticles)
		{
			UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ImpactParticles, FireHit.ImpactPoint, FireHit.ImpactNormal.Rotation());
		}
		if (HitSound)
		{
			UGameplayStatics::PlaySoundAtLocation(this, HitSound, FireHit.ImpactPoint);
		}
	}
}

void AHitScanWeapon::WeaponTraceHit(const FVector& TraceStart, const FVector& HitTarget, FHitResult& OutHit) const
{
	if (const UWorld* World = GetWorld())
	{
		const FVector TraceEnd = TraceStart + (HitTarget - TraceStart) * 1.25f;
		FVector BeamEnd = TraceEnd;
		
		World->LineTraceSingleByChannel(OutHit, TraceStart, TraceEnd, ECC_Visibility);
		if (OutHit.bBlockingHit)
		{
			BeamEnd = OutHit.ImpactPoint;
		}
		if (BeamParticles)
		{
			if (UParticleSystemComponent* Beam = UGameplayStatics::SpawnEmitterAtLocation(World, BeamParticles, TraceStart, FRotator::ZeroRotator))
			{
				Beam->SetVectorParameter("Target", BeamEnd);
			}
		}

		if (WeaponCVars::ShowDebugHits) DrawDebugSphere(GetWorld(), BeamEnd, 12, 12, FColor::Orange, false, 10.f);
	}
}
