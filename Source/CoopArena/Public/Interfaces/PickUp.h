// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "PickUp.generated.h"


class AActor;
class UUserWidget;


UINTERFACE(BlueprintType)
class COOPARENA_API UPickUp : public UInterface
{
	GENERATED_BODY()
	
};


class COOPARENA_API IPickUp
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintImplementableEvent, Category = PickUp)
	void OnPickUp(AActor* NewOwner);

	UFUNCTION(BlueprintImplementableEvent, Category = PickUp)
	void OnDrop();

	/**
	 * This function should display a widget (in the world) above itself with it's stats and information.
	 * @param The actor that looks at this Pickup.
	 */
	UFUNCTION(BlueprintImplementableEvent, Category = PickUp)
	UUserWidget* OnBeginTraceCastOver(APawn* TracingPawn);

	UFUNCTION(BlueprintImplementableEvent, Category = PickUp)
	void OnEndTraceCastOver(APawn* TracingPawn);
};