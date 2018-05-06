// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Humanoid.h"
#include "PlayerCharacter.generated.h"


class UCameraComponent;


UCLASS()
class COOPARENA_API APlayerCharacter : public AHumanoid
{
	GENERATED_BODY()
	
public:
	APlayerCharacter();

	/* Returns the characters camera location in world space. If there is no camera, then 
	 * this function returns a ZeroVector */
	UFUNCTION(BlueprintPure, Category = PlayerCharacter)
	FVector GetCameraLocation() const;
};
