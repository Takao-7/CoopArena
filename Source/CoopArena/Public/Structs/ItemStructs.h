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


//USTRUCT(BlueprintType)
//struct FItemStats
//{
//	GENERATED_BODY()
//
//	FItemStats()
//	{
//		name = "Nobody";
//		weight = 1.0f;
//		volume = 1.0f;
//		type = EItemType::None;
//	};
//		
//	/* This item's name. Must be unique! */
//	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
//	FName name;
//
//	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
//	class UTexture2D* icon;
//
//	/* This item's weight, in kg. */
//	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
//	float weight;
//
//	/* This item's volume, in cm^3. */
//	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
//	float volume;
//
//	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
//	EItemType type;
//
//	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
//	TSubclassOf<AItemBase> itemClass;
//};


