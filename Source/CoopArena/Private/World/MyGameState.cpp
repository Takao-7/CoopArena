// Fill out your copyright notice in the Description page of Project Settings.

#include "World/MyGameState.h"
#include "UnrealNetwork.h"
#include "GameFramework/GameModeBase.h"
#include "GameFramework/Controller.h"
#include "World/MyPlayerState.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "DefaultHUD.h"


AMyGameState::AMyGameState(const class FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{ 
	if (HasAuthority())
	{
		FGameModeEvents::GameModePostLoginEvent.AddUObject(this, &AMyGameState::HandleOnPostLogin);
		FGameModeEvents::GameModeLogoutEvent.AddUObject(this, &AMyGameState::HandleOnLogout);
	}
}

/////////////////////////////////////////////////////
void AMyGameState::HandleOnPostLogin(AGameModeBase* GameMode, APlayerController* NewPlayer)
{
	AMyPlayerState* playerState = Cast<AMyPlayerState>(NewPlayer->PlayerState);
	if (playerState)
	{
		playerState->RequestPlayerName_Client();
	}
}

void AMyGameState::HandleOnLogout(AGameModeBase* GameMode, AController* Exiting)
{
	AMyPlayerState* playerState = Cast<AMyPlayerState>(Exiting);
	if (playerState)
	{
		HandleOnLogout_Multicast(playerState);
	}
}

/////////////////////////////////////////////////////
void AMyGameState::OnPostLogin_Multicast_Implementation(AMyPlayerState* NewPlayerState, const FString& PlayerName)
{
	/*
	 * Set the player name only on clients. At this point the player name should be already set on the server, but
	 * may not be replicated to this client, so we just set it here.
	 */
	if (HasAuthority() == false && NewPlayerState->GetPlayerName() != PlayerName)
	{
		NewPlayerState->SetPlayerName(PlayerName);
	}
	OnPlayerLogout.Broadcast(Cast<AMyPlayerState>(NewPlayerState));
}

void AMyGameState::HandleOnLogout_Multicast_Implementation(AMyPlayerState* PlayerState)
{
	OnPlayerLogout.Broadcast(PlayerState);
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
void AMyGameState::OnGameOver_Implementation()
{
	APlayerController* playerController = GetWorld()->GetFirstPlayerController();
	if (playerController)
	{
		ADefaultHUD* hud = Cast<ADefaultHUD>(playerController->GetHUD());
		if (hud)
		{
			hud->SetState(EHUDState::MatchEnd);
		}
	}
}

/////////////////////////////////////////////////////
void AMyGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AMyGameState, _TeamScore);
	DOREPLIFETIME(AMyGameState, _WaveNumber);
}