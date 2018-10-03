#include "CoopArenaGameMode.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "SpawnPoint.h"


/////////////////////////////////////////////////////
void ACoopArenaGameMode::FindSpawnPoints()
{
	TArray<AActor*> spawnPoint_actors;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ASpawnPoint::StaticClass(), spawnPoint_actors);

	for (AActor* spawnPoint : spawnPoint_actors)
	{
		SpawnPoints.AddUnique(Cast<ASpawnPoint>(spawnPoint));
	}
}

/////////////////////////////////////////////////////
void ACoopArenaGameMode::RegisterSpawnPoint(ASpawnPoint* SpawnPoint)
{
	ensure(SpawnPoint);
	SpawnPoints.AddUnique(SpawnPoint);
}

/////////////////////////////////////////////////////
AActor* ACoopArenaGameMode::GetPlayerStart(AController* Player, const FString& IncomingName /* = TEXT("") */)
{
	if (SpawnPoints.Num() == 0)
	{
		FindSpawnPoints();

		if (SpawnPoints.Num() == 0)
		{
			UE_LOG(LogTemp, Error, TEXT("No spawn points found!"));
			return nullptr;
		}
	}
	
	TArray<ASpawnPoint*> freeSpawnPoints;

	for (ASpawnPoint* point : SpawnPoints)
	{
		const bool bIsSafe = point->IsSafeToSpawn(FName(*IncomingName));
		if (bIsSafe)
		{
			freeSpawnPoints.AddUnique(point);
		}
	}

	if (freeSpawnPoints.Num() > 0)
	{
		const int32 index = FMath::RandRange(0, freeSpawnPoints.Num() - 1);
		return freeSpawnPoints[index];
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("No free spawn points found!"));

		const int32 index = FMath::RandRange(0, SpawnPoints.Num() - 1);
		return SpawnPoints[index];
	}
}