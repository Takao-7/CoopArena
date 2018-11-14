// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Magazine.h"
#include "Components/ActorComponent.h"
#include "SimpleInventory.generated.h"


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

	UPROPERTY()
	TSubclassOf<AMagazine> magClass;

	UPROPERTY()
	int32 stackSize;
};


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent), Blueprintable )
class COOPARENA_API USimpleInventory : public UActorComponent
{
	GENERATED_BODY()

	/////////////////////////////////////////////////////
				/* Parameters & variables */
	/////////////////////////////////////////////////////
protected:
	/* The maximum number of magazines, for each magazine type, that this inventory can hold */
	UPROPERTY(EditDefaultsOnly, Category = "Inventory", meta = (DisplayName = "Maximum number of magazines"))
	TMap<TSubclassOf<AMagazine>, int32> m_MaxNumberOfMagazines;

	/**
	* The default maximum number of magazines for each magazine type.
	* Will only be used if a specific magazines type is not set in @see m_MaxNumberOfMagaziens.
	*/
	UPROPERTY(EditDefaultsOnly, Category = "Inventory", meta = (DisplayName = "Default maximum number of magazines"))
	int32 m_DefaultMaxNumberOfMagazines;

	/* The types and number of magazines that this inventory should spawn with */
	UPROPERTY(EditDefaultsOnly, Category = "Inventory", meta = (DisplayName = "Magazines to spawn with"))
	TMap<TSubclassOf<AMagazine>, int32> m_MagazinesToSpawnWith;

	/* Magazines and their count that are currently in this inventory */
	UPROPERTY(BlueprintReadOnly, Replicated, Category = "Inventory", meta = (DisplayName = "Stored magazines"))
	TArray<FMagazineStack> m_StoredMagazines;


	/////////////////////////////////////////////////////
						/* Getter */
	/////////////////////////////////////////////////////
public:
	/* Gets the maximum number of magazines for the given magazine type that can be stored in this inventory */
	UFUNCTION(BlueprintPure, Category = "Inventory")
	int32 GetMaxMagazineCountForType(TSubclassOf<AMagazine> MagazineType) const;

	/* Returns the number of magazines, from the given type, that are currently in this inventory */
	UFUNCTION(BlueprintPure, Category = "Inventory")
	int32 GetNumberOfMagazinesForType(TSubclassOf<AMagazine>& MagazineType) const;


	/////////////////////////////////////////////////////
					/* Check functions */
	/////////////////////////////////////////////////////
public:
	/**
	* Has this inventory enough space to store the given magazine?
	* @param MagazineType The given magazine to look for
	* @param NumMagazinesToStore How many of the given magazine we want to check for
	* @return True if we have enough space to store the given magazines
	*/
	UFUNCTION(BlueprintPure, Category = "Inventory")
	bool HasSpaceForMagazine(TSubclassOf<AMagazine> MagazineType, int32 NumMagazinesToStore = 1) const;

	/* Has this inventory the given number of magazines? */
	UFUNCTION(BlueprintPure, Category = "Inventory")
	bool HasMagazines(TSubclassOf<AMagazine> MagazineType, int32 NumMagazines = 1) const;


	/////////////////////////////////////////////////////
			/* Alter magazines in inventory */
	/////////////////////////////////////////////////////
public:
	/**
	 * Adds a number of magazines, from the given type, to this inventory
	 * @return True if the magazines where successfully added.
	 */
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	bool AddMagazineToInventory(TSubclassOf<AMagazine> MagazineType, int32 NumMagazinesToStore = 1);

private:
	/**
	 * DO NOT CALL THIS FUNCTION DIRECTLY.
	 * Server function to add magazines on the server to the inventory.
	 * Will check if there is enough room in the inventory to add the magazines.
	 * Is called by @see AddMagazineToInventory.
	 */
	UFUNCTION(BlueprintCallable, Server, WithValidation, Reliable, Category = "Inventory")
	void AddMagazineToInventory_Server(TSubclassOf<AMagazine> MagazineType, int32 NumMagazinesToStore = 1);

public:
	/**
	 * Removes a number of magazines, from the given type, from this inventory
	 * @return True if the magazines where successfully removed.
	 */
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	bool GetMagazineFromInventory(TSubclassOf<AMagazine> MagazineType, int32 NumMagazinesToRemove = 1);

	/**
	 * DO NOT CALL THIS FUNCTION DIRECTLY.
	 * Server function to remove magazines on the server from the inventory.
	 * Will check if there are enough magazines in the inventory.
	 * Is called by @see GetMagazineToInventory.
	 */
	UFUNCTION(BlueprintCallable, Server, WithValidation, Reliable, Category = "Inventory")
	void GetMagazineFromInventory_Server(TSubclassOf<AMagazine> MagazineType, int32 NumMagazinesToRemove = 1);


	/////////////////////////////////////////////////////
					/* Misc functions */
	/////////////////////////////////////////////////////
public:	
	USimpleInventory();	

protected:
	virtual void BeginPlay() override;

private:
	/* [Server] Adds the magazines to this inventory that are specified in @m_MazainesToSpawnWith */
	void SetupDefaultMagazines();

	FMagazineStack* FindMagazineStack(const TSubclassOf<AMagazine>& MagazineType);
	const FMagazineStack* FindMagazineStack(const TSubclassOf<AMagazine>& MagazineType) const;
};
