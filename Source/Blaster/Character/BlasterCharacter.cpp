// Copyright Nathan Guerin


#include "BlasterCharacter.h"

#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Blaster/Physics/BlasterCollisionChannels.h"
#include "Blaster/BlasterComponents/CombatComponent.h"
#include "Blaster/BlasterTypes/TurningInPlace.h"
#include "Blaster/GameMode/BlasterGameMode.h"
#include "Blaster/Player/BlasterPlayerController.h"
#include "Blaster/Weapon/Weapon.h"
#include "Components/CapsuleComponent.h"
#include "Components/WidgetComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Net/UnrealNetwork.h"
#include "NiagaraComponent.h"
#include "Blaster/BlasterComponents/BuffComponent.h"
#include "Blaster/BlasterComponents/LagCompensationComponent.h"
#include "Blaster/Player/BlasterPlayerState.h"
#include "Blaster/Weapon/WeaponTypes.h"
#include "Kismet/GameplayStatics.h"
#include "PhysicsEngine/PhysicsAsset.h"

ABlasterCharacter::ABlasterCharacter()
{
	PrimaryActorTick.bCanEverTick = true;

	NetUpdateFrequency = 66.f;
	MinNetUpdateFrequency = 33.f;

	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(GetMesh());
	CameraBoom->TargetArmLength = 600.f;
	CameraBoom->bUsePawnControlRotation = true;

	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	FollowCamera->bUsePawnControlRotation = false;

	bUseControllerRotationYaw = false;
	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->RotationRate = FRotator(0.f, 0.f, 900.f);
	
	GetCharacterMovement()->NavAgentProps.bCanCrouch = true;

	GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);
	GetMesh()->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);
	GetMesh()->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);

	GetMesh()->SetCollisionObjectType(ECC_SkeletalMesh);

	OverheadWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("OverheadWidget"));
	OverheadWidget->SetupAttachment(RootComponent);

	Combat = CreateDefaultSubobject<UCombatComponent>(TEXT("CombatComponent"));
	Combat->SetIsReplicated(true);

	Buff = CreateDefaultSubobject<UBuffComponent>(TEXT("BuffComponent"));
	Buff->SetIsReplicated(true);

	LagCompensation = CreateDefaultSubobject<ULagCompensationComponent>(TEXT("LagCompensationComponent"));

	TurningInPlace = ETurningInPlace::ETIP_NotTurning;

	DissolveTimeline = CreateDefaultSubobject<UTimelineComponent>(TEXT("DissolveTimelineComponent"));
	
	DissolveParticlesSystem = CreateDefaultSubobject<UNiagaraComponent>(TEXT("DissolveParticlesComponent"));
	DissolveParticlesSystem->SetupAttachment(GetMesh());
	DissolveParticlesSystem->bAutoActivate = false;

	AttachedGrenade = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("AttachedGrenade"));
	AttachedGrenade->SetupAttachment(GetMesh(), FName("GrenadeRightHandSocket"));
	AttachedGrenade->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

FVector ABlasterCharacter::GetHitTarget() const
{
	if (Combat == nullptr) return FVector();

	return Combat->HitTarget;
}

ECombatState ABlasterCharacter::GetCombatState() const
{
	if (Combat == nullptr) return ECombatState::ECS_MAX;

	return Combat->CombatState;
}

bool ABlasterCharacter::GetDisableGameplay() const
{
	if (BlasterPlayerController == nullptr) return true;

	return BlasterPlayerController->bDisableGameplay;
}

void ABlasterCharacter::BeginPlay()
{
	Super::BeginPlay();

	HitCapsulesConstruction();
	
	if (CameraBoom)
	{
		DefaultCameraPosition_Z = CameraBoom->SocketOffset.Z;
		CurrentCameraPosition_Z = DefaultCameraPosition_Z;
	}
	if (AttachedGrenade)
	{
		AttachedGrenade->SetVisibility(false);
	}

	if (HasAuthority())
	{
		OnTakeAnyDamage.AddDynamic(this, &ThisClass::ReceiveDamage);
	}
}

