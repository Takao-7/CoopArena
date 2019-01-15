// Fill out your copyright notice in the Description page of Project Settings.

#pragma once
#include "CoreMinimal.h"
#include "ItemStructs.generated.h"


class AMagazine;


USTRUCT(BlueprintType)
struct FMagazineStack
{
	GENERATED_BODY()

public:
	FMagazineStack() {};
	FMagazineStack(TSubclassOf<AMagazine> MagClass, int32 StackSize)
	{
		this->magClass = MagClass;
		this->stackSize = StackSize;
	};

	FORCEINLINE bool operator==(const TSubclassOf<AMagazine>& OtherMagClass) const
	{
		return this->magClass == OtherMagClass;
	};

	FORCEINLINE bool operator==(const FMagazineStack& OtherStack) const
	{
		const bool bSameClass = this->magClass == OtherStack.magClass;
		const bool bSameStackSize = this->stackSize == OtherStack.stackSize;
		return bSameClass && bSameStackSize;
	};

	UPROPERTY()
	TSubclassOf<AMagazine> magClass;

	UPROPERTY()
	int32 stackSize;
};