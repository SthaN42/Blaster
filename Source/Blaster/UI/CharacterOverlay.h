// Copyright Nathan Guerin

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "CharacterOverlay.generated.h"

/**
 * 
 */
UCLASS()
class BLASTER_API UCharacterOverlay : public UUserWidget
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, BlueprintProtected = true))
	class UProgressBar* HealthBar;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, BlueprintProtected = true))
	class UTextBlock* HealthText;
};
