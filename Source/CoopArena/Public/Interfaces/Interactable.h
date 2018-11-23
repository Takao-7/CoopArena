// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Interface.h"
#include "Interactable.generated.h"


class APawn;
class UUserWidget;
class UPrimitiveComponent;


UINTERFACE(Blueprintable)
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
	 * @param HitComponent The specific component that is being interacted with.
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = Interactable)
	void OnBeginInteract(APawn* InteractingPawn, UPrimitiveComponent* HitComponent);

	/**
	 * This function is to be called when a pawn stops interacting with this actor.
	 * In case of a player, this is when the player releases the "Interact" button.
	 * 
	 * @param InteractingPawn The pawn that stopped interacting with this actor. 
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = Interactable)
	void OnEndInteract(APawn* InteractingPawn);

	/**
	 * This function is to be called, when a player looks at this actor and
	 * should enable RenderCustomDepths pass and set the CustomDepthStencil value from 252 to 255 (4 different colors).
	 * 
	 * @param Pawn The player's pawn that is looking at this actor.
	 * @param HitComponent The specific component that was hit by the line trace.
	 * @return A information widget that shows information about this actor.
	 * Can be nullptr when there is no relevant information.
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = Interactable)
	UUserWidget* OnBeginLineTraceOver(APawn* Pawn, UPrimitiveComponent* HitComponent);

	/**
	* This function is to be called, when a player stops looking at this actor and
	* should disable the RenderCustomDepths pass.
	*
	* @param Pawn The player's pawn that is stopped looking at this actor.
	*/
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = Interactable)
	void OnEndLineTraceOver(APawn* Pawn);

	/* Sets the line trace response to the interactable channel depending on the given value. */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = Interactable)
	void SetCanBeInteractedWith(bool bCanbeInteractedWith);
};


/////////////////////////////////////////////////////
				/* Interactable interface */
/////////////////////////////////////////////////////
//public:
//	virtual void OnBeginInteract_Implementation(APawn* InteractingPawn, UPrimitiveComponent* HitComponent) override;
//	virtual void OnEndInteract_Implementation(APawn* InteractingPawn) override;
//	virtual UUserWidget* OnBeginLineTraceOver_Implementation(APawn* Pawn, UPrimitiveComponent* HitComponent) override;
//	virtual void OnEndLineTraceOver_Implementation(APawn* Pawn) override;
//	virtual void SetCanBeInteractedWith_Implementation(bool bCanbeInteractedWith) override;