// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "RespawnComponent.generated.h"


class UHealthComponent;


DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FRespawn_Signature, AActor*, NewSpawnedActor, AController*, Controller);


/**
 * Component for re-spawn logic.
 * Can be used for both, pawns and pickups, etc. to let them re-spawn after death (if they have a HealthComponent) or after they get destroyed.
 */
UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class COOPARENA_API URespawnComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	URespawnComponent();

	/**
	 * Re-spawn our owner. Kills (@see UHealthComponent & @see m_bRespawnOnDeathAndDestroy) or destroys (@see m_bRespawnOnDeathAndDestroy) our owner before re-spawning.
	 * The re-spawn delay (@see m_RespawnDelay) will be taken into account if the our owner is not being destroyed.
	 */
	UFUNCTION(BlueprintCallable, Category = "Respawn")
	void Respawn();

	UFUNCTION(BlueprintCallable, Category = "Respawn")
	void SetEnableRespawn(bool bShouldRespawn) { m_bEnableRespawn = bShouldRespawn; };

	/* Event will be called when we re-spawned. Controller can be null, if our owner isn't a pawn. */
	FRespawn_Signature OnRespawn_Event;

	UFUNCTION(BlueprintPure, Category = "Respawn")
	float GetRespawnDelay() const { return m_RespawnDelay; };

protected:
	/* Should we re-spawn at all? This can be enabled during gameplay and only has to be true when our owner dies or gets destroyed. */
	UPROPERTY(EditDefaultsOnly, Category = "Respawn", meta = (DisplayName = "Enable respawn"))
	bool m_bEnableRespawn;

	/* Should we re-spawn at the location where we died? */
	UPROPERTY(EditDefaultsOnly, Category = "Respawn", meta = (DisplayName = "Respawn at death location?"))
	bool m_bRespawnAtDeathLocation;

	/**
	* If true, we will re-spawn when our owner dies (has a HealthComponent, when the event OnDeath is fired) or when he gets destroyed (OnDestroy() is called on him).
	* We will not re-spawn two times.
	* If false, we will only re-spawn if we die. When there is no HealthComponent, then only when our owner gets destroyed.
	*/
	UPROPERTY(EditDefaultsOnly, Category = "Respawn", meta = (DisplayName = "Respawn on OnDestroy()?"))
	bool m_bRespawnOnDestroy;

	/* If true, the old actor will be destroyed when we re-spawn. */
	UPROPERTY(EditDefaultsOnly, Category = "Respawn", meta = (DisplayName = "Destroy old actor on respawn"))
	bool m_bDestroyOldActorOnRespawn;

	/* The delay, in seconds, after death / destroy to re-spawn. */
	UPROPERTY(EditDefaultsOnly, Category = "Respawn", meta = (DisplayName = "Respawn delay", ClampMin = 0.0f))
	float m_RespawnDelay;

	virtual void BeginPlay() override;

private:
	UHealthComponent* m_HealthComp;

	AActor* FindRespawnPoint();
	AActor* SpawnNewActor();

	/* Handles the actual re-spawning. Shouldn't be called directly. Call Respawn() instead. */
	void HandleRespawn();

	UFUNCTION()
	void HandleOnDestroy(AActor* DestroyedActor);

	UFUNCTION()
	void HandleOnDeath();
};