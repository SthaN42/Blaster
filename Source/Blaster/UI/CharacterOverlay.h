// Copyright Nathan Guerin

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "CharacterOverlay.generated.h"

class UProgressBar;
class UTextBlock;
/**
 * 
 */
UCLASS()
class BLASTER_API UCharacterOverlay : public UUserWidget
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, BlueprintProtected = true))
	UProgressBar* HealthBar;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, BlueprintProtected = true))
	UTextBlock* HealthText;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, BlueprintProtected = true))
	UTextBlock* ScoreText;
	
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, BlueprintProtected = true))
	UTextBlock* DefeatsText;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, BlueprintProtected = true))
	UTextBlock* WeaponAmmoText;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, BlueprintProtected = true))
	UTextBlock* CarriedAmmoText;
};
