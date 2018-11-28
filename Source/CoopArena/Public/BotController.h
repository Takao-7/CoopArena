// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "BotController.generated.h"


class AActor;


/**
 * 
 */
UCLASS()
class COOPARENA_API ABotController : public AAIController
{
	GENERATED_BODY()
	
public:
	UFUNCTION(BlueprintImplementableEvent)
	void SetAttackTarget(FVector Location, AActor* Actor = nullptr);
};
