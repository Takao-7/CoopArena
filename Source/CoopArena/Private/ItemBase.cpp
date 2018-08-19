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
	UActorComponent* inventoryActorComponents = InteractingPawn->GetComponentByClass(UInventoryComponent::StaticClass());
	UInventoryComponent* inventory = Cast<UInventoryComponent>(inventoryActorComponents);
	
	if (inventory)
	{
		bool bItemSuccessfullyAdded = inventory->AddItem(this);
		if (bItemSuccessfullyAdded)
		{
			Destroy();
		}
	}
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