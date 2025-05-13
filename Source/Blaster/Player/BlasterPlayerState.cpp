// Copyright Nathan Guerin


#include "BlasterPlayerState.h"

#include "BlasterPlayerController.h"
#include "Blaster/Character/BlasterCharacter.h"

void ABlasterPlayerState::AddToScore(const int32 ScoreAmount)
{
	SetScore(GetScore() + ScoreAmount);

	if (GetController())
	{
		GetController()->SetHUDScore(GetScore());
	}
}

void ABlasterPlayerState::OnRep_Score()
{
	Super::OnRep_Score();

	if (GetController())
	{
		GetController()->SetHUDScore(GetScore());
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
