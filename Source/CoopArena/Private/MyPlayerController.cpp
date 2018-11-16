#include "MyPlayerController.h"
#include "MyGameState.h"
#include "Engine/World.h"
#include "GameFramework/Controller.h"
#include "MyPlayerState.h"
#include "PlayerCharacter.h"


/////////////////////////////////////////////////////
void AMyPlayerController::StartSpectating(AActor* ActorToSpectate /*= nullptr*/)
{
	ensureMsgf(HasAuthority(), TEXT("Only call this function as the server!"));

	m_DeathLocation = GetPawn()->GetActorLocation();
	m_MyCharacter = Cast<APlayerCharacter>(GetPawn());

	PlayerState->bIsSpectator = true;
	PlayerState->bOnlySpectator = true;
	bPlayerIsWaiting = true;

	APlayerCharacter* playerCharacter = Cast<APlayerCharacter>(ActorToSpectate);
	Possess(playerCharacter);
	SetViewTargetWithBlend(playerCharacter, 1.0f);
	StartSpectating_Client(playerCharacter);
}

void AMyPlayerController::StartSpectating_Client_Implementation(APlayerCharacter* PlayerToSpectate = nullptr)
{
	if (PlayerToSpectate)
	{		
		PlayerToSpectate->SetThirdPersonCameraToActive();		
	}
}

/////////////////////////////////////////////////////
void AMyPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	FInputActionBinding& bindungNextPlayer = InputComponent->BindAction("SpectateNextPlayer", IE_Pressed, this, &AMyPlayerController::SpectateNextPlayer_Server);
	bindungNextPlayer.bConsumeInput = false;

	FInputActionBinding& bindungPreviusPlayer = InputComponent->BindAction("SpectatePreviousPlayer", IE_Pressed, this, &AMyPlayerController::SpectatePreviousPlayer_Server);
	bindungPreviusPlayer.bConsumeInput = false;
}

/////////////////////////////////////////////////////
bool AMyPlayerController::IsSpectating()
{
	return PlayerState->bIsSpectator;
}

/////////////////////////////////////////////////////
const FVector& AMyPlayerController::GetDeathLocation() const
{
	return m_DeathLocation;
}

/////////////////////////////////////////////////////
APlayerCharacter* AMyPlayerController::GetLastPossessedCharacter()
{
	return m_MyCharacter;
}

/////////////////////////////////////////////////////
void AMyPlayerController::SpectateNextPlayer_Server_Implementation()
{
	if (PlayerState->bIsSpectator)
	{
		ViewAPlayer(1);
	}
}

bool AMyPlayerController::SpectateNextPlayer_Server_Validate()
{
	return true;
}

void AMyPlayerController::SpectatePreviousPlayer_Server_Implementation()
{
	if (PlayerState->bIsSpectator)
	{
		ViewAPlayer(-1);
	}
}

bool AMyPlayerController::SpectatePreviousPlayer_Server_Validate()
{
	return true;
}