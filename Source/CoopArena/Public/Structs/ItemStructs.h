// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Enums/ItemEnums.h"
#include "ItemStructs.generated.h"


class AItemBase;


USTRUCT(BlueprintType)
struct FItemStats
{
	GENERATED_BODY()

	FItemStats()
	{
		name = "Nobody";
		weight = 1.0f;
		volume = 1.0f;
		type = EItemType::None;
	}
		
	/* This item's name. Must be unique! */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FName name;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	class UTexture2D* icon;

	/* This item's weight, in kg. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	float weight;

	/* This item's volume, in cm^3. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	float volume;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	EItemType type;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TSubclassOf<AItemBase> itemClass;
};