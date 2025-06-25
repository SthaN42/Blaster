// Copyright Nathan Guerin

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "BuffComponent.generated.h"


class ABlasterCharacter;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class BLASTER_API UBuffComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UBuffComponent();
	friend ABlasterCharacter;
	
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	/* Heal Buff */
	
	UFUNCTION(BlueprintCallable, Category="Buff")
	void Heal(float HealAmount, float HealingTime);

	/* Speed Buff */
	
	void SetInitialSpeeds(float BaseSpeed, float AimSpeed, float CrouchSpeed) { InitialBaseSpeed = BaseSpeed; InitialAimSpeed = AimSpeed; InitialCrouchSpeed = CrouchSpeed; }

	UFUNCTION(BlueprintCallable, Category="Buff")
	void BuffSpeed(float BuffBaseSpeed, float BuffAimSpeed, float BuffCrouchSpeed, float BuffDuration);

	/* Jump Buff */
	
	void SetInitialJumpVelocity(float Velocity) { InitialJumpVelocity = Velocity; }

	UFUNCTION(BlueprintCallable, Category="Buff")
	void BuffJump(float BuffJumpVelocity, float BuffDuration);
	
protected:
	virtual void BeginPlay() override;

	void HealRampUp(float DeltaTime);

private:
	UPROPERTY()
	TObjectPtr<ABlasterCharacter> Character;

	/* Heal Buff */

	bool bHealing = false;
	float HealingRate = 0.f;
	float AmountToHeal = 0.f;

	/* Speed Buff */

	FTimerHandle SpeedBuffTimer;
	float InitialBaseSpeed;
	float InitialAimSpeed;
	float InitialCrouchSpeed;
	
	void ResetSpeeds();

	UFUNCTION(NetMulticast, Reliable)
	void MulticastSpeedBuff(float BaseSpeed, float AimSpeed, float CrouchSpeed);

	/* Jump Buff */

	FTimerHandle JumpBuffTimer;
	float InitialJumpVelocity;

	void ResetJump();

	UFUNCTION(NetMulticast, Reliable)
	void MulticastJumpBuff(float JumpVelocity);
};