void ABlasterCharacter::HitCapsulesConstruction()
{
	if (GetMesh() == nullptr || GetMesh()->GetPhysicsAsset() == nullptr) return;

	for (const TObjectPtr<USkeletalBodySetup>& BodySetup : GetMesh()->GetPhysicsAsset()->SkeletalBodySetups)
	{
		const FName BoneName = BodySetup->BoneName;
		const FTransform BoneWorldTransform = GetMesh()->GetBoneTransform(GetMesh()->GetBoneIndex(BoneName));
		for (const auto& SphylElem : BodySetup->AggGeom.SphylElems)
		{
			const FTransform LocTransform = SphylElem.GetTransform();
			const FTransform WorldTransform = LocTransform * BoneWorldTransform;
			
			UCapsuleComponent* HitCapsule = NewObject<UCapsuleComponent>(this, UCapsuleComponent::StaticClass(), BoneName, RF_Transient);
			HitCapsule->SetupAttachment(GetMesh(), BoneName);
			HitCapsule->SetPhysMaterialOverride(BodySetup->GetPhysMaterial());
			HitCapsule->SetCollisionObjectType(ECC_HitBox);
			HitCapsule->SetCollisionResponseToAllChannels(ECR_Ignore);
			HitCapsule->SetCollisionResponseToChannel(ECC_HitBox, ECR_Block);
			HitCapsule->SetCollisionEnabled(ECollisionEnabled::NoCollision);
			HitCapsule->SetWorldTransform(WorldTransform);
			HitCapsule->SetCapsuleSize(SphylElem.Radius, SphylElem.Length / 2.f + SphylElem.Radius);

			HitCapsule->RegisterComponent();
			HitCollisionCapsules.Add(BoneName, HitCapsule);
		}
	}
}

void ABlasterCharacter::Destroyed()
{
	Super::Destroyed();

	const ABlasterGameMode* BlasterGameMode = Cast<ABlasterGameMode>(UGameplayStatics::GetGameMode(this));
	const bool bMatchNotInProgress = BlasterGameMode && BlasterGameMode->GetMatchState() != MatchState::InProgress;
	
	if (Combat && Combat->EquippedWeapon && bMatchNotInProgress)
	{
		Combat->EquippedWeapon->Destroy();
	}
}

void ABlasterCharacter::Jump()
{
	if (bIsCrouched)
	{
		UnCrouch();
	}
	else
	{
		Super::Jump();
	}
}

void ABlasterCharacter::AimOffset(float DeltaTime)
{
	if (Combat && Combat->EquippedWeapon == nullptr) return;
	
	const float Speed = CalculateSpeed();
	const bool bIsInAir = GetCharacterMovement()->IsFalling();

	if (Speed == 0.f && !bIsInAir) // Standing still, not jumping
	{
		bRotateRootBone = true;
		const FRotator CurrentAimRotation = FRotator(0.f, GetBaseAimRotation().Yaw, 0.f);
		const FRotator DeltaAimRotation = UKismetMathLibrary::NormalizedDeltaRotator(CurrentAimRotation, StartingAimRotation);
		AO_Yaw = DeltaAimRotation.Yaw;

		if (TurningInPlace == ETurningInPlace::ETIP_NotTurning)
		{
			InterpAO_Yaw = AO_Yaw;
		}
		TurnInPlace(DeltaTime);
	}
	if (Speed > 0.f || bIsInAir) // Running or jumping
	{
		bRotateRootBone = false;
		StartingAimRotation = FRotator(0.f, GetBaseAimRotation().Yaw, 0.f);
		AO_Yaw = 0.f;

		TurningInPlace = ETurningInPlace::ETIP_NotTurning;
	}
}

void ABlasterCharacter::SimProxiesTurn()
{
	if (Combat == nullptr || Combat->EquippedWeapon == nullptr) return;

	bRotateRootBone = false;

	if (CalculateSpeed() > 0.f)
	{
		TurningInPlace = ETurningInPlace::ETIP_NotTurning;
		return;
	}

	ProxyRotationLastFrame = ProxyRotation;
	ProxyRotation = GetActorRotation();

	ProxyYaw = UKismetMathLibrary::NormalizedDeltaRotator(ProxyRotation, ProxyRotationLastFrame).Yaw;
	
	if (FMath::Abs(ProxyYaw) > TurnThreshold)
	{
		if (ProxyYaw > TurnThreshold)
		{
			TurningInPlace = ETurningInPlace::ETIP_Right;
		}
		else if (ProxyYaw < -TurnThreshold)
		{
			TurningInPlace = ETurningInPlace::ETIP_Left;
		}
		else
		{
			TurningInPlace = ETurningInPlace::ETIP_NotTurning;
		}
		return;
	}
	TurningInPlace = ETurningInPlace::ETIP_NotTurning;
}

