// Fill out your copyright notice in the Description page of Project Settings.

#include "World/MyGameState.h"
#include "UnrealNetwork.h"


/////////////////////////////////////////////////////
AMyGameState::AMyGameState(const class FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{

}

/////////////////////////////////////////////////////
int32 AMyGameState::GetTotalScore() const
{
	return m_TotalScore;
}

void AMyGameState::AddScore(int32 Score)
{
	m_TotalScore += Score;
}

/////////////////////////////////////////////////////
void AMyGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AMyGameState, m_TotalScore);
}