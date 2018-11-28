// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "MyPlayerState.generated.h"

/**
 * 
 */
UCLASS()
class COOPARENA_API AMyPlayerState : public APlayerState
{
	GENERATED_BODY()

public:
	AMyPlayerState(const FObjectInitializer& ObjectInitializer);

	virtual void Reset() override;

	void SetTeamNumber(int32 NewTeamNumber);

	UFUNCTION(BlueprintPure, Category = "Player state")
	int32 GetTeamNumber() const;

	void AddKill();

	UFUNCTION(BlueprintPure, Category = "Player state")
	int32 GetKills() const;

	void AddDeath();

	UFUNCTION(BlueprintPure, Category = "Player state")
	int32 GetDeaths() const;

	void ScorePoints(int32 Points, bool bAddKill = true);

	UFUNCTION(BlueprintPure, Category = "Player state")
	float GetScore() const;

private:
	UPROPERTY(Transient, Replicated)
	int32 _NumKills;

	UPROPERTY(Transient, Replicated)
	int32 _NumDeaths;

	/* Team number assigned to player */
	UPROPERTY(Transient, Replicated)
	int32 _TeamNumber;
};
