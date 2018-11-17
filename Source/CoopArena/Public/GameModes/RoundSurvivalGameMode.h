// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameModes/CoopArenaGameMode.h"
#include "RoundSurvivalGameMode.generated.h"


UCLASS()
class COOPARENA_API ARoundSurvivalGameMode : public ACoopArenaGameMode
{
	GENERATED_BODY()
	
public:
	virtual AActor* ChoosePlayerStart_Implementation(AController* Player) override;

	void StartWave();
	void EndWave();
};