// Fill out your copyright notice in the Description page of Project Settings.

#include "RoundSurvivalGameMode.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "TeamSpawnPoint.h"


AActor* ARoundSurvivalGameMode::ChoosePlayerStart_Implementation(AController* Player)
{
	TArray<AActor*> teamSpawnPoints;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ATeamSpawnPoint::StaticClass(), teamSpawnPoints);
	return teamSpawnPoints[0];
}