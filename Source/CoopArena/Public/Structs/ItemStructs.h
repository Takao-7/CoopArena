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

	/**
	 * This value is only used, when this item is not-splittable (bIsNotSplittable = true).
	 * This item's volume, in m^3 per piece.
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	float volume;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	EItemType type;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TSubclassOf<AItemBase> itemClass;

	/**
	 * Is this item splittable or not? A gun is not splittable, but ore, water, etc. would be splittable.
	 * A not-splittable item is only moved in whole units.
	 */ 
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	bool bIsNotSplittable;

//private:
//	/* How much or many, depending on the measure unit, are currently stored in this inventory. */
//	UPROPERTY()
//	float stackSize;
//
//public:
//	/* 
//	 * Changes the amount of this item.
//	 * Will through an error, when the measureUnit is "None" and a not int-like value is given.
//	 * @param AmountToChange The amount the stack size should be changed by. Positive for increasing, negative for decreasing the stack.
//	 * @return How much was changed.
//	 */
//	float ChangeAmount(float AmountToChange)
//	{
//		if (measureUnit == EMeasureUnit::None)
//		{
//			float intPart;
//			float fractPart = FMath::Modf(AmountToChange, &intPart);
//			if (fractPart != 0.0f)
//			{
//				UE_LOG(LogTemp, Fatal, TEXT("Try to reduce a not-whole amount from an item with a none measure unit. AmountToChange: %f. FractPart: %f."), AmountToChange, fractPart);
//			}
//		}
//
//		float rest = stackSize - AmountToChange;
//		float actualAmountRemoved = AmountToChange;
//
//		if (rest < 0.0f)
//		{
//			UE_LOG(LogTemp, Error, TEXT("It was tried to remove more from a stack then there was. Stacksize: %f. AmountToChange: %f."), stackSize, AmountToChange);
//			actualAmountRemoved = stackSize;
//		}
//
//		stackSize += actualAmountRemoved;
//		return actualAmountRemoved;
//	}
//
//	float GetStackSize() const { return stackSize; }
};