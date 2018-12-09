// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerStart.h"
#include "SpawnPoint.generated.h"


class USphereComponent;


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
	USphereComponent* GetSafeZone() const { return SafeZone; };

	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;

	/**
	 * Called when an instance of this class is placed (in editor) or spawned.
	 * @param Transform The transform the actor was constructed at.
	 */
	virtual void OnConstruction(const FTransform& Transform) override;

	/* Is the given controller allowed to spawn here? */
	bool IsAllowedToSpawn(AController* Controller);

protected:
	/* Area around the spawn point that needs to be enemy-free in order to spawn a player. */
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadWrite, Category = "Spawn point")
	USphereComponent* SafeZone;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn point", meta = (DisplayName = "Safe zone radius"))
	float m_SafeZoneRadius;

	/* Are players allowed to spawn here? */
	UPROPERTY(EditDefaultsOnly, Category = "Spawn point", meta = (DisplayName = "Allow player spawn"))
	bool m_bAllowPlayerSpawn;

	/* Are bots allowed to spawn here? */
	UPROPERTY(EditDefaultsOnly, Category = "Spawn point", meta = (DisplayName = "Allow bot spawn"))
	bool m_bAllowBotSpawn;
};