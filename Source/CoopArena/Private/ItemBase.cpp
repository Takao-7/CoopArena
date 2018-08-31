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


FItemStats& AItemBase::GetItemStats()
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
	UInventoryComponent* inventory = Cast<UInventoryComponent>(InteractingPawn->GetComponentByClass(UInventoryComponent::StaticClass()));

	bool bItemSuccessfullyAdded = inventory->AddItem(_itemStats, 1.0f);
	if (bItemSuccessfullyAdded)
	{
		Destroy();
	}
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


void AItemBase::BeginPlay()
{
	Super::BeginPlay();

	if (GetMesh())
	{
		GetMesh()->SetCustomDepthStencilValue(253);
	}
}
