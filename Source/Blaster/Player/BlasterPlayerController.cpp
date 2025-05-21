// Copyright Nathan Guerin


#include "BlasterPlayerController.h"

#include "Blaster/BlasterGameplayTags.h"
#include "Blaster/Character/BlasterCharacter.h"
#include "Blaster/GameMode/BlasterGameMode.h"
#include "Blaster/Input/BlasterInputComponent.h"
#include "Blaster/UI/Announcement.h"
#include "Blaster/UI/BlasterHUD.h"
#include "Blaster/UI/CharacterOverlay.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "GameFramework/Character.h"
#include "GameFramework/GameMode.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"

ABlasterCharacter* ABlasterPlayerController::GetBlasterCharacter() const
{
	return Cast<ABlasterCharacter>(GetCharacter());
}

void ABlasterPlayerController::BeginPlay()
{
	Super::BeginPlay();

	BlasterHUD = Cast<ABlasterHUD>(GetHUD());
	
	ServerCheckMatchState();
}

void ABlasterPlayerController::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ABlasterPlayerController, MatchState);
}

void ABlasterPlayerController::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	SetHUDTime();

	CheckTimeSync(DeltaSeconds);

	PollInit();
}

void ABlasterPlayerController::CheckTimeSync(const float DeltaTime)
{
	TimeSyncRunningTime += DeltaTime;
	if (IsLocalController() && TimeSyncRunningTime >= TimeSyncFrequency)
	{
		ServerRequestServerTime(GetWorld()->GetTimeSeconds());
		TimeSyncRunningTime = 0.f;
	}
}

void ABlasterPlayerController::SetHUDHealth(const float Health, const float MaxHealth)
{
	BlasterHUD = BlasterHUD == nullptr ? Cast<ABlasterHUD>(GetHUD()) : BlasterHUD;
	if (BlasterHUD && BlasterHUD->CharacterOverlay && BlasterHUD->CharacterOverlay->HealthBar && BlasterHUD->CharacterOverlay->HealthText)
	{
		const float HealthPercentage = Health / MaxHealth;
		BlasterHUD->CharacterOverlay->HealthBar->SetPercent(HealthPercentage);
		const FString HealthString = FString::Printf(TEXT("%d/%d"), FMath::CeilToInt(Health), FMath::CeilToInt(MaxHealth));
		BlasterHUD->CharacterOverlay->HealthText->SetText(FText::FromString(HealthString));
	}
	else
	{
		bInitializeCharacterOverlay = true;
		HUDHealth = Health;
		HUDMaxHealth = MaxHealth;
	}
}

void ABlasterPlayerController::SetHUDScore(const int32 Score)
{
	BlasterHUD = BlasterHUD == nullptr ? Cast<ABlasterHUD>(GetHUD()) : BlasterHUD;
	if (BlasterHUD && BlasterHUD->CharacterOverlay && BlasterHUD->CharacterOverlay->ScoreText)
	{
		const FString ScoreString = FString::Printf(TEXT("Score: %d"), Score);
		BlasterHUD->CharacterOverlay->ScoreText->SetText(FText::FromString(ScoreString));
	}
	else
	{
		bInitializeCharacterOverlay = true;
		HUDScore = Score;
	}
}

void ABlasterPlayerController::SetHUDDefeats(const int32 Defeats)
{
	BlasterHUD = BlasterHUD == nullptr ? Cast<ABlasterHUD>(GetHUD()) : BlasterHUD;
	if (BlasterHUD && BlasterHUD->CharacterOverlay && BlasterHUD->CharacterOverlay->DefeatsText)
	{
		const FString DefeatsString = FString::Printf(TEXT("Defeats: %d"), Defeats);
		BlasterHUD->CharacterOverlay->DefeatsText->SetText(FText::FromString(DefeatsString));
	}
	else
	{
		bInitializeCharacterOverlay = true;
		HUDDefeats = Defeats;
	}
}

void ABlasterPlayerController::SetHUDWeaponAmmo(const int32 Ammo)
{
	BlasterHUD = BlasterHUD == nullptr ? Cast<ABlasterHUD>(GetHUD()) : BlasterHUD;
	if (BlasterHUD && BlasterHUD->CharacterOverlay && BlasterHUD->CharacterOverlay->WeaponAmmoText)
	{
		const FString AmmoString = FString::Printf(TEXT("%d"), Ammo);
		BlasterHUD->CharacterOverlay->WeaponAmmoText->SetText(FText::FromString(AmmoString));
	}
}

