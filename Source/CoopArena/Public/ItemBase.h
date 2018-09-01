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
	UFUNCTION(BlueprintCallable, Category = ItemBase)
	virtual void SetSimulatePhysics(bool bSimulatePhysics);

	UFUNCTION(BlueprintPure, Category = ItemBase)
	virtual FORCEINLINE FItemStats& GetItemStats();

	UFUNCTION(BlueprintPure, Category = ItemBase)
	virtual void SetItemStats(FItemStats& newItemStats);

	UFUNCTION(BlueprintPure, Category = ItemBase)
	virtual FORCEINLINE UMeshComponent* GetMesh() const;

	/* Interactable interface */

	virtual void OnBeginInteract_Implementation(APawn* InteractingPawn, UPrimitiveComponent* HitComponent) override;
	virtual void OnEndInteract_Implementation(APawn* InteractingPawn) override;
	virtual UUserWidget* OnBeginLineTraceOver_Implementation(APawn* Pawn, UPrimitiveComponent* HitComponent) override;
	virtual void OnEndLineTraceOver_Implementation(APawn* Pawn) override;
	virtual void SetCanBeInteractedWith_Implementation(bool bCanbeInteractedWith) override;

	/* Interactable interface end */
protected:
	virtual void BeginPlay() override;	

	void SetUpInteractionVolume();

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = ItemBase, meta = (DisplayName = "Item Stats"))
	FItemStats _itemStats;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = ItemBase, meta = (DisplayName = "Item Widget"))
	UUserWidget* _itemWidget;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = ItemBase, meta = (DisplayName = "Interaction volume"))
	UShapeComponent* _InteractionVolume;
};