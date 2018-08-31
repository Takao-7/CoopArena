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
		density = 1.0f;
		volume = 1.0f;
		type = EItemType::None;
		bIsNotSplittable = true;
	}
		
	/* This item's name. Must be unique! */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FName name;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	class UTexture2D* icon;

	/* Weight, in kg, of one cubic meter (m^3) (or one piece, in case of bIsNotSplittable = true) of this item. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	float density;

	/* This item's volume, in m^3 per piece. Only used if the item is not-splittable. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (EditCondition = "bIsNotSplittable"))
	float volume;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	EItemType type;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TSubclassOf<AItemBase> itemClass;

	/**
	 * Is this item splittable? A gun is not splittable, but ore, water, etc. would be splittable.
	 * A not-splittable item is only moved in whole units.
	 */ 
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	bool bIsNotSplittable;
};