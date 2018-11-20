// Fill out your copyright notice in the Description page of Project Settings.

#include "World/MyGameState.h"
#include "UnrealNetwork.h"


/////////////////////////////////////////////////////
AMyGameState::AMyGameState(const class FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer) { }

/////////////////////////////////////////////////////
int32 AMyGameState::GetTotalScore() const
{
	return _TeamScore;
}

void AMyGameState::AddScore(int32 Score)
{
	ensureMsgf(HasAuthority(), TEXT("Only the server is allowed to add score."));
	_TeamScore += Score;
}

/////////////////////////////////////////////////////
void AMyGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AMyGameState, _TeamScore);
}