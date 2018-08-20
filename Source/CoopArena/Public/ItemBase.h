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


UCLASS(abstract)
class COOPARENA_API AItemBase : public AActor, public IInteractable
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintPure, Category = ItemBase)
	virtual const FItemStats& GetItemStats() const;

	UFUNCTION(BlueprintPure, Category = ItemBase)
	virtual void SetItemStats(FItemStats& newItemStats);

	/* Interactable interface */
	virtual void OnBeginInteract_Implementation(APawn* InteractingPawn) override;
	virtual void OnEndInteract_Implementation(APawn* InteractingPawn) override;
	virtual UUserWidget* OnBeginLineTraceOver_Implementation(APawn* Pawn) override;
	virtual void OnEndLineTraceOver_Implementation(APawn* Pawn) override;
	/* Interactable interface end */

protected:
	UFUNCTION(BlueprintCallable, Category = ItemBase)
	virtual UInventoryComponent* FindCorrectInventory(TArray<UActorComponent*> inventoryActorComponents) const;

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = ItemBase)
	FItemStats _itemStats;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = ItemBase)
	UUserWidget* _itemWidget;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = ItemBase)
	UMeshComponent* _Mesh;
};