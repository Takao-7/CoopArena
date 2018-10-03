// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "RespawnComponent.generated.h"


class UHealthComponent;


DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FRespawn_Signature, AActor*, NewSpawnedActor, AController*, Controller);


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class COOPARENA_API URespawnComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	URespawnComponent();

	/** 
	 * Re-spawn our owner. Kills (@see UHealthComponent & @see m_bRespawnOnDeathAndDestroy) or destroys (@see m_bRespawnOnDeathAndDestroy) our owner before.
	 * When m_RespawnDelay is set, the delay will be taken into account.
	 */
	UFUNCTION(BlueprintCallable, Category = "Respawn")
	void Respawn();

	UFUNCTION(BlueprintCallable, Category = "Respawn")
	void EnableRespawn(bool bShouldRespawn) { m_bEnableRespawn = bShouldRespawn; };

	/* Event will be called when we re-spawned. Controller can be null, if our owner isn't a pawn. */
	FRespawn_Signature OnRespawn_Event;

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
	UPROPERTY(EditDefaultsOnly, Category = "Respawn", meta = (DisplayName = "Respawn on death and OnDestroy()?"))
	bool m_bRespawnOnDeathAndDestroy;

	/* If true, the old actor will be destroyed when we re-spawn. */
	UPROPERTY(EditDefaultsOnly, Category = "Respawn", meta = (DisplayName = "Destroy old actor on respawn"))
	bool m_bDestroyOldActorOnRespawn;

	/* The time, in seconds, after death / destroy it takes for us to re-spawn. */
	UPROPERTY(EditDefaultsOnly, Category = "Respawn", meta = (DisplayName = "Respawn delay"))
	float m_RespawnDelay;

	virtual void BeginPlay() override;

private:
	UHealthComponent* m_HealthComp;
	bool m_OwnerAlreadyDestroyed;

	AActor* FindRespawnPoint();
	AActor* SpawnNewActor();
	void HandleRespawn();

	UFUNCTION()
	void HandleOnDestroy(AActor* DestroyedActor);

	UFUNCTION()
	void HandleOnDeath();
};