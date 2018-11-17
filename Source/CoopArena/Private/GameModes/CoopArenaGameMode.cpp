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
	m_DefaultPlayerTeam = "Player Team";
	m_DefaultBotTeam = "Bot Team";

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
		m_SpawnPoints.AddUnique(Cast<ASpawnPoint>(spawnPoint));
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
	m_PlayerCharacters.AddUnique(PlayerCharacter);
	m_NumPlayersAlive++;
}

void ACoopArenaGameMode::UnregisterPlayerCharacter(APlayerCharacter* PlayerCharacter)
{
	ensureMsgf(PlayerCharacter, TEXT("PlayerCharacter is null. Do call this function if the parameter is null."));
	m_PlayerCharacters.RemoveSwap(PlayerCharacter);
	m_NumPlayersAlive--;
}

/////////////////////////////////////////////////////
AActor* ACoopArenaGameMode::FindPlayerStart_Implementation(AController* Player, const FString& IncomingName /*= TEXT("")*/)
{
	AActor* playerStart = Super::ChoosePlayerStart_Implementation(Player);
	if (playerStart && playerStart->IsA(APlayerStartPIE::StaticClass()))
	{
		return playerStart;
	}

	if (m_SpawnPoints.Num() == 0)
	{
		return Super::FindPlayerStart_Implementation(Player, IncomingName);
	}

	for (ASpawnPoint* spawnPoint : m_SpawnPoints)
	{
		if (IncomingName.IsEmpty() && spawnPoint->PlayerStartTag != m_DefaultBotTeam)
		{
			return spawnPoint;
		}
		else if (spawnPoint->PlayerStartTag == *IncomingName)
		{
			return spawnPoint;
		}
	}	

	return m_SpawnPoints[FMath::RandRange(0, m_SpawnPoints.Num() - 1)];
}

/////////////////////////////////////////////////////
void ACoopArenaGameMode::PostLogin(APlayerController* NewPlayer)
{
	m_PlayerControllers.AddUnique(NewPlayer);
	Super::PostLogin(NewPlayer);
}

void ACoopArenaGameMode::Logout(AController* Exiting)
{
	m_PlayerControllers.RemoveSwap(Cast<APlayerController>(Exiting));
	m_NumPlayersAlive--;
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
