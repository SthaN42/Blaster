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
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;

	/** Replication notifies */
	virtual void OnRep_Score() override;
	UFUNCTION()
	virtual void OnRep_Defeats();

	void AddToScore(const int32 ScoreAmount);
	void AddToDefeats(const int32 DefeatsAmount);
	
private:
	UPROPERTY()
	ABlasterCharacter* Character;
	
	UPROPERTY()
	ABlasterPlayerController* Controller;

	UPROPERTY(ReplicatedUsing = OnRep_Defeats, Category = PlayerState, BlueprintGetter=GetDefeats)
	int32 Defeats;

	/** Getters / Setters */
public:
	ABlasterCharacter* GetCharacter();
	ABlasterPlayerController* GetController();

	UFUNCTION(BlueprintGetter)
	FORCEINLINE int32 GetDefeats() const { return Defeats; }

	void SetDefeats(const int32 NewDefeats);
};
