// Copyright Nathan Guerin

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Blaster/Interfaces/InteractWithCrosshairsInterface.h"
#include "Components/TimelineComponent.h"
#include "BlasterCharacter.generated.h"

class UBuffComponent;
class ABlasterPlayerController;
class ABlasterPlayerState;
enum class ETurningInPlace : uint8;
class UCombatComponent;
class AWeapon;
class UWidgetComponent;
class UInputMappingContext;
class UCameraComponent;
class USpringArmComponent;
class UNiagaraComponent;
enum class ECombatState : uint8;

UCLASS()
class BLASTER_API ABlasterCharacter : public ACharacter, public IInteractWithCrosshairsInterface
{
	GENERATED_BODY()

public:
	ABlasterCharacter();
	
	virtual void Tick(float DeltaSeconds) override;

	virtual void Destroyed() override;

	virtual void PawnClientRestart() override;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	virtual void PostInitializeComponents() override;

	void PlayFireMontage(bool bAiming) const;
	void PlayReloadMontage() const;
	void PlayElimMontage() const;
	void PlayThrowGrenadeMontage() const;

	virtual void OnRep_ReplicatedMovement() override;

	void Elim();

	UFUNCTION(NetMulticast, Reliable)
	void MulticastElim();

	UFUNCTION(BlueprintImplementableEvent)
	void ShowSniperScopeWidget(bool bShowScope);

	void EquipButtonPressed();
	void DropButtonPressed();
	void GrenadeButtonPressed();
	void ReloadButtonPressed();
	void AimButtonPressed();
	void AimButtonReleased();
	void FireButtonPressed(const bool bPressed);

	virtual void Jump() override;
	void ToggleCrouch();

	void SpawnDefaultWeapon() const;

	void UpdateHUDHealth();
	void UpdateHUDShield();
	void UpdateHUDAmmo();

	UPROPERTY()
	ABlasterPlayerState* BlasterPlayerState;

	/* Getters / Setters */

	UFUNCTION(BlueprintCallable)
	ABlasterPlayerController* GetBlasterPlayerController();

	void SetOverlappingWeapon(AWeapon* InWeapon);

	UFUNCTION(BlueprintCallable)
	bool IsWeaponEquipped() const;

	UFUNCTION(BlueprintCallable)
	AWeapon* GetEquippedWeapon() const;
	
	UFUNCTION(BlueprintCallable)
	bool IsAiming() const;

	UFUNCTION(BlueprintCallable)
	bool IsLocallyReloading() const;
	
	UFUNCTION(BlueprintCallable)
	FORCEINLINE float GetAO_Yaw() const { return AO_Yaw; }

	UFUNCTION(BlueprintCallable)
	FORCEINLINE ETurningInPlace GetTurningInPlace() const { return TurningInPlace; }

	UFUNCTION(BlueprintCallable)
	FVector GetHitTarget() const;

	FORCEINLINE UCameraComponent* GetFollowCamera() const { return FollowCamera; }

	UFUNCTION(BlueprintCallable)
	FORCEINLINE bool ShouldRotateRootBone() const { return bRotateRootBone; }

	UFUNCTION(BlueprintCallable)
	FORCEINLINE bool IsEliminated() const { return bEliminated; }

	FORCEINLINE float GetHealth() const { return Health; }
	FORCEINLINE void SetHealth(const float Amount) { Health = Amount; }

	FORCEINLINE float GetMaxHealth() const { return MaxHealth; }

	FORCEINLINE float GetShield() const { return Shield; }
	FORCEINLINE void SetShield(const float Amount) { Shield = Amount; }

	FORCEINLINE float GetMaxShield() const { return MaxShield; }
	
	UFUNCTION(BlueprintCallable)
	ECombatState GetCombatState() const;

	/** Returns the Combat Component of the character. Can be null on the first few frames of the game. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Components")
	FORCEINLINE UCombatComponent* GetCombat() const { return Combat; }
	
	/** Returns the Buff Component of the character. Can be null on the first few frames of the game. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Components")
	FORCEINLINE UBuffComponent* GetBuff() const { return Buff; }

	UFUNCTION(BlueprintCallable)
	bool GetDisableGameplay() const;

	FORCEINLINE UAnimMontage* GetReloadMontage() const { return ReloadMontage; }

	FORCEINLINE UStaticMeshComponent* GetAttachedGrenade() const { return AttachedGrenade; }

protected:
	virtual void BeginPlay() override;

	void AimOffset(float DeltaTime);

	void SimProxiesTurn();

	void PlayHitReactMontage() const;

	UFUNCTION()
	void ReceiveDamage(AActor* DamagedActor, float Damage, const UDamageType* DamageType, class AController* InstigatorController, AActor* DamageCauser);

	// Poll for any relevant classes and initialize our HUD
	void PollInit();

	void RotateInPlace(float DeltaSeconds);

	void DropOrDestroyWeapon(AWeapon* Weapon);

private:
	UPROPERTY(VisibleAnywhere, Category = "Camera")
	TObjectPtr<USpringArmComponent> CameraBoom;

	UPROPERTY(VisibleAnywhere, Category = "Camera")
	TObjectPtr<UCameraComponent> FollowCamera;

	UPROPERTY(EditAnywhere, Category = "Input")
	TObjectPtr<UInputMappingContext> BlasterMappingContext;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Indicators", meta = (AllowPrivateAccess = true))
	TObjectPtr<UWidgetComponent> OverheadWidget;

	UPROPERTY(ReplicatedUsing = OnRep_OverlappingWeapon)
	TObjectPtr<AWeapon> OverlappingWeapon;

	UFUNCTION()
	void OnRep_OverlappingWeapon(AWeapon* LastWeapon);

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, BlueprintGetter = "GetCombat", Category = "Combat", meta = (AllowPrivateAccess = true))
	TObjectPtr<UCombatComponent> Combat;

	UPROPERTY(visibleAnywhere, BlueprintReadOnly, BlueprintGetter = "GetBuff", Category = "Buff", meta = (AllowPrivateAccess = true))
	TObjectPtr<UBuffComponent> Buff;

	UFUNCTION(Server, Reliable)
	void ServerEquipButtonPressed();

	UFUNCTION(Server, Reliable)
	void ServerDropButtonPressed();

	/* Animation */

