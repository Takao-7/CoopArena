// Fill out your copyright notice in the Description page of Project Settings.

#include "ItemBase.h"
#include "Components/InventoryComponent.h"
#include "GameFramework/Pawn.h"
#include "PlayerCharacter.h"
#include "Components/StaticMeshComponent.h"


void AItemBase::SetSimulatePhysics(bool bSimulatePhysics)
{
	if (GetMesh())
	{
		if (bSimulatePhysics)
		{
			GetMesh()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		}
		else
		{
			GetMesh()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		}
		GetMesh()->SetSimulatePhysics(bSimulatePhysics);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("No mesh on %s."), *GetName());
	}
}

const FItemStats& AItemBase::GetItemStats() const
{
	return _itemStats;
}


void AItemBase::SetItemStats(FItemStats& newItemStats)
{
	_itemStats = newItemStats;
}


UMeshComponent* AItemBase::GetMesh() const
{
	return nullptr;
}

void AItemBase::OnBeginInteract_Implementation(APawn* InteractingPawn, UPrimitiveComponent* HitComponent)
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


UUserWidget* AItemBase::OnBeginLineTraceOver_Implementation(APawn* Pawn, UPrimitiveComponent* HitComponent)
{
	if (GetMesh())
	{
		GetMesh()->SetRenderCustomDepth(true);
	}
	return _itemWidget;
}


void AItemBase::OnEndLineTraceOver_Implementation(APawn* Pawn)
{
	if (GetMesh())
	{
		GetMesh()->SetRenderCustomDepth(false);
	}
}