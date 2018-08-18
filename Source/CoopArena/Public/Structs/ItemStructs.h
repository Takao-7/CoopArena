// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Enums/ItemEnums.h"
#include "ItemStructs.generated.h"


USTRUCT(BlueprintType)
struct FItemStats
{
	GENERATED_BODY()

	FItemStats()
	{
		Name = "Nobody";
		Weight = 0.0f;
		Type = EItemType::None;
		ChargeLeft = -1.0f;
	}

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FName Name;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	class UTexture2D* Icon;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	float Weight;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	EItemType Type;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TSubclassOf<class AItemBase> Class;

	/* How much charge, shots, etc. the item has left. -1 = this value is not important for this item. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	float ChargeLeft;
};