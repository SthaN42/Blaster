// Copyright Nathan Guerin

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "OverheadWidget.generated.h"

class UTextBlock;
/**
 * 
 */
UCLASS()
class BLASTER_API UOverheadWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable)
	void SetDisplayText(const FString& TextToDisplay) const;

	UFUNCTION(BlueprintCallable)
	void ShowPlayerNetRole(const APawn* InPawn) const;
	
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, BlueprintProtected = true))
	TObjectPtr<UTextBlock> Text_Display;
};
