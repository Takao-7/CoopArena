#include "CoopArenaGameMode.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "SpawnPoint.h"
#include "Humanoid.h"


ACoopArenaGameMode::ACoopArenaGameMode()
{
	m_DefaultPlayerTeam = "Team1";
	m_DefaultBotTeam = "TeamBots";
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
		SpawnPoints.AddUnique(Cast<ASpawnPoint>(spawnPoint));
	}

}

/////////////////////////////////////////////////////
void ACoopArenaGameMode::RegisterPlayer(APlayerController* Controller)
{
	if (Players.Contains(Controller) || Controller == nullptr)
	{
		return;
	}

	Players.AddUnique(Controller);
	Controller->Tags.AddUnique(m_DefaultPlayerTeam);
}

void ACoopArenaGameMode::RegisterBot(AController* Controller)
{
	if (Bots.Contains(Controller) || Controller == nullptr)
	{
		return;
	}

	Bots.AddUnique(Controller);
	Controller->Tags.AddUnique(m_DefaultBotTeam);
}

/////////////////////////////////////////////////////
AActor* ACoopArenaGameMode::ChoosePlayerStart_Implementation(AController* Player)
{	
	if (Player)
	{
		APlayerController* playerController = Cast<APlayerController>(Player);
		playerController ? RegisterPlayer(playerController) : RegisterBot(Player);
	}

	if (SpawnPoints.Num() == 0)
	{
		return Super::ChoosePlayerStart_Implementation(Player);
	}

	TArray<ASpawnPoint*> freeSpawnPoints;
	if(Player)
	{
		const FString teamTag = CheckForTeamTag(*Player);

		for (ASpawnPoint* point : SpawnPoints)
		{
			const bool bIsSafe = point->IsSafeToSpawn(teamTag) && point->IsAllowedToSpawn(Player);
			if (bIsSafe)
			{
				freeSpawnPoints.AddUnique(point);
			}
		}
	}

	if (freeSpawnPoints.Num() > 0)
	{
		const int32 index = FMath::RandRange(0, freeSpawnPoints.Num() - 1);
		return freeSpawnPoints[index];
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("No free spawn points found!"));

		const int32 index = FMath::RandRange(0, SpawnPoints.Num() - 1);
		return SpawnPoints[index];
	}
}

/////////////////////////////////////////////////////
void ACoopArenaGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);
	RegisterPlayer(NewPlayer);
}

/////////////////////////////////////////////////////
void ACoopArenaGameMode::InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage)
{
	Super::InitGame(MapName, Options, ErrorMessage);
	FindSpawnPoints();
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
