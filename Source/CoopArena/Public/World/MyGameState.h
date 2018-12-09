// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameState.h"
#include "MyGameState.generated.h"

/**
 * 
 */
UCLASS()
class COOPARENA_API AMyGameState : public AGameState
{
	GENERATED_BODY()
	
private:
	/* Total accumulated score from all players  */
	UPROPERTY(Transient, Replicated)
	int32 _TeamScore;

	UPROPERTY(Transient, Replicated)
	int32 _WaveNumber;
	
public:
	AMyGameState(const class FObjectInitializer& ObjectInitializer);

	UFUNCTION(BlueprintPure, Category = "Game state")
	int32 GetTotalScore() const;

	UFUNCTION(BlueprintPure, Category = "Game state")
	int32 GetWaveNumber() const;

	void SetWaveNumber(int32 WaveNumber);

	/* [Server] Add score to the team score. */
	void AddScore(int32 Score);
};
