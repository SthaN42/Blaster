// Copyright Nathan Guerin

#pragma once

#include "CoreMinimal.h"
#include "InputActionValue.h"
#include "GameFramework/PlayerController.h"
#include "BlasterPlayerController.generated.h"

class ABlasterCharacter;
class ABlasterHUD;
struct FGameplayTag;
struct FInputActionValue;
class UBlasterInputConfig;

/**
 * 
 */
UCLASS()
class BLASTER_API ABlasterPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable)
	ABlasterCharacter* GetBlasterCharacter() const;

	void SetHUDHealth(const float Health, const float MaxHealth);
	void SetHUDScore(const int32 Score);
	void SetHUDDefeats(const int32 Defeats);
	void SetHUDWeaponAmmo(const int32 Ammo);
	void SetHUDCarriedAmmo(const int32 Ammo);

	virtual void OnPossess(APawn* InPawn) override;

protected:
	virtual void BeginPlay() override;
	
	virtual void SetupInputComponent() override;

	void Input_Move(const FInputActionValue& InputActionValue);
	void Input_Look(const FInputActionValue& InputActionValue);
	void Input_Jump(const FInputActionValue& InputActionValue);
	void Input_Crouch(const FInputActionValue& InputActionValue);
	void Input_Equip(const FInputActionValue& InputActionValue);
	void Input_AimPressed(const FInputActionValue& InputActionValue);
	void Input_AimReleased(const FInputActionValue& InputActionValue);
	void Input_FirePressed(const FInputActionValue& InputActionValue);
	void Input_FireReleased(const FInputActionValue& InputActionValue);

	void Input_AbilityInputTagPressed(FGameplayTag InputTag);
	void Input_AbilityInputTagReleased(FGameplayTag InputTag);
	void Input_AbilityInputTagHeld(FGameplayTag InputTag);

private:
	UPROPERTY(EditDefaultsOnly, Category = Input)
	TObjectPtr<UBlasterInputConfig> InputConfig;

	ABlasterHUD* BlasterHUD;
};
