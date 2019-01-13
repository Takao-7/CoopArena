// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "HUDInterface.generated.h"


UINTERFACE(MinimalAPI)
class UHUDInterface : public UInterface
{
	GENERATED_BODY()
};

class COOPARENA_API IHUDInterface
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
	/**
	 * @param bStartWave True if the message is for a starting wave, false if for a ending wave.
	 * @param WaveNumber The wave that ended/started.
	 */
	UFUNCTION(BlueprintImplementableEvent, Category = "HUD")
	void SetWaveNotification(bool bStartWave, int32 WaveNumber);

	/* Sets the number of magazines that the character has in his inventory for the weapon that he has currently equipped. */
	UFUNCTION(BlueprintImplementableEvent, Category = "HUD")
	void SetNumberOfMagazines(int32 NumMagazines);

	/* Sets the number of rounds left in the magazines. */
	UFUNCTION(BlueprintImplementableEvent, Category = "HUD")
	void SetNumberOfRounds(int32 NumRounds);	
};
