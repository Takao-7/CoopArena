// Fill out your copyright notice in the Description page of Project Settings.

#include "SpawnPoint.h"
#include "Components/BoxComponent.h"
#include "GameFramework/Pawn.h"
#include "GameModes/Deathmatch.h"
#include "Engine/World.h"


/////////////////////////////////////////////////////
ASpawnPoint::ASpawnPoint(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	SafeZone = CreateDefaultSubobject<UBoxComponent>(TEXT("SafeZone"));
	SafeZone->SetupAttachment(RootComponent);
	SafeZone->SetBoxExtent(FVector(1000.0f, 1000.0f, 300.0f));

	SafeZone->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

/////////////////////////////////////////////////////
bool ASpawnPoint::IsSafeToSpawn(FName ActorTag) const
{
	TArray<AActor*> overlappingActors;
	SafeZone->GetOverlappingActors(overlappingActors, TSubclassOf<APawn>());

	if (overlappingActors.Num() == 0)
	{
		return true;
	}

	for (AActor* actor : overlappingActors)
	{
		const bool bHasTag = actor->ActorHasTag(ActorTag);
		if (!bHasTag)
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