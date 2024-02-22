// Copyright Nathan Guerin


#include "CombatComponent.h"

#include "Blaster/Character/BlasterCharacter.h"
#include "Blaster/Weapon/Weapon.h"
#include "Components/SphereComponent.h"
#include "Engine/SkeletalMeshSocket.h"

UCombatComponent::UCombatComponent()
{
	PrimaryComponentTick.bCanEverTick = false;

}

void UCombatComponent::EquipWeapon(AWeapon* WeaponToEquip)
{
	if (Character == nullptr || WeaponToEquip == nullptr) return;

	EquippedWeapon = WeaponToEquip;
	
	EquippedWeapon->SetWeaponState(EWeaponState::EWS_Equipped);

	if (const USkeletalMeshSocket* HandSocket = Character->GetMesh()->GetSocketByName("RightHandSocket"))
	{
		HandSocket->AttachActor(EquippedWeapon, Character->GetMesh());
	}
	
	EquippedWeapon->SetOwner(Character);
}

void UCombatComponent::BeginPlay()
{
	Super::BeginPlay();
	
}
