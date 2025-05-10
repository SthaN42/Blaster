// Copyright Nathan Guerin


#include "BlasterGameMode.h"

#include "Blaster/Character/BlasterCharacter.h"
#include "GameFramework/PlayerStart.h"
#include "Kismet/GameplayStatics.h"

void ABlasterGameMode::PlayerEliminated(ABlasterCharacter* EliminatedCharacter,
                                        ABlasterPlayerController* VictimController, ABlasterPlayerController* AttackerController)
{
	if (EliminatedCharacter)
	{
		EliminatedCharacter->Elim();
	}
}

void ABlasterGameMode::RequestRespawn(ACharacter* EliminatedCharacter, AController* EliminatedController)
{
	if (EliminatedCharacter)
	{
		EliminatedCharacter->Reset();
		EliminatedCharacter->Destroy();
	}
	if (EliminatedController)
	{
		// Find the PlayerStart that is the most isolated from the other players
		TArray<AActor*> PlayerStarts;
		UGameplayStatics::GetAllActorsOfClass(this, APlayerStart::StaticClass(), PlayerStarts);
		
		TArray<AActor*> Players;
		UGameplayStatics::GetAllActorsOfClass(this, ABlasterCharacter::StaticClass(), Players);

		AActor* SelectedPlayerStart = PlayerStarts[FMath::RandRange(0, PlayerStarts.Num() - 1)];
		double SelectedPlayerStartDistance= 0.0;
		
		for (AActor* PlayerStart : PlayerStarts)
		{
			FVector PlayerStartLocation = PlayerStart->GetActorLocation();
			double ClosestDistance = TNumericLimits<double>::Max();
			
			for (const AActor* Player : Players)
			{
				const double Distance = (Player->GetActorLocation() - PlayerStartLocation).Length();
				if (Distance < ClosestDistance)
				{
					ClosestDistance = Distance;
				}
			}
			if (ClosestDistance >= SelectedPlayerStartDistance)
			{
				SelectedPlayerStartDistance = ClosestDistance;
				SelectedPlayerStart = PlayerStart;
			}
		}
		
		RestartPlayerAtPlayerStart(EliminatedController, SelectedPlayerStart);
	}
}
