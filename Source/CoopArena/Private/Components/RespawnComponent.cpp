// Fill out your copyright notice in the Description page of Project Settings.

#include "RespawnComponent.h"
#include "Components/HealthComponent.h"
#include "Humanoid.h"
#include "GameFramework/Controller.h"
#include "GameModes/CoopArenaGameMode.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "MyPlayerController.h"


URespawnComponent::URespawnComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	bAutoActivate = true;

	_bRespawnAtDeathLocation = false;
	m_bRespawnOnDestroy = false;

	m_bEnableRespawn = true;
	m_RespawnDelay = 0.0f;
	m_IsAlreadyRespawning = false;
}

/////////////////////////////////////////////////////
void URespawnComponent::Respawn()
{
	if (GetOwner()->HasAuthority() && !m_IsAlreadyRespawning)
	{
		m_IsAlreadyRespawning = true;
		if (m_RespawnDelay > 0.0f)
		{
			FTimerHandle timerhandle;
			GetWorld()->GetTimerManager().SetTimer(timerhandle, this, &URespawnComponent::HandleRespawn, m_RespawnDelay, false);
		}
		else
		{
			HandleRespawn();
		}
	}
}

void URespawnComponent::HandleRespawn()
{
	AActor* newActor = Cast<APawn>(SpawnNewActor());
	APawn* newPawn = Cast<APawn>(newActor);
	if(newPawn)
	{
		if (_MyPlayerController)
		{
			_MyPlayerController->OnPossess(newPawn);
			_MyPlayerController->SetViewTarget(newPawn);
		}
		else
		{
			AMyPlayerController* controller = Cast<AMyPlayerController>(GetOwner()->GetInstigatorController());
			if (controller)
			{
				controller->OnPossess(newPawn);
			}
		}
	}
	
	if (_HealthComp)
	{
		_HealthComp->Kill(nullptr);
	}

	OnRespawn.Broadcast(newActor, _MyPlayerController);

	if (_bDestroyOldActorOnRespawn)
	{
		GetOwner()->Destroy();
	}
}

/////////////////////////////////////////////////////
AActor* URespawnComponent::SpawnNewActor()
{
	const AActor* newSpawnPoint = _bRespawnAtDeathLocation ? GetOwner() : FindRespawnPoint();
	const FVector& location = newSpawnPoint->GetActorLocation();
	const FRotator& rotation = newSpawnPoint->GetActorRotation();
	return GetWorld()->SpawnActor(GetOwner()->GetClass(), &location, &rotation);
}

/////////////////////////////////////////////////////
void URespawnComponent::BeginPlay()
{
	Super::BeginPlay();

	_HealthComp = Cast<UHealthComponent>(GetOwner()->GetComponentByClass(UHealthComponent::StaticClass()));	
	if (_HealthComp)
	{
		_HealthComp->OnDeath.AddDynamic(this, &URespawnComponent::HandleOnDeath);
	}	

	if (m_bRespawnOnDestroy || _HealthComp == nullptr)
	{
		GetOwner()->OnDestroyed.AddDynamic(this, &URespawnComponent::HandleOnDestroy);
	}
}

/////////////////////////////////////////////////////
AActor* URespawnComponent::FindRespawnPoint()
{
	ACoopArenaGameMode* gameMode = Cast<ACoopArenaGameMode>(GetWorld()->GetAuthGameMode());
	ensure(gameMode);

	const FString teamName = Cast<AHumanoid>(GetOwner())->GetTeamName();
	return gameMode->ChoosePlayerStart(GetOwner()->GetInstigatorController());
}

/////////////////////////////////////////////////////
void URespawnComponent::HandleOnDestroy(AActor* DestroyedActor)
{
	_MyPlayerController = Cast<AMyPlayerController>(GetOwner()->GetInstigatorController());
	if(CanRespawn())
	{
		if (_HealthComp && _HealthComp->IsAlive() || _HealthComp == nullptr)
		{
			Respawn();
		}
	}
}

/////////////////////////////////////////////////////
void URespawnComponent::HandleOnDeath(AActor* Actor, AController* Controller, AController* Killer)
{
	_MyPlayerController = Cast<AMyPlayerController>(Controller);
	if (CanRespawn())
	{
		Respawn();
	}
}

bool URespawnComponent::CanRespawn()
{
	ACoopArenaGameMode* gameMode = GetWorld()->GetAuthGameMode<ACoopArenaGameMode>();
	APlayerController* playerController = GetOwner()->GetInstigator<APlayerController>();
	const bool bGameModeAllowsRespawn = gameMode->CanRespawn(playerController, GetOwner());
	return bGameModeAllowsRespawn && m_bEnableRespawn;
}
