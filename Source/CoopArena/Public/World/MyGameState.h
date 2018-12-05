// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameState.h"
#include "MyGameState.generated.h"


class AGameModeBase;
class APlayerController;
class AMyPlayerState;


DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPlayerLogIn_Signature, AMyPlayerState*, PlayerState);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPlayerLogout_Signature, AMyPlayerState*, PlayerState);


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

	UFUNCTION(NetMulticast, Reliable, Category = "Game state")
	void HandleOnPostLogin(AGameModeBase* GameMode, APlayerController* NewPlayer);

	UFUNCTION(NetMulticast, Reliable, Category = "Game state")
	void HandleOnLogout(AGameModeBase* GameMode, AController* Exiting);
	
public:
	UPROPERTY(BlueprintAssignable, Category = "Game state")
	FOnPlayerLogIn_Signature OnPlayerLogin;

	UPROPERTY(BlueprintAssignable, Category = "Game state")
	FOnPlayerLogout_Signature OnPlayerLogout;

	AMyGameState(const class FObjectInitializer& ObjectInitializer);

	UFUNCTION(BlueprintPure, Category = "Game state")
	int32 GetTotalScore() const;

	UFUNCTION(BlueprintPure, Category = "Game state")
	int32 GetWaveNumber() const;

	void SetWaveNumber(int32 WaveNumber);

	/* [Server] Add score to the team score. */
	void AddScore(int32 Score);
};
