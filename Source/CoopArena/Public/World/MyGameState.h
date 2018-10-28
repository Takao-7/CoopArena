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
	UPROPERTY(Replicated)
	int32 m_TotalScore;
	
public:
	AMyGameState(const class FObjectInitializer& ObjectInitializer);

	UFUNCTION(BlueprintPure, Category = "Game state")
	int32 GetTotalScore() const;

	void AddScore(int32 Score);
};
