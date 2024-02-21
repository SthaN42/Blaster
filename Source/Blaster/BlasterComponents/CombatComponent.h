// Copyright Nathan Guerin

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CombatComponent.generated.h"


class AWeapon;
class ABlasterCharacter;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class BLASTER_API UCombatComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UCombatComponent();
	friend ABlasterCharacter;

	void EquipWeapon(AWeapon* WeaponToEquip);
	
protected:
	virtual void BeginPlay() override;

private:
	TObjectPtr<ABlasterCharacter> Character;
	TObjectPtr<AWeapon> EquippedWeapon;
};