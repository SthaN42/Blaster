// Copyright Nathan Guerin

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "BlasterPlayerState.generated.h"

class ABlasterPlayerController;
class ABlasterCharacter;
/**
 * 
 */
UCLASS()
class BLASTER_API ABlasterPlayerState : public APlayerState
{
	GENERATED_BODY()

public:
	void AddToScore(const int32 ScoreAmount);

	virtual void OnRep_Score() override;

	ABlasterCharacter* GetCharacter();
	ABlasterPlayerController* GetController();
	
private:
	ABlasterCharacter* Character;
	
	ABlasterPlayerController* Controller;
};
