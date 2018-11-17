// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameModes/CoopArenaGameMode.h"
#include "RoundSurvivalGameMode.generated.h"


class AHumanoid;
class APlayerCharacter;
class AMyPlayerController;


DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnWaveStart_Event);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnWaveEnd_Event);


UCLASS()
class COOPARENA_API ARoundSurvivalGameMode : public ACoopArenaGameMode
{
	GENERATED_BODY()

protected:
	/* How many bots should be spawned per player in the first wave? */
	UPROPERTY(EditDefaultsOnly, Category = "Round survival game mode", meta = (DisplayName = "Bots to spawn per player", ClampMin = 1))
	int32 m_BotsToSpawnPerPlayer;

	/**
	 * After each round, the number of bots that are spawned (@see m_BotsToSpawnPerPlayer)
	 * are multiplied by this value.
	 */ 
	UPROPERTY(EditDefaultsOnly, Category = "Round survival game mode", meta = (DisplayName = "Wave strength increase", ClampMin = 1.0f))
	float m_WaveStrengthIncrease;
	
	/* Bot classes to spawn. Will pick a random class each time a bot is spawned */
	UPROPERTY(EditDefaultsOnly, Category = "Round survival game mode", meta = (DisplayName = "Bots to spawn"))
	TArray<TSubclassOf<AHumanoid>> m_BotsToSpawn;

	UPROPERTY(EditDefaultsOnly, Category = "Round survival game mode", meta = (DisplayName = "Points per bot kill", ClampMin = 0))
	int32 m_PointsPerBotKill;

	/* How many points will be subtracted from the player score for each team kill */
	UPROPERTY(EditDefaultsOnly, Category = "Round survival game mode", meta = (DisplayName = "Points per bot kill", ClampMin = 0))
	int32 m_PointPenaltyForTeamKill;

	/* The time, in seconds, after which a new wave starts. */
	UPROPERTY(EditDefaultsOnly, Category = "Round survival game mode", meta = (DisplayName = "Wave length", ClampMin = 1.0f))
	float m_WaveLength;
	
	UPROPERTY(BlueprintReadOnly, Category = "Round survival game mode", meta = (DisplayName = "Current wave"))
	int32 m_CurrentWave;

	/////////////////////////////////////////////////////
					/* Match flow */
	/////////////////////////////////////////////////////
protected:
	/** @return True if ready to Start Match. Games should override this */
	bool ReadyToStartMatch_Implementation() override;

	/** @return true if ready to End Match. Games should override this */
	bool ReadyToEndMatch_Implementation() override;

public:
	UFUNCTION(BlueprintCallable, Category = "Round survival game mode", Exec)
	void StartWave();

	UFUNCTION(BlueprintCallable, Category = "Round survival game mode", Exec)
	void EndWave();

	UFUNCTION(BlueprintPure, Category = "Round survival game mode")
	FORCEINLINE int32 GetCurrentWaveNumber() const { return m_CurrentWave; };
	
	UPROPERTY(BlueprintAssignable, Category = "Round survival game mode")
	FOnWaveStart_Event OnWaveStart_Event;

	UPROPERTY(BlueprintAssignable, Category = "Round survival game mode")
	FOnWaveEnd_Event OnWaveEnd_Event;

public:
	ARoundSurvivalGameMode();

	virtual void InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage) override;

	UFUNCTION(BlueprintCallable, Category = "Round survival game mode", Exec)
	void SpawnBots();

	/* Returns the number of bots that are currently alive */
	UFUNCTION(BlueprintPure, Category = "Round survival game mode")
	FORCEINLINE int32 GetNumberOfAliveBots() const { return m_BotsAlive.Num(); }

	/**
	 * Return the specific player start actor that should be used for the next spawn
	 * This will either use a previously saved startactor, or calls ChoosePlayerStart
	 *
	 * @param Player The AController for whom we are choosing a Player Start
	 * @param IncomingName Specifies the tag of a Player Start to use
	 * @returns Actor chosen as player start (usually a PlayerStart)
	 */
	virtual AActor* FindPlayerStart_Implementation(AController* Player, const FString& IncomingName = TEXT("")) override;

	/**
	 * Return the 'best' player start for this player to spawn from
	 * Default implementation looks for a random unoccupied spot
	 *
	 * @param Player is the controller for whom we are choosing a playerstart
	 * @returns AActor chosen as player start (usually a PlayerStart)
	 */
	virtual AActor* ChoosePlayerStart_Implementation(AController* Player) override;

private:
	void DestroyDeadBotBodies();

	void ReviveDeadPlayers();

	/* Is called when the OnBotDeath_Event is fired */
	UFUNCTION()
	void HandleBotDeath(AActor* DeadBot, AController* Killer);

	/* Is called when the OnPlayerDeath_Event is fired */
	UFUNCTION()
	void HandlePlayerDeath(APlayerCharacter* DeadPlayer, AController* Killer);

	void StartSpectating(AMyPlayerController* PlayerController);

	TArray<ASpawnPoint*> m_BotSpawnPoints;

	/** Bots that are currently alive */
	TArray<AHumanoid*> m_BotsAlive;

	/* Bots that are dead. They will be deleted at the end of the wave and then cleared from this array. */
	TArray<AHumanoid*> m_BotsDead;

	FTimerHandle m_RoundTimerHandle;
};