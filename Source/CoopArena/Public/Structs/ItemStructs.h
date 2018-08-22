// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Enums/ItemEnums.h"
#include "ItemStructs.generated.h"


/*  */
UENUM(BlueprintType)
enum class  EMeasureUnit : uint8
{
	/* This item is not splittable. E.g. for weapons, etc. */
	None,

	/* In m^3 */
	Volume,

	/* In kg */
	Mass
};


USTRUCT(BlueprintType)
struct FItemStats
{
	GENERATED_BODY()

	FItemStats()
	{
		name = "Nobody";
		weight = 0.0f;
		type = EItemType::None;
		measureUnit = EMeasureUnit::None;
		stackSize = 1.0f;
	}

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FName name;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	class UTexture2D* icon;

	/* How much does one "piece" of this item weights. In kg. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	float weight;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	EItemType type;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TSubclassOf<class AItemBase> itemClass;

	/* 
	 * The measure unit that this item will be counted in.
	 * "None" means, that this item can't be split and therefore can only exist in whole pieces (e.g. a you can't have 0.5 guns).
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	EMeasureUnit measureUnit;

private:
	/* How much or many, depending on the measure unit, are currently stored in this inventory. */
	UPROPERTY()
	float stackSize;

public:
	/* 
	 * Changes the amount of this item.
	 * Will through an error, when the measureUnit is "None" and a not int-like value is given.
	 * @param AmountToChange The amount the stack size should be changed by. Positive for increasing, negative for decreasing the stack.
	 * @return How much was changed.
	 */
	float ChangeAmount(float AmountToChange)
	{
		if (measureUnit == EMeasureUnit::None)
		{
			float intPart;
			float fractPart = FMath::Modf(AmountToChange, &intPart);
			if (fractPart != 0.0f)
			{
				UE_LOG(LogTemp, Fatal, TEXT("Try to reduce a not-whole amount from an item with a none measure unit. AmountToChange: %f. FractPart: %f."), AmountToChange, fractPart);
			}
		}

		float rest = stackSize - AmountToChange;
		float actualAmountRemoved = AmountToChange;

		if (rest < 0.0f)
		{
			UE_LOG(LogTemp, Error, TEXT("It was tried to remove more from a stack then there was. Stacksize: %f. AmountToChange: %f."), stackSize, AmountToChange);
			actualAmountRemoved = stackSize;
		}

		stackSize += actualAmountRemoved;
		return actualAmountRemoved;
	}

	float GetStackSize() const { return stackSize; }
};