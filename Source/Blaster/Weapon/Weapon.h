// Copyright Nathan Guerin

#pragma once

#include "CoreMinimal.h"
#include "WeaponTypes.h"
#include "GameFramework/Actor.h"
#include "Weapon.generated.h"

class ABlasterPlayerController;
class ABlasterCharacter;
class ACasing;
class UWidgetComponent;
class USphereComponent;

UENUM(BlueprintType)
enum class EWeaponState : uint8
{
	EWS_Initial UMETA(DisplayName = "InitialState"),
	EWS_Equipped UMETA(DisplayName = "Equipped"),
	EWS_Dropped UMETA(DisplayName = "Dropped"),
	
	EWS_MAX UMETA(DisplayName = "DefaultMax")
};

UCLASS()
class BLASTER_API AWeapon : public AActor
{
	GENERATED_BODY()
	
public:	
	AWeapon();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	virtual void OnRep_Owner() override;

	void SetHUDAmmo();

	void ShowPickupWidget(const bool bShowWidget) const;

	virtual void Fire(const FVector& HitTarget);

	void Dropped();

	void AddAmmo(const int32 AmmoToAdd);

	/* Getters / Setters */

	void SetWeaponState(const EWeaponState InState);
	FORCEINLINE USphereComponent* GetAreaSphere() const { return AreaSphere; }

	UFUNCTION(BlueprintCallable)
	USkeletalMeshComponent* GetWeaponMesh() const { return WeaponMesh; }

	FORCEINLINE float GetZoomedFOV() const { return ZoomedFOV; }
	FORCEINLINE float GetZoomInterpSpeed() const { return ZoomInterpSpeed; }

	FORCEINLINE int32 GetAmmo() const { return Ammo; }
	FORCEINLINE int32 GetMagCapacity() const { return MagCapacity; }

	bool IsEmpty() const;

	FORCEINLINE EWeaponType GetWeaponType() const { return WeaponType; }

	/* Weapon crosshair textures */
	
	UPROPERTY(EditAnywhere, Category = "WeaponProperties|Crosshairs")
	TObjectPtr<UTexture2D> CrosshairCenter;

	UPROPERTY(EditAnywhere, Category = "WeaponProperties|Crosshairs")
	TObjectPtr<UTexture2D> CrosshairLeft;

	UPROPERTY(EditAnywhere, Category = "WeaponProperties|Crosshairs")
	TObjectPtr<UTexture2D> CrosshairRight;

	UPROPERTY(EditAnywhere, Category = "WeaponProperties|Crosshairs")
	TObjectPtr<UTexture2D> CrosshairTop;

	UPROPERTY(EditAnywhere, Category = "WeaponProperties|Crosshairs")
	TObjectPtr<UTexture2D> CrosshairBottom;

	/* Zoomed FOV while aiming */

	UPROPERTY(EditAnywhere, Category = "WeaponProperties|Zoom")
	float ZoomedFOV = 30.f;

	UPROPERTY(EditAnywhere, Category = "WeaponProperties|Zoom")
	float ZoomInterpSpeed = 20.f;

	/* Automatic fire */

	UPROPERTY(EditAnywhere, Category = "WeaponProperties")
	float FireDelay = 0.15f;

	UPROPERTY(EditAnywhere, Category = "WeaponProperties")
	bool bAutomatic = true;

	UPROPERTY(EditAnywhere, Category = "WeaponProperties|Sounds")
	TObjectPtr<USoundBase> EquipSound;

	UPROPERTY(EditAnywhere, Category = "WeaponProperties|Sounds")
	TObjectPtr<USoundBase> DroppedSound;

protected:
	virtual void BeginPlay() override;

	UFUNCTION()
	virtual void OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void OnSphereEnOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	ABlasterCharacter* GetOwnerCharacter();

	ABlasterPlayerController* GetOwnerController();

private:
	UPROPERTY(VisibleAnywhere, ReplicatedUsing = OnRep_WeaponState, Category = "WeaponProperties", AdvancedDisplay)
	EWeaponState WeaponState;

	UFUNCTION()
	void OnRep_WeaponState();

	UPROPERTY(EditAnywhere, Category = "WeaponProperties")
	EWeaponType WeaponType;

	UPROPERTY(EditAnywhere, ReplicatedUsing = OnRep_Ammo, Category = "WeaponProperties")
	int32 Ammo;

	UFUNCTION()
	void OnRep_Ammo();

	void SpendRound();

	UPROPERTY(EditAnywhere, Category = "WeaponProperties")
	int32 MagCapacity;
	
	UPROPERTY(VisibleAnywhere, Category = "WeaponProperties")
	TObjectPtr<USkeletalMeshComponent> WeaponMesh;

	UPROPERTY(VisibleAnywhere, Category = "WeaponProperties")
	TObjectPtr<USphereComponent> AreaSphere;

	UPROPERTY(VisibleAnywhere, Category = "WeaponProperties")
	TObjectPtr<UWidgetComponent> PickupWidget;

	UPROPERTY(EditAnywhere, Category = "WeaponProperties")
	TObjectPtr<UAnimationAsset> FireAnimation;
	
	UPROPERTY(EditAnywhere, Category = "WeaponProperties")
	TSubclassOf<ACasing> CasingClass;

	/** The name of the socket on the weapon skeletal mesh from where the casing should be spawned */
	UPROPERTY(EditDefaultsOnly, Category = "WeaponProperties")
	FName CasingSpawnSocketName = FName("AmmoEject");

	UPROPERTY()
	TObjectPtr<ABlasterCharacter> BlasterOwnerCharacter;

	UPROPERTY()
	TObjectPtr<ABlasterPlayerController> BlasterOwnerController;
};
