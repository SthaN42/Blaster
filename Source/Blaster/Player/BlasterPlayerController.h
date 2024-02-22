// Copyright Nathan Guerin

#pragma once

#include "CoreMinimal.h"
#include "InputActionValue.h"
#include "GameFramework/PlayerController.h"
#include "BlasterPlayerController.generated.h"

class ABlasterCharacter;
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

protected:
	virtual void SetupInputComponent() override;

	void Input_Move(const FInputActionValue& InputActionValue);
	void Input_Look(const FInputActionValue& InputActionValue);
	void Input_Jump(const FInputActionValue& InputActionValue);
	void Input_Crouch(const FInputActionValue& InputActionValue);
	void Input_Equip(const FInputActionValue& InputActionValue);
	void Input_AimPressed(const FInputActionValue& InputActionValue);
	void Input_AimReleased(const FInputActionValue& InputActionValue);

	void Input_AbilityInputTagPressed(FGameplayTag InputTag);
	void Input_AbilityInputTagReleased(FGameplayTag InputTag);
	void Input_AbilityInputTagHeld(FGameplayTag InputTag);

private:
	UPROPERTY(EditDefaultsOnly, Category = Input)
	TObjectPtr<UBlasterInputConfig> InputConfig;
};
