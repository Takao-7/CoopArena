// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameModes/CoopArenaGameMode.h"
#include "RoundSurvivalGameMode.generated.h"


class AHumanoid;
class APlayerCharacter;
class AMyPlayerController;
class ABot;


DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnWaveStart_Event, int32, WaveNumber);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnWaveEnd_Event, int32, WaveNumber);


UCLASS()
class COOPARENA_API ARoundSurvivalGameMode : public ACoopArenaGameMode
{
	GENERATED_BODY()

protected:
	/* How many bots should be spawned per player in the first wave? */
	UPROPERTY(EditDefaultsOnly, Category = "Round survival game mode", meta = (DisplayName = "Bots to spawn per player", ClampMin = 1))
	int32 _BotsToSpawnPerPlayer;

	/**
	 * After each round, the number of bots that are spawned (@see m_BotsToSpawnPerPlayer)
	 * are multiplied by this value.
	 */ 
	UPROPERTY(EditDefaultsOnly, Category = "Round survival game mode", meta = (DisplayName = "Wave strength increase", ClampMin = 1.0f))
	float _WaveStrengthIncreaseFactor;
	
	/* Bot classes to spawn. Will pick a random class each time a bot is spawned */
	UPROPERTY(EditDefaultsOnly, Category = "Round survival game mode", meta = (DisplayName = "Bots to spawn"))
	TArray<TSubclassOf<ABot>> _BotsToSpawn;

	UPROPERTY(EditDefaultsOnly, Category = "Round survival game mode", meta = (DisplayName = "Points per bot kill", ClampMin = 0))
	int32 _PointsPerBotKill;

	/* How many points will be subtracted from the player score for each team kill */
	UPROPERTY(EditDefaultsOnly, Category = "Round survival game mode", meta = (DisplayName = "Point penalty for team kill", ClampMin = 0))
	int32 _PointPenaltyForTeamKill;

	/* The time, in seconds, after which a new wave starts. */
	UPROPERTY(EditDefaultsOnly, Category = "Round survival game mode", meta = (DisplayName = "Wave length", ClampMin = 1.0f))
	float _WaveLength;

	/* The delay, in seconds, between the end of a wave and the start of the next. */
	UPROPERTY(EditDefaultsOnly, Category = "Round survival game mode", meta = (DisplayName = "Delay between waves", ClampMin = 0.0f))
	float _DelayBetweenWaves;
	
	UPROPERTY(BlueprintReadOnly, Category = "Round survival game mode", meta = (DisplayName = "Current wave"))
	int32 _CurrentWaveNumber;


	/////////////////////////////////////////////////////
					/* Match flow */
	/////////////////////////////////////////////////////
protected:
	/** @return True if ready to Start Match. Games should override this */
	bool ReadyToStartMatch_Implementation() override;

	/** @return true if ready to End Match. Games should override this */
	bool ReadyToEndMatch_Implementation() override;

public:
	virtual void StartMatch() override;

	UFUNCTION(BlueprintCallable, Category = "Round survival game mode", Exec)
	void StartWave();

	UFUNCTION(BlueprintCallable, Category = "Round survival game mode", Exec)
	void EndWave();

	void SetPlayersHealthToMax();

	UFUNCTION(BlueprintPure, Category = "Round survival game mode")
	FORCEINLINE int32 GetCurrentWaveNumber() const { return _CurrentWaveNumber; };
	
	UPROPERTY(BlueprintAssignable, Category = "Round survival game mode")
	FOnWaveStart_Event OnWaveStart;

	UPROPERTY(BlueprintAssignable, Category = "Round survival game mode")
	FOnWaveEnd_Event OnWaveEnd;


	/////////////////////////////////////////////////////
public:
	ARoundSurvivalGameMode();

	virtual void InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage) override;

	UFUNCTION(BlueprintCallable, Category = "Round survival game mode", Exec)
	void SpawnBots();

	bool CanSpawnBots();

	/* Returns the number of bots that are currently alive */
	UFUNCTION(BlueprintPure, Category = "Round survival game mode")
	FORCEINLINE int32 GetNumberOfAliveBots() const { return _BotsAlive.Num(); }

private:
	/* Set the target for all spawned bots. */
	UFUNCTION()
	void SetAttackTarget();

	void DestroyDeadBotBodies();

	void ReviveDeadPlayers();

	int32 GetNumberOfConnectedPlayers();

	/* Is called when the OnBotDeath_Event is fired */
	UFUNCTION()
	void HandleBotDeath(AActor* DeadBot, AController* Killer);

	/* Is called when the OnPlayerDeath_Event is fired */
	UFUNCTION()
	void HandlePlayerDeath(APlayerCharacter* DeadPlayer, AController* Killer);

	void StartSpectating(AMyPlayerController* PlayerController);

	TArray<ASpawnPoint*> _BotSpawnPoints;

	/* Bots that are currently alive */
	TArray<ABot*> _BotsAlive;

	/* Bots that are dead. They will be deleted at the start of the next wave and then cleared from this array. */
	TArray<ABot*> _BotsDead;

	FTimerHandle _RoundTimerHandle;
};