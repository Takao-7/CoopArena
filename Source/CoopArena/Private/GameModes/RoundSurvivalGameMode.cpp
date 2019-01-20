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
#include "World/CoopArenaGameInstance.h"
#include "Bot.h"
#include "BotController.h"
#include "MyGameState.h"
#include "Engine/Engine.h"
#include "MyPlayerController.h"


ARoundSurvivalGameMode::ARoundSurvivalGameMode()
{
	_CurrentWaveNumber = 0;
	_WaveStrengthIncreaseFactor = 1.0f;
	_BotsToSpawnPerPlayer = 2;
	_WaveLength = 60.0f;
	_PointPenaltyForTeamKill = 50;
	_DelayBetweenWaves = 5.0f;
	bDelayedStart = true;
	_BotDespawnTime = 30.0f;
	_bSpawnLocationLoaded = false;
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
	const int32 nunConnectedPlayers = GetNumberOfConnectedPlayers();
	const bool bIsSinglePlayer = GetCoopArenaGameInstance()->GetOnlineMode() == EOnlineMode::Offline;
	const bool bMostPlayersAreOnMap = NumTravellingPlayers < (nunConnectedPlayers / 2.0f);

	FString message = TEXT("Connected players: " + FString::FromInt(nunConnectedPlayers) + ". ");
	message.Append(TEXT("Travelling players: ") + FString::FromInt(NumTravellingPlayers));
	GetGameInstance()->GetEngine()->AddOnScreenDebugMessage(INDEX_NONE, 2.0f, FColor::Red, message);

	return _bSpawnLocationLoaded && (bIsSinglePlayer || bMostPlayersAreOnMap);
}

bool ARoundSurvivalGameMode::ReadyToEndMatch_Implementation()
{
	return _numPlayersAlive == 0;
}

/////////////////////////////////////////////////////
UCoopArenaGameInstance* ARoundSurvivalGameMode::GetCoopArenaGameInstance()
{
	UCoopArenaGameInstance* gameInstance = Cast<UCoopArenaGameInstance>(GetGameInstance());
	ensure(gameInstance);
	return gameInstance;
}

/////////////////////////////////////////////////////
int32 ARoundSurvivalGameMode::GetNumberOfConnectedPlayers()
{
	return GetCoopArenaGameInstance()->GetNumberOfConnectedPlayers();
}

/////////////////////////////////////////////////////
void ARoundSurvivalGameMode::InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage)
{
	Super::InitGame(MapName, Options, ErrorMessage);

	OnDestroyed.AddDynamic(this, &ARoundSurvivalGameMode::HandleOnDestroyed);
	BotDeath_Event.AddDynamic(this, &ARoundSurvivalGameMode::HandleBotDeath);
	PlayerDeath_Event.AddDynamic(this, &ARoundSurvivalGameMode::HandlePlayerDeath);

	SetBotSpawnChances();
	LoadBotSpawnPoints();
}

/////////////////////////////////////////////////////
void ARoundSurvivalGameMode::SetBotSpawnChances()
{
	TArray<FBotSpawn> uniqueSpawns;
	for (FBotSpawn& spawn : _BotsToSpawn)
	{
		uniqueSpawns.AddUnique(spawn);
	}
	_BotsToSpawn.Empty();

	float totalChange = 0.0f;
	for (FBotSpawn& spawn : uniqueSpawns)
	{
		totalChange += spawn.SpawnChance;
	}

	for (FBotSpawn& spawn : uniqueSpawns)
	{
		spawn.SpawnChance = FMath::GetMappedRangeValueUnclamped(FVector2D(0.0f, totalChange), FVector2D(0.0f, 1.0f), spawn.SpawnChance);
		for (int32 i = 0; i < (int32)(spawn.SpawnChance * 10); ++i)
		{
			_BotsToSpawn.Add(spawn);
		}
	}
}

/////////////////////////////////////////////////////
void ARoundSurvivalGameMode::LoadBotSpawnPoints()
{
	FLatentActionInfo latentActionInfo;
	latentActionInfo.CallbackTarget = this;
	latentActionInfo.ExecutionFunction = TEXT("OnSpawnLocationLoaded");
	latentActionInfo.Linkage = 0;
	latentActionInfo.UUID = 0;
	UGameplayStatics::LoadStreamLevel(GetWorld(), _SpawnLocations[0], true, true, latentActionInfo);
}

/////////////////////////////////////////////////////
void ARoundSurvivalGameMode::Logout(AController* Exiting)
{
	APlayerCharacter* character = Cast<APlayerCharacter>(Exiting->GetPawn());
	if (character)
	{
		character->UnequipWeapon(true, true);
		UnregisterPlayerCharacter(character);
	}
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
	SetBotSpawnChances();
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
void ARoundSurvivalGameMode::HandleOnDestroyed(AActor* DestroyedActor)
{
	GetWorld()->GetTimerManager().ClearAllTimersForObject(DestroyedActor);

	for (APlayerController* pc : _playerControllers)
	{
		pc->SetPawn(nullptr);
		pc->PlayerState->bIsSpectator = false;
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
		AHumanoid* pawn = Cast<AHumanoid>(pc->GetPawn());
		UHealthComponent* healthComp = nullptr;
		if (pawn)
		{
			healthComp = Cast<UHealthComponent>(pawn->GetComponentByClass(UHealthComponent::StaticClass()));
		}
		if (pawn == nullptr || healthComp && !healthComp->IsAlive())
		{
			AMyPlayerController* myPC = Cast<AMyPlayerController>(pc);
			ensure(myPC);

			myPC->StopSpectating();

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
		FBotSpawn& botSpawn = _BotsToSpawn[FMath::RandRange(0, _BotsToSpawn.Num() - 1)];
		TSubclassOf<ABot> botClass = botSpawn.Botclass;
		FActorSpawnParameters params;
		params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
		ABot* newBot = GetWorld()->SpawnActor<ABot>(botClass.Get(), spawnPoint->GetActorLocation(), spawnPoint->GetActorRotation(), params);
		if(newBot)
		{
			_BotsAlive.Add(newBot);

			URespawnComponent* respawnComp = Cast<URespawnComponent>(newBot->GetComponentByClass(URespawnComponent::StaticClass()));
			respawnComp->SetEnableRespawn(false);

			botSpawn.SpawnChance = FMath::Clamp(botSpawn.SpawnChance + botSpawn.SpawnChanceIncreasePerWave, 0.0f, 1.0f);
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
	for (FBotSpawn& botSpawn : _BotsToSpawn)
	{
		if (botSpawn.Botclass == nullptr)
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
		playerState->ScorePoints(bot->GetPointsPerKill());
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
	
	if (Killer && Killer->IsPlayerController())
	{
		AMyPlayerState* playerState_Killer = Cast<AMyPlayerState>(Killer->PlayerState);
		if (playerState_Killer)
		{
			playerState_Killer->ScorePoints(-_PointPenaltyForTeamKill, false);
		}
	}
	
	_playerCharactersAlive.RemoveSwap(DeadPlayer);
	_numPlayersAlive--;

	if (_numPlayersAlive == 0)
	{
		AMyGameState* gamestate = GetGameState<AMyGameState>();
		ensure(gamestate);
		gamestate->OnGameOver();
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
