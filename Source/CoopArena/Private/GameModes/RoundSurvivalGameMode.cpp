// Fill out your copyright notice in the Description page of Project Settings.

#include "RoundSurvivalGameMode.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Engine/PlayerStartPIE.h"
#include "Kismet/GameplayStatics.h"
#include "TeamSpawnPoint.h"
#include "Components/HealthComponent.h"
#include "Components/RespawnComponent.h"
#include "World/MyPlayerState.h"
#include "GameFramework/Controller.h"
#include "PlayerCharacter.h"
#include "MyPlayerController.h"
#include "CoopArenaGameInstance.h"


ARoundSurvivalGameMode::ARoundSurvivalGameMode()
{
	_CurrentWaveNumber = 0;
	_WaveStrengthIncreaseFactor = 2.0f;
	_BotsToSpawnPerPlayer = 2;
	_PointsPerBotKill = 10;
	_WaveLength = 60.0f;
	_PointPenaltyForTeamKill = 50;
	_DelayBetweenWaves = 3.0f;
	bDelayedStart = true;
}

/////////////////////////////////////////////////////
bool ARoundSurvivalGameMode::ReadyToStartMatch_Implementation()
{
	const UCoopArenaGameInstance* gameInstance = Cast<UCoopArenaGameInstance>(GetGameInstance());
	ensure(gameInstance);

	const int32 numConnectedPlayers = gameInstance->GetNumberOfConnectedPlayers();
	const int32 numPlayersOnMap = _playerControllers.Num();
	return (numPlayersOnMap >= numConnectedPlayers) && numPlayersOnMap > 0;
}

bool ARoundSurvivalGameMode::ReadyToEndMatch_Implementation()
{
	return _numPlayersAlive == 0;
}

/////////////////////////////////////////////////////
void ARoundSurvivalGameMode::InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage)
{
	Super::InitGame(MapName, Options, ErrorMessage);

	for (ASpawnPoint* spawnPoint : _spawnPoints)
	{
		if (spawnPoint->PlayerStartTag == _defaultBotTeam)
		{
			_BotSpawnPoints.AddUnique(spawnPoint);
		}
	}

	BotDeath_Event.AddDynamic(this, &ARoundSurvivalGameMode::HandleBotDeath);
	PlayerDeath_Event.AddDynamic(this, &ARoundSurvivalGameMode::HandlePlayerDeath);
}

/////////////////////////////////////////////////////
void ARoundSurvivalGameMode::StartMatch()
{
	Super::StartMatch();
	CanSpawnBots() ? StartWave() : EndMatch();
}

void ARoundSurvivalGameMode::ForceStartMatch()
{
	StartWave();
	Super::StartMatch();
}

/////////////////////////////////////////////////////
void ARoundSurvivalGameMode::StartWave()
{
	if (HasMatchEnded())
	{
		return;
	}

	_CurrentWaveNumber++;
	DestroyDeadBotBodies();
	SpawnBots();
	GetWorld()->GetTimerManager().SetTimer(_RoundTimerHandle, this, &ARoundSurvivalGameMode::EndWave, _WaveLength);
	WaveStart_Event.Broadcast();

	FString text("Wave " + FString::FromInt(_CurrentWaveNumber) + " has started.");
	Broadcast(nullptr, text, NAME_Global);
}

void ARoundSurvivalGameMode::EndWave()
{
	GetWorld()->GetTimerManager().ClearTimer(_RoundTimerHandle);
	ReviveDeadPlayers();
	WaveEnd_Event.Broadcast();

	FString text("Wave " + FString::FromInt(_CurrentWaveNumber) + " has ended.");
	Broadcast(nullptr, text, NAME_Global);

	if(IsMatchInProgress())
	{
		FTimerHandle timerHandle;
		GetWorld()->GetTimerManager().SetTimer(timerHandle, this, &ARoundSurvivalGameMode::StartWave, _DelayBetweenWaves);
	}
}

/////////////////////////////////////////////////////
void ARoundSurvivalGameMode::DestroyDeadBotBodies()
{
	for (AHumanoid* bot : _BotsDead)
	{
		bot->Destroy();
	}
	_BotsDead.Empty(_BotsDead.Num() * 2);
}

/////////////////////////////////////////////////////
void ARoundSurvivalGameMode::ReviveDeadPlayers()
{
	for (APlayerController* pc : _playerControllers)
	{
		if (pc->PlayerState->bIsSpectator)
		{
			pc->PlayerState->bIsSpectator = false;
			pc->PlayerState->bOnlySpectator = false;
			pc->bPlayerIsWaiting = false;

			AMyPlayerController* myPC = Cast<AMyPlayerController>(pc);
			APlayerCharacter* playerCharacter = myPC->GetLastPossessedCharacter();
			playerCharacter->Revive();
			_numPlayersAlive++;
		}
	}
}

/////////////////////////////////////////////////////
AActor* ARoundSurvivalGameMode::FindPlayerStart_Implementation(AController* Player, const FString& IncomingName /*= TEXT("")*/)
{
	return Super::FindPlayerStart_Implementation(Player, IncomingName);
}

