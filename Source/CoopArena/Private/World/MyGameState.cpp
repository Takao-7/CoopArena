// Fill out your copyright notice in the Description page of Project Settings.

#include "World/MyGameState.h"
#include "UnrealNetwork.h"
#include "GameFramework/GameModeBase.h"
#include "GameFramework/Controller.h"
#include "World/MyPlayerState.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "DefaultHUD.h"
#include "UserWidget.h"
#include "WidgetLayoutLibrary.h"
#include "CoopArenaGameInstance.h"


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
	OnPlayerLogin.Broadcast(playerState);
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
void AMyGameState::OnPostLogin_Multicast_Implementation(AMyPlayerState* NewPlayerState)
{
	OnPlayerLogin.Broadcast(NewPlayerState);
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
void AMyGameState::EndMatch_Implementation()
{
	if (HasAuthority())
	{
		return;
	}

	UCoopArenaGameInstance* gameInstance = Cast<UCoopArenaGameInstance>(GetGameInstance());
	if (gameInstance)
	{
		gameInstance->DestroySession();
	}
}

/////////////////////////////////////////////////////
void AMyGameState::ShowLoadingScreen_Multicast_Implementation()
{
	if (_LoadingScreenClass)
	{
		UWorld* world = GetWorld();
		UWidgetLayoutLibrary::RemoveAllWidgets(world->GetFirstPlayerController());
		UUserWidget* loadingScreen = CreateWidget(world->GetFirstPlayerController(), _LoadingScreenClass, TEXT("Loading screen"));
		loadingScreen->AddToViewport(10);
	}
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