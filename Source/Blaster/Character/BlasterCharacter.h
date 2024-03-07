// Copyright Nathan Guerin

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "BlasterCharacter.generated.h"

enum class ETurningInPlace : uint8;
class UCombatComponent;
class AWeapon;
class UWidgetComponent;
class UInputMappingContext;
class UCameraComponent;
class USpringArmComponent;

UCLASS()
class BLASTER_API ABlasterCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	ABlasterCharacter();
	
	virtual void Tick(float DeltaTime) override;

	virtual void PawnClientRestart() override;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	virtual void PostInitializeComponents() override;

	void PlayFireMontage(bool bAiming);

	void EquipButtonPressed();
	void AimButtonPressed();
	void AimButtonReleased();
	void FireButtonPressed(bool bPressed);

	virtual void Jump() override;
	void ToggleCrouch();
	
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

protected:
	virtual void BeginPlay() override;

	void AimOffset(float DeltaTime);

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

	float AO_Yaw;
	float InterpAO_Yaw;
	FRotator StartingAimRotation;

	ETurningInPlace TurningInPlace;
	void TurnInPlace(float DeltaTime);

	UPROPERTY(EditAnywhere, Category = "Combat")
	TObjectPtr<UAnimMontage> FireWeaponMontage;
};
