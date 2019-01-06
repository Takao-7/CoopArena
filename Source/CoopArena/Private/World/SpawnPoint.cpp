// Fill out your copyright notice in the Description page of Project Settings.

#include "SpawnPoint.h"
#include "Components/SphereComponent.h"
#include "Components/HealthComponent.h"
#include "Components/CapsuleComponent.h"
#include "Humanoid.h"
#include "GameModes/Deathmatch.h"
#include "Engine/World.h"
#include "Engine/Engine.h"


/////////////////////////////////////////////////////
ASpawnPoint::ASpawnPoint(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	m_SafeZoneRadius = 1000.0f;

	SafeZone = CreateDefaultSubobject<USphereComponent>(TEXT("SafeZone"));
	SafeZone->SetupAttachment(RootComponent);
	SafeZone->SetSphereRadius(m_SafeZoneRadius, false);

	SafeZone->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	SafeZone->SetCollisionResponseToAllChannels(ECR_Ignore);
	SafeZone->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);

	m_bAllowBotSpawn = true;
	m_bAllowPlayerSpawn = true;
}

/////////////////////////////////////////////////////
bool ASpawnPoint::IsSafeToSpawn(const FString& TagToCompare) const
{
	TArray<AActor*> overlappingActors;
	SafeZone->GetOverlappingActors(overlappingActors, AHumanoid::StaticClass());

	if (overlappingActors.Num() == 0)
	{
		return true;
	}

	for (AActor* actor : overlappingActors)
	{
		const AHumanoid* humanoid = Cast<AHumanoid>(actor);
		const UHealthComponent* healthComp = Cast<UHealthComponent>(humanoid->GetComponentByClass(UHealthComponent::StaticClass()));
		if (!healthComp->IsAlive())
		{
			continue;
		}
		const FString& teamName = humanoid->GetTeamName();
		const bool bIsSafe = teamName == TagToCompare || teamName == "";
		if (!bIsSafe)
		{
			return false;
		}
	}	

	return true;
}

#if WITH_EDITOR
/////////////////////////////////////////////////////
void ASpawnPoint::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
	SafeZone ? SafeZone->SetSphereRadius(m_SafeZoneRadius, false) : SafeZone = Cast<USphereComponent>(GetComponentByClass(USphereComponent::StaticClass()));
}
#endif

/////////////////////////////////////////////////////
void ASpawnPoint::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	FHitResult hitResult;
	const FVector start = GetActorLocation();
	const FVector end = start + FVector(0.0f, 0.0f, -(m_SafeZoneRadius + 1));

	const bool bHitSomething = GetWorld()->LineTraceSingleByChannel(hitResult, start, end, ECC_Visibility);
	if (bHitSomething)
	{
		const FVector newLocation = hitResult.Location + FVector(0.0f, 0.0f, GetCapsuleComponent()->GetScaledCapsuleHalfHeight());
		SetActorLocation(newLocation);
	}
}

/////////////////////////////////////////////////////
bool ASpawnPoint::IsAllowedToSpawn(AController* Controller)
{
	const bool bIsPlayer = Controller->IsPlayerController();
	return bIsPlayer ? m_bAllowPlayerSpawn : m_bAllowBotSpawn;
}
