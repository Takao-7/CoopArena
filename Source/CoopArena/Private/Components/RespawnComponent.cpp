// Fill out your copyright notice in the Description page of Project Settings.

#include "RespawnComponent.h"
#include "Components/HealthComponent.h"
#include "Humanoid.h"
#include "GameFramework/Controller.h"
#include "GameModes/CoopArenaGameMode.h"
#include "Engine/World.h"
#include "TimerManager.h"


URespawnComponent::URespawnComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	bAutoActivate = true;

	m_bRespawnAtDeathLocation = false;
	m_bRespawnOnDestroy = false;

	m_bEnableRespawn = true;
	m_RespawnDelay = 2.0f;
	m_IsAlreadyRespawning = false;
}

/////////////////////////////////////////////////////
void URespawnComponent::Respawn()
{
	if (GetOwner()->HasAuthority() && m_bEnableRespawn && !m_IsAlreadyRespawning)
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
	AActor* newActor = SpawnNewActor();
	AController* controller = GetOwner()->GetInstigatorController();
	if (controller)
	{
		controller->Possess(Cast<APawn>(newActor));
	}

	if (m_HealthComp)
	{
		m_HealthComp->Kill();
	}

	OnRespawn.Broadcast(newActor, controller);

	if (m_bDestroyOldActorOnRespawn)
	{
		GetOwner()->Destroy();
	}
}

/////////////////////////////////////////////////////
AActor* URespawnComponent::SpawnNewActor()
{
	const AActor* newSpawnPoint = m_bRespawnAtDeathLocation ? GetOwner() : FindRespawnPoint();
	const FVector location = newSpawnPoint->GetActorLocation();
	const FRotator rotation = newSpawnPoint->GetActorRotation();
	return GetWorld()->SpawnActor(GetOwner()->GetClass(), &location, &rotation);
}

/////////////////////////////////////////////////////
void URespawnComponent::BeginPlay()
{
	Super::BeginPlay();

	m_HealthComp = Cast<UHealthComponent>(GetOwner()->GetComponentByClass(UHealthComponent::StaticClass()));	
	if (m_HealthComp)
	{
		m_HealthComp->OnDeath.AddDynamic(this, &URespawnComponent::HandleOnDeath);
	}	

	const APawn* pawnOwner = Cast<APawn>(GetOwner());
	const AController* controller = GetOwner()->GetInstigatorController();
	if (pawnOwner == nullptr || controller == nullptr)
	{

	}

	if (m_bRespawnOnDestroy || m_HealthComp == nullptr)
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
	if (m_HealthComp && m_HealthComp->IsAlive() || m_HealthComp == nullptr)
	{
		Respawn();
	}
}

/////////////////////////////////////////////////////
void URespawnComponent::HandleOnDeath()
{
	Respawn();
}
