// Copyright Nathan Guerin

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Engine/DataAsset.h"
#include "BlasterInputConfig.generated.h"

class UInputAction;

USTRUCT(BlueprintType)
struct FBlasterInputAction
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly)
	TObjectPtr<const UInputAction> InputAction = nullptr;

	UPROPERTY(EditDefaultsOnly, meta = (Categories = InputTag))
	FGameplayTag InputTag = FGameplayTag();
};

/**
 * 
 */
UCLASS(BlueprintType, Const)
class BLASTER_API UBlasterInputConfig : public UDataAsset
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Blaster|Input")
	const UInputAction* FindNativeInputActionForTag(const FGameplayTag& InputTag, const bool bLogNotFound = true) const;
	
	UFUNCTION(BlueprintCallable, Category = "Blaster|Input")
	const UInputAction* FindAbilityInputActionForTag(const FGameplayTag& InputTag, const bool bLogNotFound = true) const;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (TitleProperty = InputAction))
	TArray<FBlasterInputAction> NativeInputActions;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (TitleProperty = InputAction))
	TArray<FBlasterInputAction> AbilityInputActions;
};