	bool bRotateRootBone;
	float TurnThreshold = 5.f;
	FRotator ProxyRotationLastFrame;
	FRotator ProxyRotation;
	float ProxyYaw;
	float TimeSinceLastMovementReplication;
	
	float AO_Yaw;
	float InterpAO_Yaw;
	FRotator StartingAimRotation;
	float CalculateSpeed() const;

	ETurningInPlace TurningInPlace;
	void TurnInPlace(float DeltaTime);

	// Montages

	UPROPERTY(EditAnywhere, Category = "Combat|Animations")
	TObjectPtr<UAnimMontage> FireWeaponMontage;

	UPROPERTY(EditAnywhere, Category = "Combat|Animations")
	TObjectPtr<UAnimMontage> ReloadMontage;

	UPROPERTY(EditAnywhere, Category = "Combat|Animations")
	TObjectPtr<UAnimMontage> HitReactMontage;

	UPROPERTY(EditAnywhere, Category = "Combat|Animations")
	TObjectPtr<UAnimMontage> ElimMontage;
	
	UPROPERTY(EditAnywhere, Category = "Combat|Animations")
	TObjectPtr<UAnimMontage> ThrowGrenadeMontage;

	/* Camera position */
	
	float DefaultCameraPosition_Z;
	
	float CurrentCameraPosition_Z;

	UPROPERTY(EditAnywhere, Category = "Camera")
	float CrouchedCameraInterpSpeed = 10.f;
	
	void InterpCameraPosition(float DeltaTime);

	void HideCharacterIfCameraClose() const;

	/* Distance from the camera to the character where we'll start to hide the character for better visibility */
	UPROPERTY(EditAnywhere, Category = "Camera")
	float CameraThreshold = 200.f;

	/* Player Health */
	
	UPROPERTY(EditDefaultsOnly, Category = "Player Stats")
	float MaxHealth = 100.f;

	UPROPERTY(EditDefaultsOnly, ReplicatedUsing = OnRep_Health, Category = "Player Stats")
	float Health = 100.f;

	UFUNCTION()
	void OnRep_Health(float LastHealth);

	UPROPERTY(EditDefaultsOnly, Category = "Player Stats")
	float MaxShield = 100.f;

	UPROPERTY(EditDefaultsOnly, ReplicatedUsing = OnRep_Shield, Category = "Player Stats")
	float Shield = 0.f;

	UFUNCTION()
	void OnRep_Shield(float LastShield);

	bool bEliminated = false;

	FTimerHandle ElimTimer;

	UPROPERTY(EditDefaultsOnly)
	float ElimDelay = 3.f;

	void ElimTimerFinished();

	UPROPERTY()
	ABlasterPlayerController* BlasterPlayerController;

	/* Elim Effect */

	UPROPERTY(EditAnywhere, Category = "Elim Effect")
	USoundBase* ElimSound;

	UPROPERTY(VisibleAnywhere, Category = "Elim Effect")
	UTimelineComponent* DissolveTimeline;

	FOnTimelineFloat DissolveTrack;

	UPROPERTY(EditAnywhere, Category = "Elim Effect")
	UCurveFloat* DissolveCurve;
	
	UPROPERTY(EditAnywhere, Category = "Elim Effect")
	UMaterialInstance* DissolveMaterialInstance;
	
	UPROPERTY(VisibleAnywhere, Category = "Elim Effect", AdvancedDisplay)
	UMaterialInstanceDynamic* DynamicDissolveMaterialInstance;

	UPROPERTY(VisibleAnywhere, Category = "Elim Effect")
	TObjectPtr<UNiagaraComponent> DissolveParticlesSystem;

	UFUNCTION()
	void UpdateDissolveEffect(float DissolveValue);
	void StartDissolveEffect();

	/* Grenade */

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UStaticMeshComponent> AttachedGrenade;

	/* Default Weapon */

	UPROPERTY(EditDefaultsOnly, Category = "Combat")
	TSubclassOf<AWeapon> DefaultWeaponClass;
};
