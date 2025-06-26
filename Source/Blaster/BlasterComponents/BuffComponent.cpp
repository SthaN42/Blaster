// Copyright Nathan Guerin


#include "BuffComponent.h"

#include "CombatComponent.h"
#include "Blaster/Character/BlasterCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"

UBuffComponent::UBuffComponent()
{
	PrimaryComponentTick.bCanEverTick = true;

}

void UBuffComponent::BeginPlay()
{
	Super::BeginPlay();
	
}

void UBuffComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	HealRampUp(DeltaTime);
	ShieldRampUp(DeltaTime);
}

void UBuffComponent::Heal(float HealAmount, float HealingTime)
{
	bHealing = true;
	HealingRate = HealAmount / HealingTime;
	AmountToHeal += HealAmount;
}

void UBuffComponent::HealRampUp(float DeltaTime)
{
	if (!bHealing || Character == nullptr || Character->IsEliminated()) return;

	const float HealThisFrame = HealingRate * DeltaTime;

	Character->SetHealth(FMath::Clamp(Character->GetHealth() + HealThisFrame, 0.f, Character->GetMaxHealth()));
	Character->UpdateHUDHealth();
	AmountToHeal -= HealThisFrame;

	if (AmountToHeal <= 0.f || Character->GetHealth() >= Character->GetMaxHealth())
	{
		bHealing = false;
		AmountToHeal = 0.f;
	}
}

void UBuffComponent::ReplenishShield(float ShieldAmount, float ReplenishTime)
{
	bReplenishingShield = true;
	ShieldReplenishRate = ShieldAmount / ReplenishTime;
	ShieldReplenishAmount += ShieldAmount;
}

void UBuffComponent::ShieldRampUp(float DeltaTime)
{
	if (!bReplenishingShield || Character == nullptr || Character->IsEliminated()) return;

	const float ReplenishThisFrame = ShieldReplenishRate * DeltaTime;

	Character->SetShield(FMath::Clamp(Character->GetShield() + ReplenishThisFrame, 0.f, Character->GetMaxShield()));
	Character->UpdateHUDShield();
	ShieldReplenishAmount -= ReplenishThisFrame;

	if (ShieldReplenishAmount <= 0.f || Character->GetShield() >= Character->GetMaxShield())
	{
		bReplenishingShield = false;
		ShieldReplenishAmount = 0.f;
	}
}

void UBuffComponent::BuffSpeed(float BuffBaseSpeed, float BuffAimSpeed, float BuffCrouchSpeed, float BuffDuration)
{
	if (Character && Character->GetCharacterMovement() && Character->GetCombat())
	{
		Character->GetWorldTimerManager().SetTimer(SpeedBuffTimer, this, &ThisClass::ResetSpeeds, BuffDuration);

		MulticastSpeedBuff(BuffBaseSpeed, BuffAimSpeed, BuffCrouchSpeed);
	}
}

void UBuffComponent::ResetSpeeds()
{
	if (Character && Character->GetCharacterMovement() && Character->GetCombat())
	{
		MulticastSpeedBuff(InitialBaseSpeed, InitialAimSpeed, InitialCrouchSpeed);
	}
}

void UBuffComponent::MulticastSpeedBuff_Implementation(float BaseSpeed, float AimSpeed, float CrouchSpeed)
{
	if (Character && Character->GetCharacterMovement() && Character->GetCombat())
	{
		Character->GetCharacterMovement()->MaxWalkSpeed = BaseSpeed;
		Character->GetCharacterMovement()->MaxWalkSpeedCrouched = CrouchSpeed;
		Character->GetCombat()->SetSpeeds(BaseSpeed, AimSpeed, CrouchSpeed);
	}
}

void UBuffComponent::BuffJump(float BuffJumpVelocity, float BuffDuration)
{
	if (Character && Character->GetCharacterMovement())
	{
		Character->GetWorldTimerManager().SetTimer(JumpBuffTimer, this, &ThisClass::ResetJump, BuffDuration);

		MulticastJumpBuff(BuffJumpVelocity);
	}
}

void UBuffComponent::ResetJump()
{
	if (Character && Character->GetCharacterMovement())
	{
		MulticastJumpBuff(InitialJumpVelocity);
	}
}

void UBuffComponent::MulticastJumpBuff_Implementation(float JumpVelocity)
{
	if (Character && Character->GetCharacterMovement())
	{
		Character->GetCharacterMovement()->JumpZVelocity = JumpVelocity;
	}
}
