// Copyright Nathan Guerin


#include "BlasterPlayerState.h"

#include "BlasterPlayerController.h"
#include "Blaster/Character/BlasterCharacter.h"
#include "Net/UnrealNetwork.h"

void ABlasterPlayerState::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ABlasterPlayerState, Defeats);
}

void ABlasterPlayerState::AddToScore(const int32 ScoreAmount)
{
	SetScore(GetScore() + ScoreAmount);

	if (GetController())
	{
		GetController()->SetHUDScore(FMath::FloorToInt(GetScore()));
	}
}

void ABlasterPlayerState::AddToDefeats(const int32 DefeatsAmount)
{
	SetDefeats(GetDefeats() + DefeatsAmount);

	if (GetController())
	{
		GetController()->SetHUDDefeats(GetDefeats());
	}
}

void ABlasterPlayerState::OnRep_Score()
{
	Super::OnRep_Score();

	if (GetController())
	{
		GetController()->SetHUDScore(FMath::FloorToInt(GetScore()));
	}
}

void ABlasterPlayerState::OnRep_Defeats()
{
	if (GetController())
	{
		GetController()->SetHUDDefeats(GetDefeats());
	}
}

ABlasterCharacter* ABlasterPlayerState::GetCharacter()
{
	if (Character == nullptr)
	{
		Character = Cast<ABlasterCharacter>(GetPawn());
	}
	return Character;
}

ABlasterPlayerController* ABlasterPlayerState::GetController()
{
	if (Controller == nullptr)
	{
		Controller = Cast<ABlasterPlayerController>(GetCharacter()->Controller);
	}
	return Controller;
}

void ABlasterPlayerState::SetDefeats(const int32 NewDefeats)
{
	Defeats = NewDefeats;
}
