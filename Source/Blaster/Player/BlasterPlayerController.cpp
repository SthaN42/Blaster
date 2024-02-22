// Copyright Nathan Guerin


#include "BlasterPlayerController.h"

#include "Blaster/BlasterGameplayTags.h"
#include "Blaster/BlasterComponents/CombatComponent.h"
#include "Blaster/Character/BlasterCharacter.h"
#include "Blaster/Input/BlasterInputComponent.h"
#include "GameFramework/Character.h"

ABlasterCharacter* ABlasterPlayerController::GetBlasterCharacter() const
{
	return CastChecked<ABlasterCharacter>(GetCharacter());
}

void ABlasterPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	UBlasterInputComponent* BlasterInputComponent = CastChecked<UBlasterInputComponent>(InputComponent);

	BlasterInputComponent->BindAbilityActions(InputConfig, this, &ThisClass::Input_AbilityInputTagPressed, &ThisClass::Input_AbilityInputTagReleased, &ThisClass::Input_AbilityInputTagHeld);

	BlasterInputComponent->BindNativeAction(InputConfig, this, BlasterGameplayTags::InputTag_Move, ETriggerEvent::Triggered, &ThisClass::Input_Move, false);
	BlasterInputComponent->BindNativeAction(InputConfig, this, BlasterGameplayTags::InputTag_Look, ETriggerEvent::Triggered, &ThisClass::Input_Look, false);
	BlasterInputComponent->BindNativeAction(InputConfig, this, BlasterGameplayTags::InputTag_Jump, ETriggerEvent::Triggered, &ThisClass::Input_Jump, false);
	BlasterInputComponent->BindNativeAction(InputConfig, this, BlasterGameplayTags::InputTag_Crouch, ETriggerEvent::Triggered, &ThisClass::Input_Crouch, false);
	BlasterInputComponent->BindNativeAction(InputConfig, this, BlasterGameplayTags::InputTag_Equip, ETriggerEvent::Triggered, &ThisClass::Input_Equip, false);
}

void ABlasterPlayerController::Input_Move(const FInputActionValue& InputActionValue)
{
	if (!GetPawn()) return;
	
	const FVector2D Value = InputActionValue.Get<FVector2D>();
	const FRotator MovementRotation(0.f, GetControlRotation().Yaw, 0.f);

	if (Value.X != 0.f)
	{
		const FVector MovementDirection = MovementRotation.RotateVector(FVector::RightVector);
		GetPawn()->AddMovementInput(MovementDirection, Value.X);
	}

	if (Value.Y != 0.f)
	{
		const FVector MovementDirection = MovementRotation.RotateVector(FVector::ForwardVector);
		GetPawn()->AddMovementInput(MovementDirection, Value.Y);
	}
}

void ABlasterPlayerController::Input_Look(const FInputActionValue& InputActionValue)
{
	if (!GetPawn()) return;
	
	const FVector2D Value = InputActionValue.Get<FVector2D>();

	if (Value.X != 0.f)
	{
		GetPawn()->AddControllerYawInput(Value.X);
	}
	if (Value.Y != 0.f)
	{
		GetPawn()->AddControllerPitchInput(Value.Y);
	}
}

void ABlasterPlayerController::Input_Jump(const FInputActionValue& InputActionValue)
{
	if (!GetCharacter()) return;

	GetCharacter()->Jump();
}

void ABlasterPlayerController::Input_Crouch(const FInputActionValue& InputActionValue)
{
	if (!GetBlasterCharacter()) return;

	GetBlasterCharacter()->ToggleCrouch();
}

void ABlasterPlayerController::Input_Equip(const FInputActionValue& InputActionValue)
{
	if (!GetBlasterCharacter()) return;

	GetBlasterCharacter()->EquipButtonPressed();
}

void ABlasterPlayerController::Input_AbilityInputTagPressed(FGameplayTag InputTag)
{
}

void ABlasterPlayerController::Input_AbilityInputTagReleased(FGameplayTag InputTag)
{
}

void ABlasterPlayerController::Input_AbilityInputTagHeld(FGameplayTag InputTag)
{
}
