// Fill out your copyright notice in the Description page of Project Settings.

#include "RoundSurvivalGameMode.h"
#include "Engine/World.h"
#include "Engine/PlayerStartPIE.h"
#include "Kismet/GameplayStatics.h"
#include "TeamSpawnPoint.h"


AActor* ARoundSurvivalGameMode::ChoosePlayerStart_Implementation(AController* Player)
{
	AActor* foundPlayerStart = Super::ChoosePlayerStart_Implementation(Player);
	if (foundPlayerStart && foundPlayerStart->IsA<APlayerStartPIE>())
	{
		return foundPlayerStart;
	}
	
	TArray<AActor*> teamSpawnPoints;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ATeamSpawnPoint::StaticClass(), teamSpawnPoints);

	return teamSpawnPoints.Num() > 0 ? teamSpawnPoints[0] : foundPlayerStart;
}