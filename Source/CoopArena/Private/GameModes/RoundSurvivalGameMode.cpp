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


ARoundSurvivalGameMode::ARoundSurvivalGameMode()
{
	m_CurrentWave = 0;
	m_WaveStrengthIncrease = 2.0f;
	m_BotsToSpawnPerPlayer = 2;
	m_PointsPerBotKill = 10;
	m_WaveLength = 60.0f;
	m_PointPenaltyForTeamKill = 50;
}

/////////////////////////////////////////////////////
bool ARoundSurvivalGameMode::ReadyToStartMatch_Implementation()
{
	return Super::ReadyToStartMatch_Implementation();
}

bool ARoundSurvivalGameMode::ReadyToEndMatch_Implementation()
{
	return numPlayersAlive > 0;
}

/////////////////////////////////////////////////////
void ARoundSurvivalGameMode::InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage)
{
	Super::InitGame(MapName, Options, ErrorMessage);

	for (ASpawnPoint* spawnPoint : spawnPoints)
	{
		if (spawnPoint->PlayerStartTag == defaultBotTeam)
		{
			m_BotSpawnPoints.AddUnique(spawnPoint);
		}
	}

	OnBotDeath_Event.AddDynamic(this, &ARoundSurvivalGameMode::HandleBotDeath);
	OnPlayerDeath_Event.AddDynamic(this, &ARoundSurvivalGameMode::HandlePlayerDeath);
}

/////////////////////////////////////////////////////
void ARoundSurvivalGameMode::StartWave()
{
	m_CurrentWave++;
	SpawnBots();
	GetWorld()->GetTimerManager().SetTimer(m_RoundTimerHandle, this, &ARoundSurvivalGameMode::EndWave, m_WaveLength);
	OnWaveStart_Event.Broadcast();
}

void ARoundSurvivalGameMode::EndWave()
{
	GetWorld()->GetTimerManager().ClearTimer(m_RoundTimerHandle);
	DestroyDeadBotBodies();
	ReviveDeadPlayers();
	OnWaveEnd_Event.Broadcast();
	StartWave();
}

/////////////////////////////////////////////////////
void ARoundSurvivalGameMode::DestroyDeadBotBodies()
{
	for (AHumanoid* bot : m_BotsDead)
	{
		bot->Destroy();
	}
	m_BotsDead.Empty(m_BotsDead.Num() * 2);
}

/////////////////////////////////////////////////////
void ARoundSurvivalGameMode::ReviveDeadPlayers()
{
	for (APlayerController* pc : playerControllers)
	{
		if (pc->PlayerState->bIsSpectator)
		{
			pc->PlayerState->bIsSpectator = false;
			pc->PlayerState->bOnlySpectator = false;
			pc->bPlayerIsWaiting = false;

			AMyPlayerController* myPC = Cast<AMyPlayerController>(pc);
			APlayerCharacter* playerCharacter = myPC->GetLastPossessedCharacter();
			playerCharacter->Revive();
			numPlayersAlive++;
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
	const float waveIncrease = m_CurrentWave > 1 ? m_CurrentWave * m_WaveStrengthIncrease : 1;
	const int32 numBotsToSpawn = m_BotsToSpawnPerPlayer * GetNumPlayers() * waveIncrease;
	const int32 numSpawnPoints = m_BotSpawnPoints.Num();
	if (numSpawnPoints == 0 || numBotsToSpawn == 0)
	{
		UE_LOG(LogTemp, Error, TEXT("No bot spawn points on the map or no bots to spawn."));
		return;
	}

	for (int32 i = 0; i < numBotsToSpawn; ++i)
	{
		AActor* spawnPoint = m_BotSpawnPoints[FMath::RandRange(0, numSpawnPoints - 1)];
		TSubclassOf<AHumanoid> botClassToSpawn = m_BotsToSpawn[FMath::RandRange(0, m_BotsToSpawn.Num() - 1)];
		AHumanoid* newBot = GetWorld()->SpawnActor<AHumanoid>(botClassToSpawn.Get(), spawnPoint->GetActorLocation(), spawnPoint->GetActorRotation());
		if(newBot)
		{
			m_BotsAlive.Add(newBot);

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
void ARoundSurvivalGameMode::HandleBotDeath(AActor* DeadBot, AController* Killer)
{
	if (HasMatchStarted() == false || m_CurrentWave == 0)
	{
		return;
	}

	AHumanoid* bot = Cast<AHumanoid>(DeadBot);

	m_BotsAlive.RemoveSingleSwap(bot);
	m_BotsDead.Add(bot);

	AMyPlayerState* playerState = Cast<AMyPlayerState>(Killer->PlayerState);
	if (playerState)
	{
		playerState->ScorePoints(m_PointsPerBotKill);
	}

	if (GetNumberOfAliveBots() == 0)
	{
		EndWave();
	}
}

void ARoundSurvivalGameMode::HandlePlayerDeath(APlayerCharacter* DeadPlayer, AController* Killer)
{
	if (HasMatchStarted() == false || m_CurrentWave == 0)
	{
		return;
	}
	
	AMyPlayerController* playerController = Cast<AMyPlayerController>(DeadPlayer->GetController());
	ensureMsgf(playerController, TEXT("The player controller does not derive from AMyPlayerController!"));

	AMyPlayerState* playerState = Cast<AMyPlayerState>(DeadPlayer->PlayerState);
	ensureMsgf(playerState, TEXT("Player state does not derive from AMyPlayerState"));
	playerState->AddDeath();

	numPlayersAlive--;
	if (numPlayersAlive == 0)
	{
		// #todo Game over
		EndMatch();
	}

	StartSpectating(playerController);

	if (Killer && Killer->IsPlayerController())
	{
		AMyPlayerState* playerState_Killer = Cast<AMyPlayerState>(Killer->PlayerState);
		playerState_Killer->ScorePoints(-m_PointPenaltyForTeamKill, false);
	}
}

/////////////////////////////////////////////////////
void ARoundSurvivalGameMode::StartSpectating(AMyPlayerController* PlayerController)
{
	AMyPlayerState* playerState = Cast<AMyPlayerState>(PlayerController->PlayerState);
	ensureMsgf(playerState, TEXT("Player state does not derive from AMyPlayerState"));
	
	/* Find alive player to watch. */
	for (APlayerCharacter* pc : playerCharacters)
	{
		if (pc->IsAlive())
		{
			PlayerController->StartSpectating(pc);
		}
	}
}
