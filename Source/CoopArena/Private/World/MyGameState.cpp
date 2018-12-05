// Fill out your copyright notice in the Description page of Project Settings.

#include "World/MyGameState.h"
#include "UnrealNetwork.h"
#include "GameFramework/GameModeBase.h"
#include "GameFramework/Controller.h"
#include "World/MyPlayerState.h"


/////////////////////////////////////////////////////
AMyGameState::AMyGameState(const class FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{ 
	FGameModeEvents::GameModePostLoginEvent.AddUObject(this, &AMyGameState::HandleOnPostLogin);
	FGameModeEvents::GameModeLogoutEvent.AddUObject(this, &AMyGameState::HandleOnLogout);
}

/////////////////////////////////////////////////////
int32 AMyGameState::GetTotalScore() const
{
	return _TeamScore;
}

int32 AMyGameState::GetWaveNumber() const
{
	return _WaveNumber;
}

/////////////////////////////////////////////////////
void AMyGameState::SetWaveNumber(int32 WaveNumber)
{
	_WaveNumber = WaveNumber;
}

/////////////////////////////////////////////////////
void AMyGameState::AddScore(int32 Score)
{
	ensureMsgf(HasAuthority(), TEXT("Only the server is allowed to add score."));
	_TeamScore += Score;
}

/////////////////////////////////////////////////////
void AMyGameState::HandleOnPostLogin_Implementation(AGameModeBase* GameMode, APlayerController* NewPlayer)
{
	OnPlayerLogin.Broadcast(Cast<AMyPlayerState>(NewPlayer->PlayerState));
}

void AMyGameState::HandleOnLogout_Implementation(AGameModeBase* GameMode, AController* Exiting)
{
	OnPlayerLogout.Broadcast(Cast<AMyPlayerState>(Exiting->PlayerState));
}

/////////////////////////////////////////////////////
void AMyGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AMyGameState, _TeamScore);
	DOREPLIFETIME(AMyGameState, _WaveNumber);
}