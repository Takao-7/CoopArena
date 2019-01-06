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
#include "Bot.h"
#include "BotController.h"
#include "MyGameState.h"


ARoundSurvivalGameMode::ARoundSurvivalGameMode()
{
	_CurrentWaveNumber = 0;
	_WaveStrengthIncreaseFactor = 1.0f;
	_BotsToSpawnPerPlayer = 2;
	_PointsPerBotKill = 10;
	_WaveLength = 60.0f;
	_PointPenaltyForTeamKill = 50;
	_DelayBetweenWaves = 5.0f;
	bDelayedStart = true;
	_BotDespawnTime = 30.0f;
	_bSpawnLocationLoaded = false;

	if (_SpawnLocations.Num() == 0)
	{
		_SpawnLocations.Add(TEXT("SpawnPoints_North"));
	}
	_ChoosenSpawnLocation = _SpawnLocations[0];
}

/////////////////////////////////////////////////////
void ARoundSurvivalGameMode::OnSpawnLocationLoaded()
{
	FindSpawnPoints();

	for (ASpawnPoint* spawnPoint : _spawnPoints)
	{
		if (spawnPoint->PlayerStartTag == _defaultBotTeam)
		{
			_BotSpawnPoints.AddUnique(spawnPoint);
		}
	}

	_bSpawnLocationLoaded = true;
}

/////////////////////////////////////////////////////
bool ARoundSurvivalGameMode::ReadyToStartMatch_Implementation()
{
	const int32 numPlayersOnMap = _playerControllers.Num();
	const int32 numConnectedPlayers = GetNumberOfConnectedPlayers();

	ENetMode netMode = GetNetMode();
	const bool bIsSinglePlayer = (netMode == ENetMode::NM_Standalone) || numConnectedPlayers == 0;
	const bool bAllPlayersAreOnMap = numPlayersOnMap == numConnectedPlayers;

	return _bSpawnLocationLoaded && (bIsSinglePlayer || bAllPlayersAreOnMap);
}

bool ARoundSurvivalGameMode::ReadyToEndMatch_Implementation()
{
	return _numPlayersAlive == 0;
}

/////////////////////////////////////////////////////
int32 ARoundSurvivalGameMode::GetNumberOfConnectedPlayers()
{
	const UCoopArenaGameInstance* gameInstance = Cast<UCoopArenaGameInstance>(GetGameInstance());
	ensure(gameInstance);
	return gameInstance->GetNumberOfConnectedPlayers();
}

/////////////////////////////////////////////////////
void ARoundSurvivalGameMode::InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage)
{
	Super::InitGame(MapName, Options, ErrorMessage);

	BotDeath_Event.AddDynamic(this, &ARoundSurvivalGameMode::HandleBotDeath);
	PlayerDeath_Event.AddDynamic(this, &ARoundSurvivalGameMode::HandlePlayerDeath);

	FLatentActionInfo latentActionInfo;
	latentActionInfo.CallbackTarget = this;
	latentActionInfo.ExecutionFunction = TEXT("OnSpawnLocationLoaded");
	latentActionInfo.Linkage = 0;
	latentActionInfo.UUID = 0;
	UGameplayStatics::LoadStreamLevel(GetWorld(), _ChoosenSpawnLocation, true, true, latentActionInfo);
}

/////////////////////////////////////////////////////
void ARoundSurvivalGameMode::StartMatch()
{
	Super::StartMatch();
	if (CanSpawnBots() && _bSpawnLocationLoaded)
	{
		FTimerHandle timerhandle;
		GetWorld()->GetTimerManager().SetTimer(timerhandle, [&]() {StartWave(); }, 2.0f, false);
	}
}

/////////////////////////////////////////////////////
void ARoundSurvivalGameMode::StartWave()
{
	if (HasMatchEnded())
	{
		return;
	}

	_CurrentWaveNumber++;
	GetGameState<AMyGameState>()->SetWaveNumber(_CurrentWaveNumber);
	DestroyDeadBotBodies();
	SpawnBots();
	SetAttackTarget();
	GetWorld()->GetTimerManager().SetTimer(_RoundTimerHandle, this, &ARoundSurvivalGameMode::EndWave, _WaveLength);
	OnWaveStart.Broadcast(_CurrentWaveNumber);

	FString text("Wave " + FString::FromInt(_CurrentWaveNumber) + " has started.");
	Broadcast(nullptr, text, NAME_Global);
}

