// Fill out your copyright notice in the Description page of Project Settings.

#include "ItemBase.h"
#include "Components/InventoryComponent.h"
#include "GameFramework/Pawn.h"
#include "PlayerCharacter.h"
#include "Components/StaticMeshComponent.h"
#include "Components/ShapeComponent.h"
#include "CoopArena.h"


AItemBase::AItemBase()
{
	SetReplicates(true);
	SetReplicateMovement(false);
	bNetUseOwnerRelevancy = true;


	_collisionChannels.Visibility = ECR_Block;
	_collisionChannels.Camera = ECR_Block;
	_collisionChannels.GameTraceChannel1 = ECR_Ignore;	// Projectile
	_collisionChannels.GameTraceChannel2 = ECR_Block;	// Interactable
	_collisionChannels.GameTraceChannel3 = ECR_Ignore;	// Projectile penetration
}

/////////////////////////////////////////////////////
void AItemBase::ShouldSimulatePhysics(bool bSimulatePhysics)
{
	if (GetMesh())
	{
		GetMesh()->SetSimulatePhysics(bSimulatePhysics);
		if (bSimulatePhysics)
		{
			GetMesh()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		}
		else
		{
			GetMesh()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		}
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("No mesh on %s."), *GetName());
	}
}

/////////////////////////////////////////////////////
FItemStats& AItemBase::GetItemStats()
{
	return _itemStats;
}

/////////////////////////////////////////////////////
void AItemBase::SetItemStats(FItemStats& newItemStats)
{
	_itemStats = newItemStats;
}

/////////////////////////////////////////////////////
UMeshComponent* AItemBase::GetMesh() const
{
	return nullptr;
}

/////////////////////////////////////////////////////
void AItemBase::OnBeginInteract_Implementation(APawn* InteractingPawn, UPrimitiveComponent* HitComponent)
{
	UInventoryComponent* inventory = Cast<UInventoryComponent>(InteractingPawn->GetComponentByClass(UInventoryComponent::StaticClass()));
	if (inventory == nullptr)
	{
		return;
	}

	bool bItemSuccessfullyAdded = inventory->AddItem(_itemStats, 1.0f);
	if (bItemSuccessfullyAdded)
	{
		Destroy();
	}
}

/////////////////////////////////////////////////////
UUserWidget* AItemBase::OnBeginLineTraceOver_Implementation(APawn* Pawn, UPrimitiveComponent* HitComponent)
{
	if (GetMesh())
	{
		GetMesh()->SetRenderCustomDepth(true);
	}
	return _itemWidget;
}

/////////////////////////////////////////////////////
void AItemBase::OnEndLineTraceOver_Implementation(APawn* Pawn)
{
	if (GetMesh())
	{
		GetMesh()->SetRenderCustomDepth(false);
	}
}

/////////////////////////////////////////////////////
void AItemBase::SetCanBeInteractedWith_Implementation(bool bCanbeInteractedWith)
{
	if (bCanbeInteractedWith)
	{
		_InteractionVolume->SetCollisionResponseToChannel(ECC_Interactable, ECR_Block);
		if (GetMesh())
		{
			GetMesh()->SetCollisionResponseToChannel(ECC_Interactable, ECR_Block);
		}
	}
	else
	{
		_InteractionVolume->SetCollisionResponseToChannel(ECC_Interactable, ECR_Ignore);
		if (GetMesh())
		{
			GetMesh()->SetCollisionResponseToChannel(ECC_Interactable, ECR_Ignore);
		}
	}
}

/////////////////////////////////////////////////////
void AItemBase::BeginPlay()
{
	Super::BeginPlay();

	if (GetMesh())
	{
		GetMesh()->SetCustomDepthStencilValue(253);
		GetMesh()->SetCollisionResponseToChannels(_collisionChannels);
	}

	if (GetOwner() == nullptr)
	{
		OnDrop();
	}
	else
	{
		ShouldSimulatePhysics(false);
		IInteractable::Execute_SetCanBeInteractedWith(this, false);
	}
}

/////////////////////////////////////////////////////
void AItemBase::OnDrop()
{
	DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);

	ShouldSimulatePhysics(true);
	IInteractable::Execute_SetCanBeInteractedWith(this, true);

	SetOwner(nullptr);

	if (HasAuthority())
	{
		SetReplicates(true);
		SetReplicateMovement(true);
	}
}

/////////////////////////////////////////////////////
void AItemBase::SetUpInteractionVolume()
{
	_InteractionVolume->SetCollisionResponseToAllChannels(ECR_Ignore);
	_InteractionVolume->SetCollisionResponseToChannel(ECC_Interactable, ECR_Block);
	_InteractionVolume->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	_InteractionVolume->SetupAttachment(RootComponent);
}