float ABlasterCharacter::CalculateSpeed() const
{
	FVector Velocity = GetVelocity();
	Velocity.Z = 0.f;
	return Velocity.Size();
}

void ABlasterCharacter::TurnInPlace(float DeltaTime)
{
	if (AO_Yaw > 80.f)
	{
		TurningInPlace = ETurningInPlace::ETIP_Right;
	}
	else if (AO_Yaw < -80.f)
	{
		TurningInPlace = ETurningInPlace::ETIP_Left;
	}

	if (TurningInPlace != ETurningInPlace::ETIP_NotTurning)
	{
		InterpAO_Yaw = FMath::FInterpTo(InterpAO_Yaw, 0.f, DeltaTime, 4.f);
		AO_Yaw = InterpAO_Yaw;
		if (FMath::Abs(AO_Yaw) < 15.f)
		{
			TurningInPlace = ETurningInPlace::ETIP_NotTurning;
			StartingAimRotation = FRotator(0.f, GetBaseAimRotation().Yaw, 0.f);
		}
	}
}

void ABlasterCharacter::InterpCameraPosition(float DeltaTime)
{
	if (!IsLocallyControlled()) return;
	
	if (CameraBoom == nullptr) return;

	if (bIsCrouched)
	{
		CurrentCameraPosition_Z = FMath::FInterpTo(CurrentCameraPosition_Z, CrouchedEyeHeight, DeltaTime, CrouchedCameraInterpSpeed);
	}
	else
	{
		CurrentCameraPosition_Z = FMath::FInterpTo(CurrentCameraPosition_Z, DefaultCameraPosition_Z, DeltaTime, CrouchedCameraInterpSpeed);
	}

	CameraBoom->SocketOffset.Z = CurrentCameraPosition_Z;
}

void ABlasterCharacter::HideCharacterIfCameraClose() const
{
	if (!IsLocallyControlled()) return;

	const bool bCameraTooClose = (FollowCamera->GetComponentLocation() - GetActorLocation()).Size() < CameraThreshold;

	GetMesh()->SetVisibility(!bCameraTooClose);

	if (Combat && Combat->EquippedWeapon && Combat->EquippedWeapon->GetWeaponMesh())
	{
		Combat->EquippedWeapon->GetWeaponMesh()->bOwnerNoSee = bCameraTooClose;
	}
	if (Combat && Combat->SecondaryWeapon && Combat->SecondaryWeapon->GetWeaponMesh())
	{
		Combat->SecondaryWeapon->GetWeaponMesh()->bOwnerNoSee = bCameraTooClose;
	}
}

void ABlasterCharacter::OnRep_Health(float LastHealth)
{
	UpdateHUDHealth();

	if (Health < LastHealth)
	{
		PlayHitReactMontage();
	}
}

void ABlasterCharacter::OnRep_Shield(float LastShield)
{
	UpdateHUDShield();

	if (Shield < LastShield)
	{
		PlayHitReactMontage();
	}
}

void ABlasterCharacter::UpdateHUDHealth()
{
	if (GetBlasterPlayerController())
	{
		GetBlasterPlayerController()->SetHUDHealth(Health, MaxHealth);
	}
}

void ABlasterCharacter::UpdateHUDShield()
{
	if (GetBlasterPlayerController())
	{
		GetBlasterPlayerController()->SetHUDShield(Shield, MaxShield);
	}
}

void ABlasterCharacter::UpdateHUDAmmo()
{
	if (GetBlasterPlayerController() && Combat && Combat->EquippedWeapon)
	{
		GetBlasterPlayerController()->SetHUDCarriedAmmo(Combat->CarriedAmmo);
		GetBlasterPlayerController()->SetHUDWeaponAmmo(Combat->EquippedWeapon->GetAmmo());
	}
}

void ABlasterCharacter::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	PollInit();
	
	RotateInPlace(DeltaSeconds);

	InterpCameraPosition(DeltaSeconds);
	
	HideCharacterIfCameraClose();
}

void ABlasterCharacter::RotateInPlace(float DeltaSeconds)
{
	if (GetBlasterPlayerController() && GetBlasterPlayerController()->bDisableGameplay)
	{
		bUseControllerRotationYaw = false;
		TurningInPlace = ETurningInPlace::ETIP_NotTurning;
		return;
	}
	
	if (GetLocalRole() > ROLE_SimulatedProxy && IsLocallyControlled())
	{
		AimOffset(DeltaSeconds);
	}
	else
	{
		TimeSinceLastMovementReplication += DeltaSeconds;
		if (TimeSinceLastMovementReplication > 0.25f)
		{
			OnRep_ReplicatedMovement();
		}
	}
}

