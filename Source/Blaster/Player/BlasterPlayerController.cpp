// Copyright Nathan Guerin


#include "BlasterPlayerController.h"

#include "Blaster/BlasterGameplayTags.h"
#include "Blaster/BlasterComponents/CombatComponent.h"
#include "Blaster/Character/BlasterCharacter.h"
#include "Blaster/Input/BlasterInputComponent.h"
#include "Blaster/UI/BlasterHUD.h"
#include "Blaster/UI/CharacterOverlay.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "GameFramework/Character.h"

ABlasterCharacter* ABlasterPlayerController::GetBlasterCharacter() const
{
	return CastChecked<ABlasterCharacter>(GetCharacter());
}

void ABlasterPlayerController::BeginPlay()
{
	Super::BeginPlay();

	BlasterHUD = Cast<ABlasterHUD>(GetHUD());
}

void ABlasterPlayerController::SetHUDHealth(const float Health, const float MaxHealth)
{
	BlasterHUD = BlasterHUD == nullptr ? Cast<ABlasterHUD>(GetHUD()) : BlasterHUD;
	if (BlasterHUD && BlasterHUD->CharacterOverlay && BlasterHUD->CharacterOverlay->HealthBar && BlasterHUD->CharacterOverlay->HealthText)
	{
		const float HealthPercentage = Health / MaxHealth;
		BlasterHUD->CharacterOverlay->HealthBar->SetPercent(HealthPercentage);
		const FString HealthString = FString::Printf(TEXT("%d/%d"), FMath::CeilToInt(Health), FMath::CeilToInt(MaxHealth));
		BlasterHUD->CharacterOverlay->HealthText->SetText(FText::FromString(HealthString));
	}
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
	BlasterInputComponent->BindNativeAction(InputConfig, this, BlasterGameplayTags::InputTag_Aim, ETriggerEvent::Started, &ThisClass::Input_AimPressed, false);
	BlasterInputComponent->BindNativeAction(InputConfig, this, BlasterGameplayTags::InputTag_Aim, ETriggerEvent::Completed, &ThisClass::Input_AimReleased, false);
	BlasterInputComponent->BindNativeAction(InputConfig, this, BlasterGameplayTags::InputTag_Fire, ETriggerEvent::Started, &ThisClass::Input_FirePressed, false);
	BlasterInputComponent->BindNativeAction(InputConfig, this, BlasterGameplayTags::InputTag_Fire, ETriggerEvent::Completed, &ThisClass::Input_FireReleased, false);
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
	if (!GetBlasterCharacter()) return;

	GetBlasterCharacter()->Jump();
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

void ABlasterPlayerController::Input_AimPressed(const FInputActionValue& InputActionValue)
{
	if (!GetBlasterCharacter()) return;

	GetBlasterCharacter()->AimButtonPressed();
}

void ABlasterPlayerController::Input_AimReleased(const FInputActionValue& InputActionValue)
{
	if (!GetBlasterCharacter()) return;
	
	GetBlasterCharacter()->AimButtonReleased();
}

void ABlasterPlayerController::Input_FirePressed(const FInputActionValue& InputActionValue)
{
	if (!GetBlasterCharacter()) return;

	GetBlasterCharacter()->FireButtonPressed(true);
}

void ABlasterPlayerController::Input_FireReleased(const FInputActionValue& InputActionValue)
{
	if (!GetBlasterCharacter()) return;

	GetBlasterCharacter()->FireButtonPressed(false);
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
