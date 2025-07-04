// Copyright Nathan Guerin

#pragma once

#include "CoreMinimal.h"
#include "Blaster/BlasterTypes/CombatState.h"
#include "Blaster/Character/BlasterCharacter.h"
#include "Blaster/UI/BlasterHUD.h"
#include "Blaster/Weapon/WeaponTypes.h"
#include "Components/ActorComponent.h"
#include "CombatComponent.generated.h"

class AProjectile;
class ABlasterHUD;
class ABlasterPlayerController;
class AWeapon;

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

	void SwapWeapons();

	void DropWeapon();

	void ThrowGrenade();

	UFUNCTION(BlueprintCallable)
	void ThrowGrenadeFinished();
	
	void Reload();

	UFUNCTION(BlueprintCallable)
	void FinishReload();
	
	void FireButtonPressed(const bool bPressed);

	UFUNCTION(BlueprintCallable)
	void ShotgunShellReload();

	void JumpToShotgunEnd() const;

	UFUNCTION(BlueprintCallable)
	void LaunchGrenade() const;

	UFUNCTION(Server, Reliable)
	void ServerLaunchGrenade(const FVector_NetQuantize& Target) const;

	UFUNCTION(BlueprintCallable, Category = "Pickup")
	void PickupAmmo(EWeaponType WeaponType, int32 AmmoAmount);

	/* Getters / Setters */
	
	FORCEINLINE int32 GetCarriedGrenades() const { return CarriedGrenades; }
	
	FORCEINLINE int32 GetStartingGrenades() const { return StartingGrenades; }

	void SetSpeeds(float InBaseWalkSpeed, float InAimWalkSpeed, float InCrouchSpeed);

	bool ShouldSwapWeapons() const;

	bool bLocallyReloading = false;
	
protected:
	UPROPERTY(EditDefaultsOnly, Category = "Combat|Grenades")
	TSubclassOf<AProjectile> GrenadeClass;
	
	void SetAiming(bool bIsAiming);

	UFUNCTION(Server, Reliable)
	void ServerSetAiming(bool bIsAiming);

	void Fire();
	void FireProjectileWeapon();
	void FireHitScanWeapon();
	void FireShotgunWeapon();
	
	void LocalFire(const FVector_NetQuantize& TraceHitTarget);
	void LocalShotgunFire(const TArray<FVector_NetQuantize>& TraceHitTargets);

	UFUNCTION(Server, Reliable)
	void ServerFire(const FVector_NetQuantize& TraceHitTarget);

	UFUNCTION(NetMulticast, Reliable)
	void MulticastFire(const FVector_NetQuantize& TraceHitTarget);

	UFUNCTION(Server, Reliable)
	void ServerShotgunFire(const TArray<FVector_NetQuantize>& TraceHitTargets);
	
	UFUNCTION(NetMulticast, Reliable)
	void MulticastShotgunFire(const TArray<FVector_NetQuantize>& TraceHitTargets);

	void TraceUnderCrosshairs(FHitResult& TraceHitResult);

	void SetHUDCrosshairs(float DeltaTime);

	UFUNCTION(Server, Reliable)
	void ServerReload();
	
	void HandleReload();

	UFUNCTION(Server, Reliable)
	void ServerThrowGrenade();
	
	int32 GetAmountToReload();

	void AttachActorToSocket(AActor* ActorToAttach, const FName& SocketName) const;
	void AttachActorToLeftHand(AActor* ActorToAttach) const;

	void UpdateCarriedAmmo();

	void DropEquippedWeapon();
	void ReloadEmptyWeapon();
	
	void SetAttachedGrenadeVisibility(const bool bInVisibility) const;

	void EquipPrimaryWeapon(AWeapon* WeaponToEquip);
	void EquipSecondaryWeapon(AWeapon* WeaponToEquip);

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

	UPROPERTY(ReplicatedUsing = OnRep_SecondaryWeapon)
	TObjectPtr<AWeapon> SecondaryWeapon;

	UFUNCTION()
	void OnRep_SecondaryWeapon();

	UPROPERTY(ReplicatedUsing = OnRep_CombatState)
	ECombatState CombatState = ECombatState::ECS_Unoccupied;

	UFUNCTION()
	void OnRep_CombatState();

	UPROPERTY(ReplicatedUsing = OnRep_Aiming)
	bool bAiming = false;

	bool bAimButtonPressed = false;

	UFUNCTION()
	void OnRep_Aiming();

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
	int32 CarriedAmmo = 0;

	UFUNCTION()
	void OnRep_CarriedAmmo();
	
	TMap<EWeaponType, int32> CarriedAmmoMap;

	UPROPERTY(EditDefaultsOnly, Category = "Combat", meta=(ForceInlineRow))
	TMap<EWeaponType, int32> StartingCarriedAmmoMap;

	UPROPERTY(EditDefaultsOnly, Category = "Combat", meta=(ForceInlineRow))
	TMap<EWeaponType, int32> MaxCarriedAmmoMap;

	void InitializeCarriedAmmo();

	void UpdateAmmoValues();
	void UpdateShotgunAmmoValues();

	void UpdateHUDAmmo();

	UPROPERTY(ReplicatedUsing = OnRep_CarriedGrenades)
	int32 CarriedGrenades = 0;

	UFUNCTION()
	void OnRep_CarriedGrenades();

	UPROPERTY(EditDefaultsOnly, Category = "Combat|Grenades")
	int32 StartingGrenades = 4;
	
	UPROPERTY(EditDefaultsOnly, Category = "Combat|Grenades")
	int32 MaxGrenades = 4;

	void UpdateHUDGrenades();
};
