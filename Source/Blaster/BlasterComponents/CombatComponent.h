// Copyright Nathan Guerin

#pragma once

#include "CoreMinimal.h"
#include "Blaster/BlasterTypes/CombatState.h"
#include "Blaster/UI/BlasterHUD.h"
#include "Blaster/Weapon/WeaponTypes.h"
#include "Components/ActorComponent.h"
#include "CombatComponent.generated.h"

class ABlasterHUD;
class ABlasterPlayerController;
class AWeapon;
class ABlasterCharacter;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class BLASTER_API UCombatComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UCombatComponent();
	friend ABlasterCharacter;

	virtual void BeginPlay() override;

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	void EquipWeapon(AWeapon* WeaponToEquip);

	void DropWeapon();
	
	void Reload();

	UFUNCTION(BlueprintCallable)
	void FinishReload();
	
	void FireButtonPressed(const bool bPressed);
	
protected:
	void SetAiming(bool bIsAiming);

	UFUNCTION(Server, Reliable)
	void ServerSetAiming(bool bIsAiming);

	void Fire();

	UFUNCTION(Server, Reliable)
	void ServerFire(const FVector_NetQuantize& TraceHitTarget);

	UFUNCTION(NetMulticast, Reliable)
	void MulticastFire(const FVector_NetQuantize& TraceHitTarget);

	void TraceUnderCrosshairs(FHitResult& TraceHitResult);

	void SetHUDCrosshairs(float DeltaTime);

	UFUNCTION(Server, Reliable)
	void ServerReload();
	
	void HandleReload() const;
	
	int32 GetAmountToReload();

private:
	UPROPERTY()
	TObjectPtr<ABlasterCharacter> Character;

	UPROPERTY()
	ABlasterPlayerController* Controller;

	UPROPERTY()
	TObjectPtr<ABlasterHUD> HUD;

	UPROPERTY(ReplicatedUsing = OnRep_EquippedWeapon)
	TObjectPtr<AWeapon> EquippedWeapon;

	UFUNCTION()
	void OnRep_EquippedWeapon();

	UPROPERTY(ReplicatedUsing = OnRep_CombatState)
	ECombatState CombatState = ECombatState::ECS_Unoccupied;

	UFUNCTION()
	void OnRep_CombatState();

	UPROPERTY(Replicated)
	bool bAiming;

	bool bFireButtonPressed;

	UPROPERTY(EditAnywhere, Category = "Character Speed")
	float BaseWalkSpeed = 600.f;
	
	UPROPERTY(EditAnywhere, Category = "Character Speed")
	float AimWalkSpeed = 400.f;

	/* HUD and Crosshair */
	FHUDPackage HUDPackage;
	
	float CrosshairVelocityFactor;
	float CrosshairInAirFactor;
	float CrosshairAimFactor;
	float CrosshairShootingFactor;

	FVector HitTarget;

	/* Aiming and FOV */

	// Field of view when not in aiming; set to the camera/s base FOV in BeginPlay
	float DefaultFOV;

	UPROPERTY(EditAnywhere, Category = "Combat")
	float ZoomedFOV = 30.f;

	float CurrentFOV;

	UPROPERTY(EditAnywhere, Category = "Combat")
	float ZoomInterpSpeed = 20.f;

	void InterpFOV(float DeltaTime);

	/* Automatic fire */
	
	FTimerHandle FireTimer;
	bool bCanFire = true;

	bool CanFire() const;

	void StartFireTimer();
	void FireTimerFinished();

	// Carried ammo for the currently equipped weapon
	UPROPERTY(ReplicatedUsing = OnRep_CarriedAmmo)
	int32 CarriedAmmo;

	UFUNCTION()
	void OnRep_CarriedAmmo();
	
	TMap<EWeaponType, int32> CarriedAmmoMap;

	UPROPERTY(EditAnywhere, Category = "Combat", meta=(ForceInlineRow))
	TMap<EWeaponType, int32> StartingCarriedAmmoMap;

	void InitializeCarriedAmmo();

	void UpdateAmmoValues();
};
