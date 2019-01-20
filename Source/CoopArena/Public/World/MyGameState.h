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

protected:
	UPROPERTY(EditDefaultsOnly, Category = "Game state", meta = (DisplayName = "Loading screen class"))
	TSubclassOf<UUserWidget> _LoadingScreenClass;
	
private:
	/* Total accumulated score from all players  */
	UPROPERTY(Transient, Replicated)
	int32 _TeamScore;

	UPROPERTY(Transient, Replicated)
	int32 _WaveNumber;

	UFUNCTION()
	void HandleOnPostLogin(AGameModeBase* GameMode, APlayerController* NewPlayer);

	UFUNCTION(NetMulticast, Reliable)
	void HandleOnLogout_Multicast(AMyPlayerState* PlayerState);

	UFUNCTION()
	void HandleOnLogout(AGameModeBase* GameMode, AController* Exiting);

public:
	/**
	 * This function will be called after a player has logged in and has send his username to the server.
	 */
	UFUNCTION(NetMulticast, Reliable, Category = "Game state")
	void OnPostLogin_Multicast(AMyPlayerState* NewPlayerState, const FString& NewPlayerName);

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

	/* This function will be called by the game mode, when all players are dead. */
	UFUNCTION(NetMulticast, Reliable, Category = "Game state")
	void OnGameOver();

	UFUNCTION(NetMulticast, Reliable, Category = "Game state")
	void ShowLoadingScreen_Multicast();

	/* This function is called when the host leaves the match. */
	UFUNCTION(NetMulticast, Reliable)
	void EndMatch();
};
