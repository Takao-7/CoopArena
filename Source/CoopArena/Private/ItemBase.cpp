// Fill out your copyright notice in the Description page of Project Settings.

#include "ItemBase.h"
#include "InventoryComponent.h"
#include "Components/MeshComponent.h"


void AItemBase::OnPickUp(APawn* NewOwner)
{

}


void AItemBase::OnDrop(APawn* OldOwner)
{

}


void AItemBase::OnBeginInteract_Implementation(APawn* InteractingPawn)
{

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