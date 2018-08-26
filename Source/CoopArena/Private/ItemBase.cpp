// Fill out your copyright notice in the Description page of Project Settings.

#include "ItemBase.h"
#include "InventoryComponent.h"
#include "GameFramework/Pawn.h"
#include "PlayerCharacter.h"
#include "Components/MeshComponent.h"


const FItemStats& AItemBase::GetItemStats() const
{
	return _itemStats;
}


void AItemBase::SetItemStats(FItemStats& newItemStats)
{
	_itemStats = newItemStats;
}


void AItemBase::OnBeginInteract_Implementation(APawn* InteractingPawn)
{
	TArray<UActorComponent*> inventoryActorComponents = InteractingPawn->GetComponentsByClass(UInventoryComponent::StaticClass());
	UInventoryComponent* inventory;

	if (inventoryActorComponents.Num() == 0)
	{
		return;
	}
	else if (inventoryActorComponents.Num() == 1)
	{
		inventory = Cast<UInventoryComponent>(inventoryActorComponents[0]);
	}
	else
	{
		/* More then one inventory is existing. Find the correct one to store this item */
		/* TODO: Implement system to find the correct inventory. */
		inventory = FindCorrectInventory(inventoryActorComponents);
	}	
	bool bItemSuccessfullyAdded = inventory->AddItem(this);
	if (bItemSuccessfullyAdded)
	{
		Destroy();
	}
}


UInventoryComponent* AItemBase::FindCorrectInventory(TArray<UActorComponent*> inventoryActorComponents) const
{
	return Cast<UInventoryComponent>(inventoryActorComponents[0]);
}


void AItemBase::OnEndInteract_Implementation(APawn* InteractingPawn)
{

}


UUserWidget* AItemBase::OnBeginLineTraceOver_Implementation(APawn* Pawn)
{
	if (_Mesh)
	{
		_Mesh->SetRenderCustomDepth(true);
	}
	return _itemWidget;
}


void AItemBase::OnEndLineTraceOver_Implementation(APawn* Pawn)
{
	if (_Mesh)
	{
		_Mesh->SetRenderCustomDepth(false);
	}
}