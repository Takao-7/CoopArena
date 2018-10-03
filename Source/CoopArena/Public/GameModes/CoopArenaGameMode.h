// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "CoopArenaGameMode.generated.h"


class ASpawnPoint;
class APlayerCharacter;
class AHumanoid;


UCLASS()
class COOPARENA_API ACoopArenaGameMode : public AGameModeBase
{
	GENERATED_BODY()
	
	
protected:
	UPROPERTY(BlueprintReadOnly, Category = "CoopArena game mode")
	TArray<ASpawnPoint*> SpawnPoints;

	UPROPERTY(BlueprintReadOnly, Category = "CoopArena game mode")
	TArray<APlayerCharacter*> PlayerCharacters;

	UPROPERTY(BlueprintReadOnly, Category = "CoopArena game mode")
	TArray<AHumanoid*> Bots;

	UFUNCTION(BlueprintCallable, Category = "CoopArena game mode")
	void FindSpawnPoints();

public:
	/**
	 * Registers the given spawn point to the game mode.
	 * @param SpawnPoint The spawn point to register. Must not be null.
	 */
	UFUNCTION(BlueprintCallable, Category = "CoopArena game mode")
	void RegisterSpawnPoint(ASpawnPoint* SpawnPoint);

	UFUNCTION(BlueprintCallable, Category = "CoopArena game mode")
	AActor* GetPlayerStart(AController* Player, const FString& IncomingName = TEXT(""));
};