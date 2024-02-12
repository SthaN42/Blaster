// Copyright Nathan Guerin

#pragma once

#include "CoreMinimal.h"
#include "BlasterInputConfig.h"
#include "EnhancedInputComponent.h"
#include "BlasterInputComponent.generated.h"

struct FGameplayTag;
class UBlasterInputConfig;
/**
 * 
 */
UCLASS()
class BLASTER_API UBlasterInputComponent : public UEnhancedInputComponent
{
	GENERATED_BODY()

public:
	template <class UserClass, typename FuncType>
	void BindNativeAction(const UBlasterInputConfig* InputConfig, UserClass* Object, const FGameplayTag& InputTag, ETriggerEvent TriggerEvent, FuncType Func, const bool bLogNotFound);

	template <class UserClass, typename PressedFuncType, typename ReleasedFuncType, typename HeldFuncType>
	void BindAbilityActions(const UBlasterInputConfig* InputConfig, UserClass* Object, PressedFuncType PressedFunc, ReleasedFuncType ReleasedFunc, HeldFuncType HeldFunc);
};

template <class UserClass, typename FuncType>
void UBlasterInputComponent::BindNativeAction(const UBlasterInputConfig* InputConfig, UserClass* Object, const FGameplayTag& InputTag, ETriggerEvent TriggerEvent, FuncType Func, const bool bLogNotFound)
{
	check(InputConfig)

	if (const UInputAction* InputAction = InputConfig->FindNativeInputActionForTag(InputTag, bLogNotFound))
	{
		BindAction(InputAction, TriggerEvent, Object, Func);
	}
}

template <class UserClass, typename PressedFuncType, typename ReleasedFuncType, typename HeldFuncType>
void UBlasterInputComponent::BindAbilityActions(const UBlasterInputConfig* InputConfig, UserClass* Object, PressedFuncType PressedFunc, ReleasedFuncType ReleasedFunc, HeldFuncType HeldFunc)
{
	check(InputConfig)

	for (const FBlasterInputAction& Action : InputConfig->AbilityInputActions)
	{
		if(Action.InputAction && Action.InputTag.IsValid())
		{
			if (PressedFunc)
			{
				BindAction(Action.InputAction, ETriggerEvent::Started, Object, PressedFunc, Action.InputTag);
			}

			if (ReleasedFunc)
			{
				BindAction(Action.InputAction, ETriggerEvent::Completed, Object, ReleasedFunc, Action.InputTag);
			}

			if (HeldFunc)
			{
				BindAction(Action.InputAction, ETriggerEvent::Triggered, Object, HeldFunc, Action.InputTag);
			}
		}
	}
}
