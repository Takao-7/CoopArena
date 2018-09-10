// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interfaces/Interactable.h"
#include "Structs/ItemStructs.h"
#include "ItemBase.generated.h"


class UInventoryComponent;
class UUserWidget;
class UMeshComponent;
class UInventoryComponent;
class UShapeComponent;


UCLASS(abstract)
class COOPARENA_API AItemBase : public AActor, public IInteractable
{
	GENERATED_BODY()

public:
<<<<<<< HEAD
	UFUNCTION(BlueprintCallable, Category = ItemBase)
	virtual void SetSimulatePhysics(bool bSimulatePhysics);
=======
	AItemBase();

	/* Sets simulate physics and collision. */
	UFUNCTION(BlueprintCallable, Category = ItemBase)
	virtual void ShouldSimulatePhysics(bool bSimulatePhysics);
>>>>>>> 17f86cef60dd7dd576fc030497f09716282c8ed8

	UFUNCTION(BlueprintPure, Category = ItemBase)
	virtual FORCEINLINE FItemStats& GetItemStats();

	UFUNCTION(BlueprintPure, Category = ItemBase)
	virtual void SetItemStats(FItemStats& newItemStats);

	UFUNCTION(BlueprintPure, Category = ItemBase)
	virtual FORCEINLINE UMeshComponent* GetMesh() const;

<<<<<<< HEAD
=======
	/* Function to call when this item is being dropped or spawned in the world without the intention to directly attach it to something. */
	UFUNCTION(BlueprintCallable, Category = ItemBase)
	void OnDrop();

>>>>>>> 17f86cef60dd7dd576fc030497f09716282c8ed8
	/* Interactable interface */

	virtual void OnBeginInteract_Implementation(APawn* InteractingPawn, UPrimitiveComponent* HitComponent) override;
	//virtual void OnEndInteract_Implementation(APawn* InteractingPawn) override;
	virtual UUserWidget* OnBeginLineTraceOver_Implementation(APawn* Pawn, UPrimitiveComponent* HitComponent) override;
	virtual void OnEndLineTraceOver_Implementation(APawn* Pawn) override;
	virtual void SetCanBeInteractedWith_Implementation(bool bCanbeInteractedWith) override;

	/* Interactable interface end */
protected:
<<<<<<< HEAD
	virtual void BeginPlay() override;	
=======
	virtual void BeginPlay() override;		
>>>>>>> 17f86cef60dd7dd576fc030497f09716282c8ed8

	void SetUpInteractionVolume();

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = ItemBase, meta = (DisplayName = "Item Stats"))
	FItemStats _itemStats;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = ItemBase, meta = (DisplayName = "Item Widget"))
	UUserWidget* _itemWidget;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = ItemBase, meta = (DisplayName = "Interaction volume"))
	UShapeComponent* _InteractionVolume;
<<<<<<< HEAD
=======

	FCollisionResponseContainer _collisionChannels;
>>>>>>> 17f86cef60dd7dd576fc030497f09716282c8ed8
};