// Copyright Nathan Guerin

#pragma once

#include "CoreMinimal.h"
#include "InputActionValue.h"
#include "GameFramework/PlayerController.h"
#include "BlasterPlayerController.generated.h"

class ABlasterGameMode;
class UCharacterOverlay;
class ABlasterCharacter;
class ABlasterHUD;
struct FGameplayTag;
struct FInputActionValue;
class UBlasterInputConfig;

/**
 * 
 */
UCLASS()
class BLASTER_API ABlasterPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable)
	ABlasterCharacter* GetBlasterCharacter() const;

	void SetHUDHealth(const float Health, const float MaxHealth);
	void SetHUDShield(const float Shield, const float MaxShield);
	void SetHUDScore(const int32 Score);
	void SetHUDDefeats(const int32 Defeats);
	void SetHUDWeaponAmmo(const int32 Ammo);
	void SetHUDCarriedAmmo(const int32 Ammo);
	void SetHUDAnnouncementCountdown(const uint32 CountdownTime);
	void SetHUDMatchCountdown(const uint32 CountdownTime);
	void SetHUDGrenades(const int32 Grenades);

	virtual void OnPossess(APawn* InPawn) override;
	virtual void ReceivedPlayer() override; // sync with the server clock as soon as possible

	virtual void Tick(float DeltaSeconds) override;
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;

	// Synced with server world clock
	virtual float GetServerTime() const;
	
	void OnMatchStateSet(FName State);

	UPROPERTY(Replicated)
	bool bDisableGameplay = false;

protected:
	virtual void BeginPlay() override;
	
	void SetHUDTime();

	void PollInit();

	void CheckPing(float DeltaSeconds);

	/* Sync time between client and sever */

	// Requests the current sever time, passing in the client's time when the request was sent
	UFUNCTION(Server, Reliable)
	void ServerRequestServerTime(const float TimeOfClientRequest);

	// Reports the current server time to the client in response to ServerRequestServerTime
	UFUNCTION(Client, Reliable)
	void ClientReportServerTime(const float TimeOfClientRequest, const float TimeServerReceivedClientRequest);
	
	UPROPERTY(editdefaultsonly, Category = "Time")
	float TimeSyncFrequency = 5.f;

	// Difference between client and server time
	float ClientServerDelta = 0.f;

	float TimeSyncRunningTime = 0.f;

	void CheckTimeSync(const float DeltaTime);
	
	/* Begin Inputs */
	virtual void SetupInputComponent() override;

	void Input_Move(const FInputActionValue& InputActionValue);
	void Input_Look(const FInputActionValue& InputActionValue);
	void Input_Jump(const FInputActionValue& InputActionValue);
	void Input_Crouch(const FInputActionValue& InputActionValue);
	void Input_Equip(const FInputActionValue& InputActionValue);
	void Input_Drop(const FInputActionValue& InputActionValue);
	void Input_ThrowGrenade(const FInputActionValue& InputActionValue);
	void Input_Reload(const FInputActionValue& InputActionValue);
	void Input_AimPressed(const FInputActionValue& InputActionValue);
	void Input_AimReleased(const FInputActionValue& InputActionValue);
	void Input_FirePressed(const FInputActionValue& InputActionValue);
	void Input_FireReleased(const FInputActionValue& InputActionValue);

	void Input_AbilityInputTagPressed(FGameplayTag InputTag);
	void Input_AbilityInputTagReleased(FGameplayTag InputTag);
	void Input_AbilityInputTagHeld(FGameplayTag InputTag);
	/* End Inputs */

	void HandleMatchIsWaitingToStart();
	void HandleMatchHasStarted();
	void HandleCooldown();

	UFUNCTION(Server, Reliable)
	void ServerCheckMatchState();

	UFUNCTION(Client, Reliable)
	void ClientJoinMidGame(const FName StateOfMatch, const float StartingTime, const float Warmup, const float Match, const float Cooldown);

	void ShowHighPingWarning();

private:
	UPROPERTY(EditDefaultsOnly, Category = Input)
	TObjectPtr<UBlasterInputConfig> InputConfig;

	UPROPERTY()
	ABlasterHUD* BlasterHUD;

	UPROPERTY()
	ABlasterGameMode* BlasterGameMode;

	float LevelStartingTime = 0.f;
	float WarmupTime = 0.f;
	float MatchTime = 0.f;
	float CooldownTime = 0.f;
	uint32 CountdownInt = 0;

	UPROPERTY(ReplicatedUsing = OnRep_MatchState)
	FName MatchState;

	UFUNCTION()
	void OnRep_MatchState();

	/* Overlay */
	
	UPROPERTY()
	UCharacterOverlay* CharacterOverlay;

	float HUDHealth;
	float HUDMaxHealth;
	bool bInitializeHealth = false;
	
	float HUDShield;
	float HUDMaxShield;
	bool bInitializeShield = false;

	int32 HUDCarriedAmmo;
	bool bInitializeCarriedAmmo = false;

	int32 HUDWeaponAmmo;
	bool bInitializeWeaponAmmo = false;

	int32 HUDGrenades;
	bool bInitializeGrenades = false;

	int32 HUDScore;
	bool bInitializeScore = false;
	
	int32 HUDDefeats;
	bool bInitializeDefeats = false;

	/* Ping */

	float HighPingRunningTime = 0.f;

	UPROPERTY(EditDefaultsOnly, Category = "Ping")
	float CheckPingFrequency = 20.f;

	UPROPERTY(EditDefaultsOnly, Category = "Ping")
	float HighPingThreshold = 50.f;
};
