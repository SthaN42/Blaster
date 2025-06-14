// Copyright Nathan Guerin


#include "BlasterInputConfig.h"

#include "Blaster/BlasterLogChannels.h"

const UInputAction* UBlasterInputConfig::FindNativeInputActionForTag(const FGameplayTag& InputTag, const bool bLogNotFound) const
{
	for (const FBlasterInputAction& Action : NativeInputActions)
	{
		if (Action.InputAction && (Action.InputTag == InputTag))
		{
			return Action.InputAction;
		}
	}

	if (bLogNotFound)
	{
		UE_LOG(LogBlaster, Error, TEXT("Can't find NativeInputAction for InputTag [%s] on InputConfig [%s]."), *InputTag.ToString(), *GetNameSafe(this));
	}

	return nullptr;
}

const UInputAction* UBlasterInputConfig::FindAbilityInputActionForTag(const FGameplayTag& InputTag, const bool bLogNotFound) const
{
	for (const FBlasterInputAction& Action : AbilityInputActions)
	{
		if (Action.InputAction && (Action.InputTag == InputTag))
		{
			return Action.InputAction;
		}
	}

	if (bLogNotFound)
	{
		UE_LOG(LogBlaster, Error, TEXT("Can't find AbilityInpuLogBlasterion for InputTag [%s] on InputConfig [%s]."), *InputTag.ToString(), *GetNameSafe(this));
	}

	return nullptr;
}
