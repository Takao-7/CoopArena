// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerStart.h"
#include "SpawnPoint.generated.h"


class UBoxComponent;


UCLASS()
class COOPARENA_API ASpawnPoint : public APlayerStart
{
	GENERATED_BODY()
	
	
public:
	ASpawnPoint(const FObjectInitializer& ObjectInitializer);

	/**
	 * Checks if there are any pawns in the safe zone that do not have the given tag.
	 * @return True if there are no pawn without the given tag in the safe zone.	 
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Spawn point")
	bool IsSafeToSpawn(const FString& TagToCompare) const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Spawn point")
	UBoxComponent* GetSafeZone() const;

protected:
	/* Area around the spawn point that needs to be enemy-free in order to spawn a player. */
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Spawn point")
	UBoxComponent* SafeZone;
};