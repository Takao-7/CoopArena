// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Interface.h"
#include "Interactable.generated.h"


class APawn;
class UWidget;


UINTERFACE(BlueprintType)
class COOPARENA_API UInteractable : public UInterface
{
	GENERATED_BODY()
};


class COOPARENA_API IInteractable
{
	GENERATED_BODY()

public:
	/**
	 * This function is to be called when a pawn starts interacting with this actor.
	 * In case of a player, this is when the player presses the "Interact" button DOWN.
	 * 
	 * @param InteractingPawn The pawn that is interacting with this actor. 
	 */
	UFUNCTION(BlueprintImplementableEvent, Category = Interactible)
	void OnBeginInteract(APawn* InteractingPawn);

	/**
	 * This function is to be called when a pawn stops interacting with this actor.
	 * In case of a player, this is when the player releases the "Interact" button.
	 * 
	 * @param InteractingPawn The pawn that stopped interacting with this actor. 
	 */
	UFUNCTION(BlueprintImplementableEvent, Category = Interactible)
	void OnEndInteract(APawn* InteractingPawn);

	/**
	 * This function is to be called, when a player looks at this actor.
	 * 
	 * @param Pawn The player's pawn that is looking at this actor.
	 * @return A information widget that shows information about this actor.
	 * Can be nullptr when there is no relevant information.
	 */
	UFUNCTION(BlueprintImplementableEvent, Category = Interactible)
	UWidget* OnBeginRayTraceOver(APawn* Pawn);

	/**
	* This function is to be called, when a player stops looking at this actor.
	*
	* @param Pawn The player's pawn that is stopped looking at this actor.
	*/
	UFUNCTION(BlueprintImplementableEvent, Category = Interactible)
	void OnEndRayTraceOver(APawn* Pawn);
};
