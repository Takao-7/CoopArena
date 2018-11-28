// Fill out your copyright notice in the Description page of Project Settings.

#include "World/MyPlayerState.h"
#include "World/MyGameState.h"
#include "Engine/World.h"
#include "Net/UnrealNetwork.h"


/////////////////////////////////////////////////////
AMyPlayerState::AMyPlayerState(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	_TeamNumber = 0;
}

/////////////////////////////////////////////////////
void AMyPlayerState::Reset()
{
	Super::Reset();

	_NumKills = 0;
	_NumDeaths = 0;
	Score = 0;
}

/////////////////////////////////////////////////////
void AMyPlayerState::ScorePoints(int32 Points, bool bAddKill /*= true*/)
{
	Score += Points;

	AMyGameState* gameState = GetWorld()->GetGameState<AMyGameState>();
	ensureMsgf(gameState, TEXT("Game state is not from class 'AMyGameState'"));
	gameState->AddScore(Points);

	if (bAddKill)
	{
		AddKill();
	}
}

/////////////////////////////////////////////////////
void AMyPlayerState::SetTeamNumber(int32 NewTeamNumber)
{
	_TeamNumber = NewTeamNumber;
}

int32 AMyPlayerState::GetTeamNumber() const
{
	return _TeamNumber;
}

/////////////////////////////////////////////////////
void AMyPlayerState::AddKill()
{
	_NumKills++;
}

int32 AMyPlayerState::GetKills() const
{
	return _NumKills;
}

/////////////////////////////////////////////////////
void AMyPlayerState::AddDeath()
{
	_NumDeaths++;
}

int32 AMyPlayerState::GetDeaths() const
{
	return _NumDeaths;
}

/////////////////////////////////////////////////////
float AMyPlayerState::GetScore() const
{
	return Score;
}

/////////////////////////////////////////////////////
void AMyPlayerState::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AMyPlayerState, _NumKills);
	DOREPLIFETIME(AMyPlayerState, _NumDeaths);
	DOREPLIFETIME(AMyPlayerState, _TeamNumber);
}