// Fill out your copyright notice in the Description page of Project Settings.

#include "PickUp.h"
#include "Components/SimpleInventory.h"
#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SphereComponent.h"
#include "GameFramework/Pawn.h"
#include "CoopArena.h"
#include "UnrealNetwork.h"


APickUp::APickUp()
{
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;
	bReplicateMovement = false;
	_bCanBeInteractedWith = true;
	_RotationSpeed = 20.0f;
	_LiveSpan = 60.0f;
	const float edgeLength = 60.0f;

	_Collision = CreateDefaultSubobject<UBoxComponent>("Collision");
	_Collision->SetCollisionResponseToAllChannels(ECR_Ignore);
	_Collision->SetCollisionResponseToChannel(ECC_WorldDynamic, ECR_Block);
	_Collision->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Block);
	_Collision->SetCollisionObjectType(ECC_PhysicsBody);
	_Collision->SetCollisionEnabled(ECollisionEnabled::PhysicsOnly);
	_Collision->SetSimulatePhysics(true);
	_Collision->SetBoxExtent(FVector(edgeLength));
	RootComponent = _Collision;

	_Mesh = CreateDefaultSubobject<UStaticMeshComponent>("Mesh");
	_Mesh->SetCollisionResponseToAllChannels(ECR_Ignore);
	_Mesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	_Mesh->SetRelativeScale3D(FVector(3.0f));
	_Mesh->SetIsReplicated(true);
	_Mesh->SetupAttachment(RootComponent);

	_InteractionBox = CreateDefaultSubobject<UBoxComponent>("InteractionBox");
	_InteractionBox->SetCollisionResponseToAllChannels(ECR_Ignore);
	_InteractionBox->SetCollisionResponseToChannel(ECC_Interactable, ECR_Block);
	_InteractionBox->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	_InteractionBox->SetupAttachment(RootComponent);
}

/////////////////////////////////////////////////////
void APickUp::BeginPlay()
{
	Super::BeginPlay();

	if (_MagazineStack.magClass)
	{
		const AMagazine* defaultMagazine = Cast<AMagazine>(_MagazineStack.magClass->GetDefaultObject(true));
		const UStaticMeshComponent* meshComp = Cast<UStaticMeshComponent>(defaultMagazine->GetMesh());
		if (meshComp)
		{
			_Mesh->SetStaticMesh(meshComp->GetStaticMesh());
			_Mesh->SetCustomDepthStencilValue(253);
		}
	}

	_Pitch = _RotationSpeed * (FMath::RandBool() ? 1.0f : -1.0f);
	_Yaw = _RotationSpeed * (FMath::RandBool() ? 1.0f : -1.0f);
	_Roll = _RotationSpeed * (FMath::RandBool() ? 1.0f : -1.0f);
	_RotationDelta = FRotator(_Pitch, _Yaw, _Roll);

	SetLifeSpan(_LiveSpan);	
}

/////////////////////////////////////////////////////
void APickUp::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	_Mesh->AddWorldRotation(_RotationDelta * DeltaSeconds);
}

/////////////////////////////////////////////////////
void APickUp::SetMagazineStack(TSubclassOf<AMagazine> MagClass, int32 NumMags)
{
	ensureMsgf(NumMags > 0, TEXT("The number of magazines must be greater than zero!"));
	_MagazineStack = FMagazineStack(MagClass, NumMags);
}


/////////////////////////////////////////////////////
			/* Interactable interface */
/////////////////////////////////////////////////////
void APickUp::OnBeginInteract_Implementation(APawn* InteractingPawn, UPrimitiveComponent* HitComponent)
{
	ensureMsgf(HasAuthority(), TEXT("Don't call this function as a client!"));
	USimpleInventory* inventory = Cast<USimpleInventory>(InteractingPawn->GetComponentByClass(USimpleInventory::StaticClass()));
	if (inventory)
	{
		int32 freeSpace = 0;
		const bool bHasRoom = inventory->HasSpaceForMagazine(_MagazineStack.magClass, freeSpace, _MagazineStack.stackSize);
		if (bHasRoom)
		{
			inventory->AddMagazineToInventory(_MagazineStack.magClass, _MagazineStack.stackSize);
			Destroy();
		}
		else if (freeSpace > 0)
		{
			inventory->AddMagazineToInventory(_MagazineStack.magClass, freeSpace);
			_MagazineStack.stackSize -= freeSpace;
			ensure(_MagazineStack.stackSize > 0);
		}
	}
}

UUserWidget* APickUp::OnBeginLineTraceOver_Implementation(APawn* Pawn, UPrimitiveComponent* HitComponent)
{
	_Mesh->SetRenderCustomDepth(true);
	return _Widget;
}

void APickUp::OnEndLineTraceOver_Implementation(APawn* Pawn)
{
	_Mesh->SetRenderCustomDepth(false);
}

void APickUp::SetCanBeInteractedWith_Implementation(bool bCanbeInteractedWith)
{
	this->_bCanBeInteractedWith = bCanbeInteractedWith;
}