void ABlasterCharacter::PawnClientRestart()
{
	Super::PawnClientRestart();

	if (const APlayerController* PlayerController = Cast<APlayerController>(GetController()))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			if (Subsystem)
			{
				Subsystem->AddMappingContext(BlasterMappingContext, 0);
			}
		}
	}
}

void ABlasterCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(ABlasterCharacter, OverlappingWeapon, COND_OwnerOnly);
	DOREPLIFETIME(ABlasterCharacter, Health);
	DOREPLIFETIME(ABlasterCharacter, Shield);
}

void ABlasterCharacter::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	if (Combat)
	{
		Combat->Character = this;
		
		if (Buff)
		{
			Buff->Character = this;
			Buff->SetInitialSpeeds(Combat->BaseWalkSpeed, Combat->AimWalkSpeed, GetCharacterMovement()->MaxWalkSpeedCrouched);
			Buff->SetInitialJumpVelocity(GetCharacterMovement()->JumpZVelocity);
		}
	}
	if (LagCompensation)
	{
		LagCompensation->Character = this;
		if (GetBlasterPlayerController())
		{
			LagCompensation->Controller = GetBlasterPlayerController();
		}
	}
}

void ABlasterCharacter::PlayFireMontage(bool bAiming) const
{
	if (Combat == nullptr || Combat->EquippedWeapon == nullptr) return;

	if (UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance(); AnimInstance && FireWeaponMontage)
	{
		AnimInstance->Montage_Play(FireWeaponMontage);
		const FName SectionName = bAiming ? FName("RifleAim") : FName("RifleHip");
		AnimInstance->Montage_JumpToSection(SectionName);
	}
}

void ABlasterCharacter::PlayReloadMontage() const
{
	if (Combat == nullptr || Combat->EquippedWeapon == nullptr) return;

	if (UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance(); AnimInstance && ReloadMontage)
	{
		AnimInstance->Montage_Play(ReloadMontage);
		FName SectionName;

		switch (Combat->EquippedWeapon->GetWeaponType())
		{
		case EWeaponType::EWT_AssaultRifle:
			SectionName = FName("Rifle");
			break;
		case EWeaponType::EWT_RocketLauncher:
			SectionName = FName("RocketLauncher");
			break;
		case EWeaponType::EWT_Pistol:
			SectionName = FName("Pistol");
			break;
		case EWeaponType::EWT_SubmachineGun:
			SectionName = FName("Pistol");
			break;
		case EWeaponType::EWT_Shotgun:
			SectionName = FName("Shotgun");
			break;
		case EWeaponType::EWT_SniperRifle:
			SectionName = FName("SniperRifle");
			break;
		case EWeaponType::EWT_GrenadeLauncher:
			SectionName = FName("GrenadeLauncher");
			break;
		}
		
		AnimInstance->Montage_JumpToSection(SectionName);
	}
}

void ABlasterCharacter::PlayElimMontage() const
{
	if (UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance(); AnimInstance && ElimMontage)
	{
		AnimInstance->Montage_Play(ElimMontage);
	}
}

void ABlasterCharacter::PlayThrowGrenadeMontage() const
{
	if (UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance(); AnimInstance && ThrowGrenadeMontage)
	{
		AnimInstance->Montage_Play(ThrowGrenadeMontage);
	}
}

void ABlasterCharacter::PlayHitReactMontage() const
{
	if (Combat == nullptr || Combat->EquippedWeapon == nullptr) return;

	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && HitReactMontage)
	{
		AnimInstance->Montage_Play(HitReactMontage);
		const FName SectionName = FName("FromFront");
		AnimInstance->Montage_JumpToSection(SectionName);
	}
}

