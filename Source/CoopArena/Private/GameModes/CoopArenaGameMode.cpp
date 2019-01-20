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
#include "DefaultHUD.h"
#include "Engine/GameInstance.h"
#include "Engine/Engine.h"
#include "Bot.h"
#include "MyPlayerController.h"


ACoopArenaGameMode::ACoopArenaGameMode()
{
	_defaultPlayerTeam = "Player Team";
	_defaultBotTeam = "Bot Team";
	bUseSeamlessTravel = true;
	_numPlayersAlive = 0;

	DefaultPawnClass = APlayerCharacter::StaticClass();
	GameStateClass = AMyGameState::StaticClass();
	PlayerStateClass = AMyPlayerState::StaticClass();
	PlayerControllerClass = AMyPlayerController::StaticClass();
	HUDClass = ADefaultHUD::StaticClass();
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
		_spawnPoints.AddUnique(Cast<ASpawnPoint>(spawnPoint));
	}
}

/////////////////////////////////////////////////////
void ACoopArenaGameMode::DestroyAllBots()
{
	TArray<AActor*> bots;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ABot::StaticClass(), bots);

	for (AActor* bot : bots)
	{
		bot->Destroy();
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
	_playerCharacters.AddUnique(PlayerCharacter);
	_playerCharactersAlive.AddUnique(PlayerCharacter);
	_numPlayersAlive++;
}

void ACoopArenaGameMode::UnregisterPlayerCharacter(APlayerCharacter* PlayerCharacter)
{
	if(_playerCharactersAlive.Find(PlayerCharacter) && _playerCharacters.Find(PlayerCharacter))
	{
		ensureMsgf(PlayerCharacter, TEXT("PlayerCharacter is null. Do call this function if the parameter is null."));
		_playerCharacters.RemoveSwap(PlayerCharacter);
		_playerCharactersAlive.RemoveSwap(PlayerCharacter);
		_numPlayersAlive--;
	}
}

/////////////////////////////////////////////////////
AActor* ACoopArenaGameMode::FindPlayerStart_Implementation(AController* Player, const FString& IncomingName /*= TEXT("")*/)
{
	AActor* playerStart = Super::ChoosePlayerStart_Implementation(Player);
	if (playerStart && playerStart->IsA(APlayerStartPIE::StaticClass()))
	{
		return playerStart;
	}

	if (_spawnPoints.Num() == 0)
	{
		return Super::FindPlayerStart_Implementation(Player, IncomingName);
	}

	for (ASpawnPoint* spawnPoint : _spawnPoints)
	{
		if (IncomingName.IsEmpty() && spawnPoint->PlayerStartTag != _defaultBotTeam)
		{
			return spawnPoint;
		}
		else if (spawnPoint->PlayerStartTag == *IncomingName)
		{
			return spawnPoint;
		}
	}	

	return _spawnPoints[FMath::RandRange(0, _spawnPoints.Num() - 1)];
}

/////////////////////////////////////////////////////
void ACoopArenaGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);
	_playerControllers.AddUnique(NewPlayer);

	AMyPlayerState* state = Cast<AMyPlayerState>(NewPlayer->PlayerState);
	if (state)
	{
		state->RequestPlayerNameUpdate_Client();
	}
}

void ACoopArenaGameMode::Logout(AController* Exiting)
{
	Super::Logout(Exiting);
	_playerControllers.RemoveSwap(Cast<APlayerController>(Exiting));
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