void ARoundSurvivalGameMode::EndWave()
{
	GetWorld()->GetTimerManager().ClearTimer(_RoundTimerHandle);
	SetPlayersHealthToMax();
	ReviveDeadPlayers();
	OnWaveEnd.Broadcast(_CurrentWaveNumber);

	FString text("Wave " + FString::FromInt(_CurrentWaveNumber) + " has ended.");
	Broadcast(nullptr, text, NAME_Global);

	if(IsMatchInProgress())
	{
		FTimerHandle timerHandle;
		GetWorld()->GetTimerManager().SetTimer(timerHandle, this, &ARoundSurvivalGameMode::StartWave, _DelayBetweenWaves);
	}
}

/////////////////////////////////////////////////////
void ARoundSurvivalGameMode::SetPlayersHealthToMax()
{
	for (APlayerCharacter* player : _playerCharactersAlive)
	{
		UHealthComponent* healthComp = Cast<UHealthComponent>(player->GetComponentByClass(UHealthComponent::StaticClass()));
		healthComp->SetHealth(healthComp->GetMaxHealth());
	}
}

/////////////////////////////////////////////////////
void ARoundSurvivalGameMode::SetAttackTarget()
{
	for (ABot* bot : _BotsAlive)
	{
		int32 index = FMath::RandRange(0, _playerCharactersAlive.Num() - 1);
		AController* controller = bot->GetController();
		ABotController* aiController = Cast<ABotController>(controller);
		if (aiController)
		{
			aiController->SetAttackTarget(_playerCharactersAlive[index]->GetActorLocation());
		}
	}
}

/////////////////////////////////////////////////////
void ARoundSurvivalGameMode::DestroyDeadBotBodies()
{
	for (AHumanoid* bot : _BotsDead)
	{
		bot->SetLifeSpan(_BotDespawnTime);
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
void ARoundSurvivalGameMode::SpawnBots()
{
	if (CanSpawnBots() == false)
	{
		return;
	}

	_NumBotsToSpawn += (_BotsToSpawnPerPlayer * GetNumPlayers()) * _WaveStrengthIncreaseFactor;
	const int32 numSpawnPoints = _BotSpawnPoints.Num();
	for (int32 i = 0; i < _NumBotsToSpawn; ++i)
	{
		AActor* spawnPoint = _BotSpawnPoints[i % _BotSpawnPoints.Num()];
		TSubclassOf<ABot> botClassToSpawn = _BotsToSpawn[FMath::RandRange(0, _BotsToSpawn.Num() - 1)];
		FActorSpawnParameters params;
		params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
		ABot* newBot = GetWorld()->SpawnActor<ABot>(botClassToSpawn.Get(), spawnPoint->GetActorLocation(), spawnPoint->GetActorRotation(), params);
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
	const int32 numSpawnPoints = _BotSpawnPoints.Num();
	bool bAllBotClassesAreValid = true;
	for (TSubclassOf<ABot>& botClass : _BotsToSpawn)
	{
		if (botClass == nullptr)
		{
			bAllBotClassesAreValid = false;
			break;
		}
	}

	if (bAllBotClassesAreValid == false || numSpawnPoints == 0 || _BotsToSpawn.Num() == 0)
	{
		UE_LOG(LogTemp, Error, TEXT("No bot spawn points on the map or no bots to spawn."));
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

	ABot* bot = Cast<ABot>(DeadBot);
	ensureMsgf(bot, TEXT("Bot does not inheret from ABot."));

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
	AMyPlayerController* playerController = Cast<AMyPlayerController>(DeadPlayer->GetController());
	ensureMsgf(playerController, TEXT("The player controller does not derive from AMyPlayerController!"));
	StartSpectating(playerController);

	if (HasMatchStarted() == false || _CurrentWaveNumber == 0)
	{
		return;
	}

	APlayerState* playerState = DeadPlayer->PlayerState;
	if(playerState)
	{
		AMyPlayerState* myPlayerState = Cast<AMyPlayerState>(playerState);
		ensureMsgf(myPlayerState, TEXT("Player state does not derive from AMyPlayerState"));
		myPlayerState->AddDeath();		
	}

	UnregisterPlayerCharacter(DeadPlayer);
	
	if (Killer && Killer->IsPlayerController())
	{
		AMyPlayerState* playerState_Killer = Cast<AMyPlayerState>(Killer->PlayerState);
		if (playerState_Killer)
		{
			playerState_Killer->ScorePoints(-_PointPenaltyForTeamKill, false);
		}
	}
	
	_numPlayersAlive--;
	if (_numPlayersAlive == 0)
	{
		// #todo Game over
		EndMatch();
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
