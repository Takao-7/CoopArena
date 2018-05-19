// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interactable.h"
#include "ItemBase.generated.h"


class UInventoryComponent;
class UUserWidget;
class UMeshComponent;


UENUM(BlueprintType)
enum class EItemType : uint8
{
	None,
	Weapon,
	Ammo,
	Tool
};


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


UCLASS(abstract)
class COOPARENA_API AItemBase : public AActor, public IInteractable
{
	GENERATED_BODY()

public:	
	/**
	 * Function to be called, when a pawn wants to pick this actor up.
	 * 
	 * @param NewOwner The pawn that wants to pick this actor up.
	 */
	UFUNCTION(BlueprintCallable, Category = ItemBase)
	virtual void OnPickUp(APawn* NewOwner);

	/**
	 * Function to be called when this actors gets dropped.
	 * 
	 * @param NewOwner The pawn that wants to pick this actor up.
	 * @param Inventory The pawn's inventory component.
	 */
	UFUNCTION(BlueprintCallable, Category = ItemBase)
	static void OnDrop(APawn* OldOwner);

	/* Interactable interface */

	virtual void OnBeginInteract_Implementation(APawn* InteractingPawn) override;
	virtual void OnEndInteract_Implementation(APawn* InteractingPawn) override;
	virtual UUserWidget* OnBeginLineTraceOver_Implementation(APawn* Pawn) override;
	virtual void OnEndLineTraceOver_Implementation(APawn* Pawn) override;

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = ItemBase)
	FItemStats _itemStats;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = ItemBase)
	UUserWidget* _itemWidget;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = ItemBase)
	UMeshComponent* _Mesh;
};