// Copyright Nathan Guerin

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Announcement.generated.h"

class UTextBlock;
/**
 * 
 */
UCLASS()
class BLASTER_API UAnnouncement : public UUserWidget
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, BlueprintProtected = true))
	UTextBlock* AnnouncementText;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, BlueprintProtected = true))
	UTextBlock* WarmupTime;
	
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, BlueprintProtected = true))
	UTextBlock* InfoText;
};
