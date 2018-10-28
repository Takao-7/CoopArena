#include "CoopArenaGameMode.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "SpawnPoint.h"
#include "GameFramework/PlayerStart.h"
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
	RegisterPlayer(Cast<APlayerController>(Player));
	return Super::ChoosePlayerStart_Implementation(Player);
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