void ABlasterPlayerController::SetHUDCarriedAmmo(const int32 Ammo)
{
	BlasterHUD = BlasterHUD == nullptr ? Cast<ABlasterHUD>(GetHUD()) : BlasterHUD;
	if (BlasterHUD && BlasterHUD->CharacterOverlay && BlasterHUD->CharacterOverlay->CarriedAmmoText)
	{
		const FString AmmoString = FString::Printf(TEXT("%d"), Ammo);
		BlasterHUD->CharacterOverlay->CarriedAmmoText->SetText(FText::FromString(AmmoString));
	}
}

void ABlasterPlayerController::SetHUDAnnouncementCountdown(const uint32 CountdownTime)
{
	BlasterHUD = BlasterHUD == nullptr ? Cast<ABlasterHUD>(GetHUD()) : BlasterHUD;
	if (BlasterHUD && BlasterHUD->Announcement && BlasterHUD->Announcement->WarmupTime)
	{
		const FString CountdownString = FString::Printf(TEXT("%02d:%02d"), CountdownTime / 60, CountdownTime % 60);
		BlasterHUD->Announcement->WarmupTime->SetText(FText::FromString(CountdownString));
	}
}

void ABlasterPlayerController::SetHUDMatchCountdown(const uint32 CountdownTime)
{
	BlasterHUD = BlasterHUD == nullptr ? Cast<ABlasterHUD>(GetHUD()) : BlasterHUD;
	if (BlasterHUD && BlasterHUD->CharacterOverlay && BlasterHUD->CharacterOverlay->MatchCountdownText)
	{
		const FString CountdownString = FString::Printf(TEXT("%02d:%02d"), CountdownTime / 60, CountdownTime % 60);
		BlasterHUD->CharacterOverlay->MatchCountdownText->SetText(FText::FromString(CountdownString));
	}
}

void ABlasterPlayerController::SetHUDTime()
{
	float TimeLeft = 0.f;

	if (HasAuthority())
	{
		const ABlasterGameMode* GameMode = Cast<ABlasterGameMode>(UGameplayStatics::GetGameMode(this));
		LevelStartingTime = GameMode ? GameMode->LevelStartingTime : LevelStartingTime;
		WarmupTime = GameMode ? GameMode->WarmupTime : WarmupTime;
		MatchTime = GameMode ? GameMode->MatchTime : MatchTime;
	}
	
	if (MatchState == MatchState::WaitingToStart) TimeLeft = WarmupTime - GetServerTime() + LevelStartingTime;
	else if (MatchState == MatchState::InProgress) TimeLeft = WarmupTime + MatchTime - GetServerTime() + LevelStartingTime;
	
	const uint32 SecondsLeft = FMath::CeilToInt(TimeLeft);
	
	if (CountdownInt != SecondsLeft)
	{
		if (MatchState == MatchState::WaitingToStart)
		{
			SetHUDAnnouncementCountdown(SecondsLeft);
		}
		else if (MatchState == MatchState::InProgress)
		{
			SetHUDMatchCountdown(SecondsLeft);
		}
	}
	CountdownInt = SecondsLeft;
}

void ABlasterPlayerController::PollInit()
{
	if (CharacterOverlay == nullptr)
	{
		if (BlasterHUD && BlasterHUD->CharacterOverlay)
		{
			CharacterOverlay = BlasterHUD->CharacterOverlay;
			if (CharacterOverlay)
			{
				SetHUDHealth(HUDHealth, HUDMaxHealth);
				SetHUDScore(HUDScore);
				SetHUDDefeats(HUDDefeats);
			}
		}
	}
}

void ABlasterPlayerController::ServerRequestServerTime_Implementation(const float TimeOfClientRequest)
{
	const float ServerTimeOfReceipt = GetWorld()->GetTimeSeconds();
	ClientReportServerTime(TimeOfClientRequest, ServerTimeOfReceipt);
}

void ABlasterPlayerController::ClientReportServerTime_Implementation(const float TimeOfClientRequest, const float TimeServerReceivedClientRequest)
{
	const float RoundTripTime = GetWorld()->GetTimeSeconds() - TimeOfClientRequest;
	const float CurrentServerTime = TimeServerReceivedClientRequest + (RoundTripTime / 2.f);

	ClientServerDelta = CurrentServerTime - GetWorld()->GetTimeSeconds();
}

float ABlasterPlayerController::GetServerTime() const
{
	if (HasAuthority()) return GetWorld()->GetTimeSeconds();
	
	return GetWorld()->GetTimeSeconds() + ClientServerDelta;
}

