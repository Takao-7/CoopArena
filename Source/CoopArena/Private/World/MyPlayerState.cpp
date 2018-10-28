// Fill out your copyright notice in the Description page of Project Settings.

#include "World/MyPlayerState.h"
#include "World/MyGameState.h"
#include "Engine/World.h"
#include "Net/UnrealNetwork.h"


/////////////////////////////////////////////////////
AMyPlayerState::AMyPlayerState(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	m_TeamNumber = 0;
}

/////////////////////////////////////////////////////
void AMyPlayerState::Reset()
{
	Super::Reset();

	m_NumKills = 0;
	m_NumDeaths = 0;
	Score = 0;
}

/////////////////////////////////////////////////////
void AMyPlayerState::ScorePoints(int32 Points)
{
	Score += Points;

	AMyGameState* gameState = GetWorld()->GetGameState<AMyGameState>();
	ensureMsgf(gameState, TEXT("Game state is not from class 'AMyGameState'"));

	gameState->AddScore(Points);
}

/////////////////////////////////////////////////////
void AMyPlayerState::SetTeamNumber(int32 NewTeamNumber)
{
	m_TeamNumber = NewTeamNumber;
}

int32 AMyPlayerState::GetTeamNumber() const
{
	return m_TeamNumber;
}

/////////////////////////////////////////////////////
void AMyPlayerState::AddKill()
{
	m_NumKills++;
}

int32 AMyPlayerState::GetKills() const
{
	return m_NumKills;
}

/////////////////////////////////////////////////////
void AMyPlayerState::AddDeath()
{
	m_NumDeaths++;
}

int32 AMyPlayerState::GetDeaths() const
{
	return m_NumDeaths;
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

	DOREPLIFETIME(AMyPlayerState, m_NumKills);
	DOREPLIFETIME(AMyPlayerState, m_NumDeaths);
	DOREPLIFETIME(AMyPlayerState, m_TeamNumber);
}