// Copyright Nathan Guerin


#include "LobbyGameMode.h"

#include "GameFramework/GameStateBase.h"

void ALobbyGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);

	const int32 NumberOfPlayers = GameState.Get()->PlayerArray.Num();
	if (NumberOfPlayers == 2)
	{
		if (UWorld* World = GetWorld())
		{
			bUseSeamlessTravel = true;
			
			FString MapPath;
			GameMap.ToSoftObjectPath().ToString().Split(FString("."), &MapPath, nullptr);
			
			World->ServerTravel(MapPath.Append("?listen"));
		}
	}
}