void ABlasterCharacter::ReceiveDamage(AActor* DamagedActor, float Damage, const UDamageType* DamageType,
	AController* InstigatorController, AActor* DamageCauser)
{
	if (bEliminated == true) return;
	
	float DamageToHealth = Damage;
	if (Shield > 0.f)
	{
		if (Shield >= Damage)
		{
			DamageToHealth = 0.f;
			Shield = FMath::Clamp(Shield - Damage, 0.f, MaxShield);
		}
		else
		{
			DamageToHealth = FMath::Clamp(DamageToHealth - Shield, 0.f, Damage);
			Shield = 0.f;
		}
	}
	
	Health = FMath::Clamp(Health - DamageToHealth, 0.f, MaxHealth);
	
	UpdateHUDHealth();
	UpdateHUDShield();
	PlayHitReactMontage();

	if (Health <= 0.f)
	{
		if (ABlasterGameMode* BlasterGameMode = GetWorld()->GetAuthGameMode<ABlasterGameMode>())
		{
			ABlasterPlayerController* AttackerController = Cast<ABlasterPlayerController>(InstigatorController);
			BlasterGameMode->PlayerEliminated(this, GetBlasterPlayerController(), AttackerController);
		}
	}
}

void ABlasterCharacter::PollInit()
{
	if (BlasterPlayerState == nullptr)
	{
		BlasterPlayerState = GetPlayerState<ABlasterPlayerState>();
		if (BlasterPlayerState)
		{
			BlasterPlayerState->AddToScore(0);
			BlasterPlayerState->AddToDefeats(0);
		}
	}
	if (BlasterPlayerController == nullptr)
	{
		if (GetBlasterPlayerController())
		{
			SpawnDefaultWeapon();
			UpdateHUDAmmo();
			UpdateHUDHealth();
			UpdateHUDShield();
		}
	}
}

void ABlasterCharacter::OnRep_ReplicatedMovement()
{
	Super::OnRep_ReplicatedMovement();

	SimProxiesTurn();
	TimeSinceLastMovementReplication = 0.f;
}

void ABlasterCharacter::Elim()
{
	if (Combat)
	{
		if (Combat->EquippedWeapon)
		{
			DropOrDestroyWeapon(Combat->EquippedWeapon);
		}
		if (Combat->SecondaryWeapon)
		{
			DropOrDestroyWeapon(Combat->SecondaryWeapon);
		}
		Combat->EquippedWeapon = nullptr;
		Combat->SecondaryWeapon = nullptr;
	}
	
	MulticastElim();

	GetWorldTimerManager().SetTimer(ElimTimer, this, &ThisClass::ElimTimerFinished, ElimDelay);
}

void ABlasterCharacter::MulticastElim_Implementation()
{
	bEliminated = true;
	PlayElimMontage();

	if (GetBlasterPlayerController())
	{
		GetBlasterPlayerController()->SetHUDWeaponAmmo(0);
		GetBlasterPlayerController()->SetHUDCarriedAmmo(0);
	}

	// Start elim effect
	if (DissolveMaterialInstance)
	{
		DynamicDissolveMaterialInstance = UMaterialInstanceDynamic::Create(DissolveMaterialInstance, this);
		GetMesh()->SetMaterial(0, DynamicDissolveMaterialInstance);
		DynamicDissolveMaterialInstance->SetScalarParameterValue(TEXT("DissolveAmount"), 0.f);
	}
	StartDissolveEffect();

	// Disable character movement
	GetCharacterMovement()->DisableMovement();
	if (GetBlasterPlayerController())
	{
		GetBlasterPlayerController()->bDisableGameplay = true;
	}
	if (Combat)
	{
		Combat->FireButtonPressed(false);
	}
	// Disable collision
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	GetMesh()->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	// Remove the sniper scope, in case it's shown while eliminated
	if (IsLocallyControlled() && Combat && Combat->bAiming && Combat->EquippedWeapon && Combat->EquippedWeapon->GetWeaponType() == EWeaponType::EWT_SniperRifle)
	{
		ShowSniperScopeWidget(false);
	}
}

void ABlasterCharacter::ElimTimerFinished()
{
	if (ABlasterGameMode* BlasterGameMode = GetWorld()->GetAuthGameMode<ABlasterGameMode>())
	{
		BlasterGameMode->RequestRespawn(this, Controller);
	}
}

void ABlasterCharacter::DropOrDestroyWeapon(AWeapon* Weapon)
{
	if (Weapon == nullptr) return;
	
	if (Weapon->bDestroyWeapon)
	{
		Weapon->Destroy();
	}
	else
	{
		Weapon->Dropped();
	}
}

void ABlasterCharacter::UpdateDissolveEffect(float DissolveValue)
{
	DynamicDissolveMaterialInstance->SetScalarParameterValue(TEXT("DissolveAmount"), DissolveValue);
}

