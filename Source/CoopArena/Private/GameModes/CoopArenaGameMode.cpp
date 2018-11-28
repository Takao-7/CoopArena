#include "CoopArenaGameMode.h"
#include "Engine/World.h"
#include "Engine/PlayerStartPIE.h"
#include "Kismet/GameplayStatics.h"
#include "SpawnPoint.h"
#include "GameFramework/PlayerStart.h"
#include "Humanoid.h"
#include "PlayerCharacter.h"
#include "MyGameState.h"
#include "MyPlayerState.h"
#include "MyPlayerController.h"


ACoopArenaGameMode::ACoopArenaGameMode()
{
	defaultPlayerTeam = "Player Team";
	defaultBotTeam = "Bot Team";

	DefaultPawnClass = APlayerCharacter::StaticClass();
	GameStateClass = AMyGameState::StaticClass();
	PlayerStateClass = AMyPlayerState::StaticClass();
	PlayerControllerClass = AMyPlayerController::StaticClass();
}

/////////////////////////////////////////////////////
void ACoopArenaGameMode::FindSpawnPoints()
{
	TArray<AActor*> spawnPoint_actors;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ASpawnPoint::StaticClass(), spawnPoint_actors);

	if (spawnPoint_actors.Num() == 0)
	{
		UE_LOG(LogTemp, Error, TEXT("No spawn points on the map!"));
		return;
	}

	for (AActor* spawnPoint : spawnPoint_actors)
	{
		spawnPoints.AddUnique(Cast<ASpawnPoint>(spawnPoint));
	}
}

/////////////////////////////////////////////////////
void ACoopArenaGameMode::InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage)
{
	Super::InitGame(MapName, Options, ErrorMessage);
	FindSpawnPoints();
}

/////////////////////////////////////////////////////
void ACoopArenaGameMode::RegisterPlayerCharacter(APlayerCharacter* PlayerCharacter)
{
	ensureMsgf(PlayerCharacter, TEXT("PlayerCharacter is null. Do call this function if the parameter is null."));
	playerCharacters.AddUnique(PlayerCharacter);
	numPlayersAlive++;
}

void ACoopArenaGameMode::UnregisterPlayerCharacter(APlayerCharacter* PlayerCharacter)
{
	ensureMsgf(PlayerCharacter, TEXT("PlayerCharacter is null. Do call this function if the parameter is null."));
	playerCharacters.RemoveSwap(PlayerCharacter);
	numPlayersAlive--;
}

/////////////////////////////////////////////////////
AActor* ACoopArenaGameMode::FindPlayerStart_Implementation(AController* Player, const FString& IncomingName /*= TEXT("")*/)
{
	AActor* playerStart = Super::ChoosePlayerStart_Implementation(Player);
	if (playerStart && playerStart->IsA(APlayerStartPIE::StaticClass()))
	{
		return playerStart;
	}

	if (spawnPoints.Num() == 0)
	{
		return Super::FindPlayerStart_Implementation(Player, IncomingName);
	}

	for (ASpawnPoint* spawnPoint : spawnPoints)
	{
		if (IncomingName.IsEmpty() && spawnPoint->PlayerStartTag != defaultBotTeam)
		{
			return spawnPoint;
		}
		else if (spawnPoint->PlayerStartTag == *IncomingName)
		{
			return spawnPoint;
		}
	}	

	return spawnPoints[FMath::RandRange(0, spawnPoints.Num() - 1)];
}

/////////////////////////////////////////////////////
void ACoopArenaGameMode::PostLogin(APlayerController* NewPlayer)
{
	playerControllers.AddUnique(NewPlayer);
	Super::PostLogin(NewPlayer);
}

void ACoopArenaGameMode::Logout(AController* Exiting)
{
	playerControllers.RemoveSwap(Cast<APlayerController>(Exiting));
	Super::Logout(Exiting);
}

/////////////////////////////////////////////////////
bool ACoopArenaGameMode::CanRespawn(APlayerController* PlayerController, AActor* Actor) const
{
	ensureMsgf(PlayerController || Actor, TEXT("Both parameters are null. At least one of them must be not-null. "));
	const bool bIsNotAPawn = Actor && Actor->IsA(APawn::StaticClass()) == false;
	return bIsNotAPawn;
}

/////////////////////////////////////////////////////
FString ACoopArenaGameMode::CheckForTeamTag(const AController& Controller) const
{
	FString teamTag;
	for (FName tag : Controller.Tags)
	{
		if (tag.ToString().Contains("Team"))
		{
			teamTag = tag.ToString();
		}
	}
	return teamTag;
}