void ABlasterPlayerController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	if (const ABlasterCharacter* BlasterCharacter = Cast<ABlasterCharacter>(InPawn))
	{
		SetHUDHealth(BlasterCharacter->GetHealth(), BlasterCharacter->GetMaxHealth());
		SetHUDCarriedAmmo(0);
	}
}

void ABlasterPlayerController::ReceivedPlayer()
{
	Super::ReceivedPlayer();

	if (IsLocalController())
	{
		ServerRequestServerTime(GetWorld()->GetTimeSeconds());
	}
}

void ABlasterPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	UBlasterInputComponent* BlasterInputComponent = CastChecked<UBlasterInputComponent>(InputComponent);

	BlasterInputComponent->BindAbilityActions(InputConfig, this, &ThisClass::Input_AbilityInputTagPressed, &ThisClass::Input_AbilityInputTagReleased, &ThisClass::Input_AbilityInputTagHeld);

	BlasterInputComponent->BindNativeAction(InputConfig, this, BlasterGameplayTags::InputTag_Move, ETriggerEvent::Triggered, &ThisClass::Input_Move, false);
	BlasterInputComponent->BindNativeAction(InputConfig, this, BlasterGameplayTags::InputTag_Look, ETriggerEvent::Triggered, &ThisClass::Input_Look, false);
	BlasterInputComponent->BindNativeAction(InputConfig, this, BlasterGameplayTags::InputTag_Jump, ETriggerEvent::Triggered, &ThisClass::Input_Jump, false);
	BlasterInputComponent->BindNativeAction(InputConfig, this, BlasterGameplayTags::InputTag_Crouch, ETriggerEvent::Triggered, &ThisClass::Input_Crouch, false);
	BlasterInputComponent->BindNativeAction(InputConfig, this, BlasterGameplayTags::InputTag_Equip, ETriggerEvent::Triggered, &ThisClass::Input_Equip, false);
	BlasterInputComponent->BindNativeAction(InputConfig, this, BlasterGameplayTags::InputTag_Drop, ETriggerEvent::Triggered, &ThisClass::Input_Drop, false);
	BlasterInputComponent->BindNativeAction(InputConfig, this, BlasterGameplayTags::InputTag_Reload, ETriggerEvent::Triggered, &ThisClass::Input_Reload, false);
	BlasterInputComponent->BindNativeAction(InputConfig, this, BlasterGameplayTags::InputTag_Aim, ETriggerEvent::Started, &ThisClass::Input_AimPressed, false);
	BlasterInputComponent->BindNativeAction(InputConfig, this, BlasterGameplayTags::InputTag_Aim, ETriggerEvent::Completed, &ThisClass::Input_AimReleased, false);
	BlasterInputComponent->BindNativeAction(InputConfig, this, BlasterGameplayTags::InputTag_Fire, ETriggerEvent::Started, &ThisClass::Input_FirePressed, false);
	BlasterInputComponent->BindNativeAction(InputConfig, this, BlasterGameplayTags::InputTag_Fire, ETriggerEvent::Completed, &ThisClass::Input_FireReleased, false);
}

void ABlasterPlayerController::Input_Move(const FInputActionValue& InputActionValue)
{
	if (!GetPawn()) return;
	
	const FVector2D Value = InputActionValue.Get<FVector2D>();
	const FRotator MovementRotation(0.f, GetControlRotation().Yaw, 0.f);

	if (Value.X != 0.f)
	{
		const FVector MovementDirection = MovementRotation.RotateVector(FVector::RightVector);
		GetPawn()->AddMovementInput(MovementDirection, Value.X);
	}

	if (Value.Y != 0.f)
	{
		const FVector MovementDirection = MovementRotation.RotateVector(FVector::ForwardVector);
		GetPawn()->AddMovementInput(MovementDirection, Value.Y);
	}
}

void ABlasterPlayerController::Input_Look(const FInputActionValue& InputActionValue)
{
	if (!GetPawn()) return;
	
	const FVector2D Value = InputActionValue.Get<FVector2D>();

	if (Value.X != 0.f)
	{
		GetPawn()->AddControllerYawInput(Value.X);
	}
	if (Value.Y != 0.f)
	{
		GetPawn()->AddControllerPitchInput(Value.Y);
	}
}

void ABlasterPlayerController::Input_Jump(const FInputActionValue& InputActionValue)
{
	if (!GetBlasterCharacter()) return;

	GetBlasterCharacter()->Jump();
}

