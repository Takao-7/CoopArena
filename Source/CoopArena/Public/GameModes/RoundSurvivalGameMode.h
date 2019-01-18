// Fill out your copyright notice in the Description page of Project Settings.

#pragma once
#include "CoreMinimal.h"
#include "GameModes/CoopArenaGameMode.h"
#include "RoundSurvivalGameMode.generated.h"


class AHumanoid;
class APlayerCharacter;
class AMyPlayerController;
class ABot;
class UCoopArenaGameInstance;


DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnWaveStart_Event, int32, WaveNumber);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnWaveEnd_Event, int32, WaveNumber);

USTRUCT(BlueprintType)
struct FBotSpawn
{
	GENERATED_BODY()

public:
	FBotSpawn() {};
	FBotSpawn(TSubclassOf<ABot> Botclass, float SpawnChance, float SpawnChanceIncreasePerWave)
	{
		this->Botclass = Botclass;
		this->SpawnChance = FMath::Clamp(SpawnChance, 0.0f, 1.0f);
		this->SpawnChanceIncreasePerWave = FMath::Clamp(SpawnChanceIncreasePerWave, 0.0f, 1.0f);
	};

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<ABot> Botclass;

	/* The chance that this bot will be spawned. 0 = 0%. 1 = 100%. */
	UPROPERTY(EditDefaultsOnly, meta = (ClampMin = 0.0f, ClampMax = 1.0f))
	float SpawnChance;

	/* After each the SpawnChance will be increased by this value, up to a maximum of 1 */
	UPROPERTY(EditDefaultsOnly, meta = (ClampMin = 0.0f, ClampMax = 1.0f))
	float SpawnChanceIncreasePerWave;
};


UCLASS()
class COOPARENA_API ARoundSurvivalGameMode : public ACoopArenaGameMode
{
	GENERATED_BODY()

protected:
	/* How many bots should be spawned per player in the first wave? */
	UPROPERTY(EditDefaultsOnly, Category = "Round survival game mode", meta = (DisplayName = "Bots to spawn per player", ClampMin = 1))
	int32 _BotsToSpawnPerPlayer;

	/**
	 * After each round, the number of bots that are spawned (@see _BotsToSpawnPerPlayer)
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

	UPROPERTY(EditDefaultsOnly, Category = "Round survival game mode", meta = (DisplayName = "Bot despawn time", ClampMin = 0.0f))
	float _BotDespawnTime;

	/**
	 * All spawn locations on the map. These are the names of the sub-levels where the spawn points are on. 
	 * Each sub-level contains a spawn point for the players and several spawn points for the bots.
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Round survival game mode")
	TArray<FName> _SpawnLocations;

	/* The chosen spawn location from _SpawnLocations. This sub-level will be loaded and used. */
	UPROPERTY()
	FName _ChoosenSpawnLocation;

	UFUNCTION()
	void OnSpawnLocationLoaded();

private:
	bool _bSpawnLocationLoaded;
	int32 _NumBotsToSpawn;
	

	/////////////////////////////////////////////////////
					/* Match flow */
	/////////////////////////////////////////////////////
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
protected:
	/** @return The CoopArena game instance. Will crash if none exists. */
	UCoopArenaGameInstance* GetCoopArenaGameInstance();

public:
	ARoundSurvivalGameMode();

	UFUNCTION(BlueprintCallable, Category = "Round survival game mode", Exec)
	void SpawnBots();

	bool CanSpawnBots();

	/* Returns the number of bots that are currently alive */
	UFUNCTION(BlueprintPure, Category = "Round survival game mode")
	FORCEINLINE int32 GetNumberOfAliveBots() const { return _BotsAlive.Num(); }

private:
	UFUNCTION()
	void HandleOnDestroyed(AActor* DestroyedActor);

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


	/////////////////////////////////////////////////////
				/* Overridden functions */
	/////////////////////////////////////////////////////
protected:
	/** @return True if ready to Start Match. Games should override this */
	bool ReadyToStartMatch_Implementation() override;

	/** @return true if ready to End Match. Games should override this */
	bool ReadyToEndMatch_Implementation() override;

public:
	virtual void InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage) override;

	virtual void Logout(AController* Exiting) override;
};