void ABlasterCharacter::StartDissolveEffect()
{
	DissolveTrack.BindDynamic(this, &ThisClass::UpdateDissolveEffect);
	if (DissolveCurve && DissolveTimeline && DissolveParticlesSystem && ElimSound)
	{
		DissolveTimeline->AddInterpFloat(DissolveCurve, DissolveTrack);
		DissolveTimeline->Play();
		DissolveParticlesSystem->Activate(true);
		UGameplayStatics::PlaySoundAtLocation(this, ElimSound, GetActorLocation());
	}
}

void ABlasterCharacter::EquipButtonPressed()
{
	ServerEquipButtonPressed();
}

void ABlasterCharacter::ServerEquipButtonPressed_Implementation()
{
	if (Combat)
	{
		if (OverlappingWeapon)
		{
			Combat->EquipWeapon(OverlappingWeapon);
		}
		else if (Combat->ShouldSwapWeapons())
		{
			Combat->SwapWeapons();
		}
	}
}

void ABlasterCharacter::DropButtonPressed()
{
// 	if (HasAuthority())
// 	{
// 		if (Combat)
// 		{
// 			Combat->DropWeapon();
// 		}
// 	}
// 	else
// 	{
// 		ServerDropButtonPressed();
// 	}
}

void ABlasterCharacter::ServerDropButtonPressed_Implementation()
{
// 	if (Combat)
// 	{
// 		Combat->DropWeapon();
// 	}
}

void ABlasterCharacter::GrenadeButtonPressed()
{
	if (Combat)
	{
		Combat->ThrowGrenade();
	}
}

void ABlasterCharacter::ReloadButtonPressed()
{
	if (Combat)
	{
		Combat->Reload();
	}
}

void ABlasterCharacter::AimButtonPressed()
{
	if (Combat && Combat->EquippedWeapon)
	{
		Combat->SetAiming(true);
	}
}

void ABlasterCharacter::AimButtonReleased()
{
	if (Combat && Combat->EquippedWeapon)
	{
		Combat->SetAiming(false);
	}
}

void ABlasterCharacter::FireButtonPressed(const bool bPressed)
{
	if (Combat && Combat->EquippedWeapon)
	{
		Combat->FireButtonPressed(bPressed);
	}
}

void ABlasterCharacter::ToggleCrouch()
{
	const UCharacterMovementComponent* MoveComp = GetCharacterMovement();
	
	if (bIsCrouched || MoveComp->bWantsToCrouch)
	{
		UnCrouch();
	}
	else if (MoveComp->IsMovingOnGround())
	{
		Crouch();
	}
}

void ABlasterCharacter::SpawnDefaultWeapon() const
{
	if (DefaultWeaponClass == nullptr) return;
	
	const ABlasterGameMode* BlasterGameMode = Cast<ABlasterGameMode>(UGameplayStatics::GetGameMode(this));
	UWorld* World = GetWorld();
	if (BlasterGameMode && World && !IsEliminated() && DefaultWeaponClass && Combat)
	{
		AWeapon* StartingWeapon = World->SpawnActor<AWeapon>(DefaultWeaponClass);
		StartingWeapon->bDestroyWeapon = true;
		Combat->EquipWeapon(StartingWeapon);
	}
}

ABlasterPlayerController* ABlasterCharacter::GetBlasterPlayerController()
{
	if (BlasterPlayerController == nullptr)
	{
		BlasterPlayerController = Cast<ABlasterPlayerController>(GetController());
	}
	return BlasterPlayerController;
}

void ABlasterCharacter::SetOverlappingWeapon(AWeapon* InWeapon)
{
	AWeapon* LastWeapon = OverlappingWeapon;
	OverlappingWeapon = InWeapon;
	
	if (IsLocallyControlled())
	{
		OnRep_OverlappingWeapon(LastWeapon);
	}
}

void ABlasterCharacter::OnRep_OverlappingWeapon(AWeapon* LastWeapon)
{
	if (OverlappingWeapon)
	{
		OverlappingWeapon->ShowPickupWidget(true);
	}
	if (LastWeapon)
	{
		LastWeapon->ShowPickupWidget(false);
	}
}

bool ABlasterCharacter::IsWeaponEquipped() const
{
	return Combat && Combat->EquippedWeapon;
}

AWeapon* ABlasterCharacter::GetEquippedWeapon() const
{
	if (Combat == nullptr) return nullptr;
	
	return Combat->EquippedWeapon;
}

bool ABlasterCharacter::IsAiming() const
{
	return Combat && Combat->bAiming;
}

bool ABlasterCharacter::IsLocallyReloading() const
{
	return Combat && Combat->bLocallyReloading;
}
