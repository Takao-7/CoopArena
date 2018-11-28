// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "SpawnPoint.h"
#include "TeamSpawnPoint.generated.h"

UCLASS()
class COOPARENA_API ATeamSpawnPoint : public ASpawnPoint
{
	GENERATED_BODY()


public:
	ATeamSpawnPoint(const FObjectInitializer& ObjectInitializer);

protected:
	UPROPERTY(EditDefaultsOnly)
	int32 m_NumSpawnPoints;

	UPROPERTY(VisibleAnywhere)
	TArray<FVector> m_SpawnPoints;	
};
