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
	bNetUseOwnerRelevancy = true;
	ShouldSimulatePhysics(false);
	IInteractable::Execute_SetCanBeInteractedWith(this, true);
}

/////////////////////////////////////////////////////
void AItemBase::ShouldSimulatePhysics(bool bSimulatePhysics)
{
	if (GetMesh())
	{
		GetMesh()->SetSimulatePhysics(bSimulatePhysics);
		if (bSimulatePhysics)
		{
			SetReplicateMovement(true);
			GetMesh()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		}
		else
		{
			SetReplicateMovement(false);
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
	Server_OnBeginInteract(InteractingPawn, HitComponent);
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
	}

	if (GetOwner() == nullptr)
	{
		OnDrop();
	}
}

/////////////////////////////////////////////////////
void AItemBase::OnDrop()
{
	DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);

	ShouldSimulatePhysics(true);
	IInteractable::Execute_SetCanBeInteractedWith(this, true);

	if (HasAuthority())
	{
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


/////////////////////////////////////////////////////
					/* Networking */
/////////////////////////////////////////////////////
void AItemBase::Server_OnBeginInteract_Implementation(APawn* InteractingPawn, UPrimitiveComponent* HitComponent)
{
	UInventoryComponent* inventory = Cast<UInventoryComponent>(InteractingPawn->GetComponentByClass(UInventoryComponent::StaticClass()));

	bool bItemSuccessfullyAdded = inventory->AddItem(_itemStats, 1.0f);
	if (bItemSuccessfullyAdded)
	{
		Destroy();
	}
}

bool AItemBase::Server_OnBeginInteract_Validate(APawn* InteractingPawn, UPrimitiveComponent* HitComponent)
{
	return _InteractionVolume->GetCollisionResponseToChannel(ECC_Interactable) == ECR_Block;
}