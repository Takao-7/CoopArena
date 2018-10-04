// Fill out your copyright notice in the Description page of Project Settings.

#include "SpawnPoint.h"
#include "Components/BoxComponent.h"
#include "Components/HealthComponent.h"
#include "Humanoid.h"
#include "GameModes/Deathmatch.h"
#include "Engine/World.h"
#include "Engine/Engine.h"


/////////////////////////////////////////////////////
ASpawnPoint::ASpawnPoint(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	SafeZone = CreateDefaultSubobject<UBoxComponent>(TEXT("SafeZone"));
	SafeZone->SetupAttachment(RootComponent);
	SafeZone->SetBoxExtent(FVector(1000.0f, 1000.0f, 300.0f));

	SafeZone->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	SafeZone->SetCollisionResponseToAllChannels(ECR_Ignore);
	SafeZone->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
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

/////////////////////////////////////////////////////
UBoxComponent* ASpawnPoint::GetSafeZone() const
{
	return SafeZone;
}