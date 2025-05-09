// Copyright Nathan Guerin

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Blaster/Interfaces/InteractWithCrosshairsInterface.h"
#include "BlasterCharacter.generated.h"

enum class ETurningInPlace : uint8;
class UCombatComponent;
class AWeapon;
class UWidgetComponent;
class UInputMappingContext;
class UCameraComponent;
class USpringArmComponent;

UCLASS()
class BLASTER_API ABlasterCharacter : public ACharacter, public IInteractWithCrosshairsInterface
{
	GENERATED_BODY()

public:
	ABlasterCharacter();
	
	virtual void Tick(float DeltaTime) override;

	virtual void PawnClientRestart() override;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	virtual void PostInitializeComponents() override;

	void PlayFireMontage(bool bAiming) const;
	
	void PlayElimMontage() const;

	virtual void OnRep_ReplicatedMovement() override;

	void Elim();

	UFUNCTION(NetMulticast, Reliable)
	void MulticastElim();

	void EquipButtonPressed();
	void AimButtonPressed();
	void AimButtonReleased();
	void FireButtonPressed(bool bPressed);

	virtual void Jump() override;
	void ToggleCrouch();

	/* Getters / Setters */
	
	void SetOverlappingWeapon(AWeapon* InWeapon);

	UFUNCTION(BlueprintCallable)
	bool IsWeaponEquipped() const;

	UFUNCTION(BlueprintCallable)
	AWeapon* GetEquippedWeapon() const;
	
	UFUNCTION(BlueprintCallable)
	bool IsAiming() const;
	
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

protected:
	virtual void BeginPlay() override;

	void AimOffset(float DeltaTime);

	void SimProxiesTurn();

	void PlayHitReactMontage() const;

	UFUNCTION()
	void ReceiveDamage(AActor* DamagedActor, float Damage, const UDamageType* DamageType, class AController* InstigatorController, AActor* DamageCauser);

	void UpdateHUDHealth();

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

	UPROPERTY(VisibleAnywhere, Category = "Combat")
	TObjectPtr<UCombatComponent> Combat;

	UFUNCTION(Server, Reliable)
	void ServerEquipButtonPressed();

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

	UPROPERTY(EditAnywhere, Category = "Combat")
	TObjectPtr<UAnimMontage> FireWeaponMontage;

	UPROPERTY(EditAnywhere, Category = "Combat")
	TObjectPtr<UAnimMontage> HitReactMontage;

	UPROPERTY(EditAnywhere, Category = "Combat")
	TObjectPtr<UAnimMontage> ElimMontage;

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

	bool bEliminated = false;
	
	UPROPERTY(EditAnywhere, Category = "Player Stats")
	float MaxHealth = 100.f;

	UPROPERTY(ReplicatedUsing = OnRep_Health, VisibleAnywhere, Category = "Player Stats")
	float Health = 100.f;

	UFUNCTION()
	void OnRep_Health();

	class ABlasterPlayerController* BlasterPlayerController;

	FTimerHandle ElimTimer;

	UPROPERTY(EditDefaultsOnly)
	float ElimDelay = 3.f;

	void ElimTimerFinished();
};
