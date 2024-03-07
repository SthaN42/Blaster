// Copyright Nathan Guerin

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Weapon.generated.h"

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

	void ShowPickupWidget(const bool bShowWidget) const;

	void SetWeaponState(const EWeaponState InState);
	FORCEINLINE USphereComponent* GetAreaSphere() const { return AreaSphere; }

	UFUNCTION(BlueprintCallable)
	USkeletalMeshComponent* GetWeaponMesh() const { return WeaponMesh; }

	FORCEINLINE float GetZoomedFOV() const { return ZoomedFOV; }
	FORCEINLINE float GetZoomInterpSpeed() const { return ZoomInterpSpeed; }

	virtual void Fire(const FVector& HitTarget);

	/* Weapon crosshair textures */
	UPROPERTY(EditAnywhere, Category = "Crosshairs")
	TObjectPtr<UTexture2D> CrosshairCenter;

	UPROPERTY(EditAnywhere, Category = "Crosshairs")
	TObjectPtr<UTexture2D> CrosshairLeft;

	UPROPERTY(EditAnywhere, Category = "Crosshairs")
	TObjectPtr<UTexture2D> CrosshairRight;

	UPROPERTY(EditAnywhere, Category = "Crosshairs")
	TObjectPtr<UTexture2D> CrosshairTop;

	UPROPERTY(EditAnywhere, Category = "Crosshairs")
	TObjectPtr<UTexture2D> CrosshairBottom;

	/* Zoomed FOV while aiming */

	UPROPERTY(EditAnywhere, Category = "Zoom")
	float ZoomedFOV = 30.f;

	UPROPERTY(EditAnywhere, Category = "Zoom")
	float ZoomInterpSpeed = 20.f;

protected:
	virtual void BeginPlay() override;

	UFUNCTION()
	virtual void OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void OnSphereEnOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

private:
	UPROPERTY(VisibleAnywhere, Category = "WeaponProperties", ReplicatedUsing = OnRep_WeaponState)
	EWeaponState WeaponState;

	UFUNCTION()
	void OnRep_WeaponState();
	
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

	UPROPERTY(EditDefaultsOnly, Category = "WeaponProperties")
	FName CasingSpawnSocketName;
};
