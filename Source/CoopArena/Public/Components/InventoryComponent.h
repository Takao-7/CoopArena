// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Components/StorageComponent.h"
#include "Structs/ItemStructs.h"
#include "InventoryComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class COOPARENA_API UInventoryComponent : public UStorageComponent
{
	GENERATED_BODY()

	
protected:
	// Called when the game starts
	virtual void BeginPlay() override;
};