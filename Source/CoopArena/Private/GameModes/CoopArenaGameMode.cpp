#include "CoopArenaGameMode.h"
#include "Engine/World.h"
#include "Engine/PlayerStartPIE.h"
#include "Kismet/GameplayStatics.h"
#include "SpawnPoint.h"
#include "GameFramework/PlayerStart.h"
#include "Humanoid.h"


ACoopArenaGameMode::ACoopArenaGameMode()
{
	m_DefaultPlayerTeam = "Player Team";
	m_DefaultBotTeam = "Bot Team";
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
AActor* ACoopArenaGameMode::FindPlayerStart_Implementation(AController* Player, const FString& IncomingName /*= TEXT("")*/)
{
	if (m_SpawnPoints.Num() == 0)
	{
		return Super::FindPlayerStart(Player, IncomingName);
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