void ABlasterPlayerController::Input_Crouch(const FInputActionValue& InputActionValue)
{
	if (!GetBlasterCharacter()) return;

	GetBlasterCharacter()->ToggleCrouch();
}

void ABlasterPlayerController::Input_Equip(const FInputActionValue& InputActionValue)
{
	if (!GetBlasterCharacter()) return;

	GetBlasterCharacter()->EquipButtonPressed();
}

void ABlasterPlayerController::Input_Drop(const FInputActionValue& InputActionValue)
{
	if (!GetBlasterCharacter()) return;

	GetBlasterCharacter()->DropButtonPressed();
}

void ABlasterPlayerController::Input_Reload(const FInputActionValue& InputActionValue)
{
	if (!GetBlasterCharacter()) return;

	GetBlasterCharacter()->ReloadButtonPressed();
}

void ABlasterPlayerController::Input_AimPressed(const FInputActionValue& InputActionValue)
{
	if (!GetBlasterCharacter()) return;

	GetBlasterCharacter()->AimButtonPressed();
}

void ABlasterPlayerController::Input_AimReleased(const FInputActionValue& InputActionValue)
{
	if (!GetBlasterCharacter()) return;
	
	GetBlasterCharacter()->AimButtonReleased();
}

void ABlasterPlayerController::Input_FirePressed(const FInputActionValue& InputActionValue)
{
	if (!GetBlasterCharacter()) return;

	GetBlasterCharacter()->FireButtonPressed(true);
}

void ABlasterPlayerController::Input_FireReleased(const FInputActionValue& InputActionValue)
{
	if (!GetBlasterCharacter()) return;

	GetBlasterCharacter()->FireButtonPressed(false);
}

void ABlasterPlayerController::Input_AbilityInputTagPressed(FGameplayTag InputTag)
{
}

void ABlasterPlayerController::Input_AbilityInputTagReleased(FGameplayTag InputTag)
{
}

void ABlasterPlayerController::Input_AbilityInputTagHeld(FGameplayTag InputTag)
{
}

void ABlasterPlayerController::OnMatchStateSet(FName State)
{
	MatchState = State;

	if (MatchState == MatchState::WaitingToStart)
	{
		HandleMatchIsWaitingToStart();
	}
	else if (MatchState == MatchState::InProgress)
	{
		HandleMatchHasStarted();
	}
}

void ABlasterPlayerController::OnRep_MatchState()
{
	if (MatchState == MatchState::WaitingToStart)
	{
		HandleMatchIsWaitingToStart();
	}
	else if (MatchState == MatchState::InProgress)
	{
		HandleMatchHasStarted();
	}
}

void ABlasterPlayerController::HandleMatchIsWaitingToStart()
{
	BlasterHUD = BlasterHUD == nullptr ? Cast<ABlasterHUD>(GetHUD()) : BlasterHUD;
	if (BlasterHUD)
	{
		if (BlasterHUD->CharacterOverlay)
		{
			BlasterHUD->CharacterOverlay->SetVisibility(ESlateVisibility::Hidden);
		}
		BlasterHUD->AddAnnouncement();
	}
}

void ABlasterPlayerController::HandleMatchHasStarted()
{
	BlasterHUD = BlasterHUD == nullptr ? Cast<ABlasterHUD>(GetHUD()) : BlasterHUD;
	if (BlasterHUD)
	{
		if (BlasterHUD->Announcement)
		{
			BlasterHUD->Announcement->SetVisibility(ESlateVisibility::Hidden);
		}
		BlasterHUD->AddCharacterOverlay();
	}
}

void ABlasterPlayerController::ServerCheckMatchState_Implementation()
{
	if (const ABlasterGameMode* GameMode = Cast<ABlasterGameMode>(UGameplayStatics::GetGameMode(this)))
	{
		LevelStartingTime = GameMode->LevelStartingTime;
		WarmupTime = GameMode->WarmupTime;
		MatchTime = GameMode->MatchTime;
		MatchState = GameMode->GetMatchState();

		ClientJoinMidGame(MatchState, LevelStartingTime, WarmupTime, MatchTime);
	}
}

void ABlasterPlayerController::ClientJoinMidGame_Implementation(const FName StateOfMatch, const float StartingTime, const float Warmup, const float Match)
{
	LevelStartingTime = StartingTime;
	WarmupTime = Warmup;
	MatchTime = Match;
	MatchState = StateOfMatch;
	OnMatchStateSet(MatchState);
}