AActor* ARoundSurvivalGameMode::ChoosePlayerStart_Implementation(AController* Player)
{
	return Super::ChoosePlayerStart_Implementation(Player);
}

/////////////////////////////////////////////////////
void ARoundSurvivalGameMode::SpawnBots()
{
	if (CanSpawnBots() == false)
	{
		return;
	}

	const float waveIncrease = _CurrentWaveNumber > 1 ? _CurrentWaveNumber * _WaveStrengthIncreaseFactor : 1;
	const int32 numBotsToSpawn = _BotsToSpawnPerPlayer * GetNumPlayers() * waveIncrease;
	const int32 numSpawnPoints = _BotSpawnPoints.Num();
	for (int32 i = 0; i < numBotsToSpawn; ++i)
	{
		AActor* spawnPoint = _BotSpawnPoints[FMath::RandRange(0, numSpawnPoints - 1)];
		TSubclassOf<AHumanoid> botClassToSpawn = _BotsToSpawn[FMath::RandRange(0, _BotsToSpawn.Num() - 1)];
		FActorSpawnParameters params;
		params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
		AHumanoid* newBot = GetWorld()->SpawnActor<AHumanoid>(botClassToSpawn.Get(), spawnPoint->GetActorLocation(), spawnPoint->GetActorRotation(), params);
		if(newBot)
		{
			_BotsAlive.Add(newBot);

			URespawnComponent* respawnComp = Cast<URespawnComponent>(newBot->GetComponentByClass(URespawnComponent::StaticClass()));
			respawnComp->SetEnableRespawn(false);
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("Bot couldn't be spawned."));
		}
	}
}

/////////////////////////////////////////////////////
bool ARoundSurvivalGameMode::CanSpawnBots()
{
	const float waveIncrease = _CurrentWaveNumber > 1 ? _CurrentWaveNumber * _WaveStrengthIncreaseFactor : 1;
	const int32 numBotsToSpawn = _BotsToSpawnPerPlayer * GetNumPlayers() * waveIncrease;
	const int32 numSpawnPoints = _BotSpawnPoints.Num();
	bool bValidBotClasses = true;
	for (TSubclassOf<AHumanoid>& botClass : _BotsToSpawn)
	{
		if (botClass == nullptr)
		{
			bValidBotClasses = false;
			break;
		}
	}

	if (bValidBotClasses == false || numSpawnPoints == 0 || numBotsToSpawn == 0 || _BotsToSpawn.Num() == 0)
	{
		UE_LOG(LogTemp, Error, TEXT("No bot spawn points on the map or no bots to spawn."));
		EndMatch();
		return false;
	}
	else
	{
		return true;
	}
}

/////////////////////////////////////////////////////
void ARoundSurvivalGameMode::HandleBotDeath(AActor* DeadBot, AController* Killer)
{
	if (HasMatchStarted() == false || _CurrentWaveNumber == 0)
	{
		return;
	}

	AHumanoid* bot = Cast<AHumanoid>(DeadBot);

	_BotsAlive.RemoveSingleSwap(bot);
	_BotsDead.Add(bot);

	AMyPlayerState* playerState = Cast<AMyPlayerState>(Killer->PlayerState);
	if (playerState)
	{
		playerState->ScorePoints(_PointsPerBotKill);
	}

	if (GetNumberOfAliveBots() == 0)
	{
		EndWave();
	}
}

void ARoundSurvivalGameMode::HandlePlayerDeath(APlayerCharacter* DeadPlayer, AController* Killer)
{
	if (HasMatchStarted() == false || _CurrentWaveNumber == 0)
	{
		return;
	}
	
	AMyPlayerController* playerController = Cast<AMyPlayerController>(DeadPlayer->GetController());
	ensureMsgf(playerController, TEXT("The player controller does not derive from AMyPlayerController!"));

	AMyPlayerState* playerState = Cast<AMyPlayerState>(DeadPlayer->PlayerState);
	ensureMsgf(playerState, TEXT("Player state does not derive from AMyPlayerState"));
	playerState->AddDeath();

	_numPlayersAlive--;
	if (_numPlayersAlive == 0)
	{
		// #todo Game over
		EndMatch();
	}

	StartSpectating(playerController);

	if (Killer && Killer->IsPlayerController())
	{
		AMyPlayerState* playerState_Killer = Cast<AMyPlayerState>(Killer->PlayerState);
		playerState_Killer->ScorePoints(-_PointPenaltyForTeamKill, false);
	}
}

/////////////////////////////////////////////////////
void ARoundSurvivalGameMode::StartSpectating(AMyPlayerController* PlayerController)
{
	AMyPlayerState* playerState = Cast<AMyPlayerState>(PlayerController->PlayerState);
	ensureMsgf(playerState, TEXT("Player state does not derive from AMyPlayerState"));
	
	/* Find alive player to watch. */
	for (APlayerCharacter* pc : _playerCharacters)
	{
		if (pc->IsAlive())
		{
			PlayerController->StartSpectating(pc);
		}
	}
}
