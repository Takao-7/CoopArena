// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interfaces/Interactable.h"
#include "Enums/ItemEnums.h"
#include "ItemBase.generated.h"


class UUserWidget;
class UMeshComponent;
class UShapeComponent;


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
	};

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
	TSubclassOf<class AItemBase> itemClass;
};


UCLASS(abstract)
class COOPARENA_API AItemBase : public AActor, public IInteractable
{
	GENERATED_BODY()

	/////////////////////////////////////////////////////
					/* Parameters */
	/////////////////////////////////////////////////////
protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = ItemBase, meta = (DisplayName = "Item Stats"))
	FItemStats _itemStats;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = ItemBase, meta = (DisplayName = "Item Widget"))
	UUserWidget* _itemWidget;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = ItemBase, meta = (DisplayName = "Interaction volume"))
	UShapeComponent* _InteractionVolume;

	FCollisionResponseContainer _collisionChannels;


	/////////////////////////////////////////////////////
				/* Item functions */
	/////////////////////////////////////////////////////
public:
	/* Sets simulate physics and collision. */
	UFUNCTION(BlueprintCallable, Category = ItemBase)
	virtual void ShouldSimulatePhysics(bool bSimulatePhysics);

	UFUNCTION(BlueprintPure, Category = ItemBase)
	virtual const FItemStats& GetItemStats() const;

	UFUNCTION(BlueprintPure, Category = ItemBase)
	virtual void SetItemStats(FItemStats& newItemStats);

	UFUNCTION(BlueprintPure, Category = ItemBase)
	virtual UMeshComponent* GetMesh() const;

	/* Function to call when this item is being dropped or spawned in the world without the intention to directly attach it to something. */
	UFUNCTION(BlueprintCallable, Category = ItemBase)
	void OnDrop();


	/////////////////////////////////////////////////////
				/* Interactable interface */
	/////////////////////////////////////////////////////
public:
	virtual void OnBeginInteract_Implementation(APawn* InteractingPawn, UPrimitiveComponent* HitComponent) override;
	//virtual void OnEndInteract_Implementation(APawn* InteractingPawn) override;
	virtual UUserWidget* OnBeginLineTraceOver_Implementation(APawn* Pawn, UPrimitiveComponent* HitComponent) override;
	virtual void OnEndLineTraceOver_Implementation(APawn* Pawn) override;
	virtual void SetCanBeInteractedWith_Implementation(bool bCanbeInteractedWith) override;


	/////////////////////////////////////////////////////
					/* Misc functions */
	/////////////////////////////////////////////////////
public:
	AItemBase();

protected:
	virtual void BeginPlay() override;		

	void SetUpInteractionVolume();
};