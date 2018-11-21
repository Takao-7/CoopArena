// Fill out your copyright notice in the Description page of Project Settings.

#include "PickUp.h"
#include "Components/SimpleInventory.h"
#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SphereComponent.h"
#include "GameFramework/Pawn.h"
#include "CoopArena.h"


APickUp::APickUp()
{
	PrimaryActorTick.bCanEverTick = true;
	_bCanBeInteractedWith = true;
	_RotationSpeed = 10.0f;
	const float edgeLength = 75.0f;

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
	_TargetRotation = FRotator(FMath::RandRange(0.0f, 360.0f), FMath::RandRange(0.0f, 360.0f), FMath::RandRange(0.0f, 360.0f));
}

/////////////////////////////////////////////////////
void APickUp::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	/*const FRotator meshRotation = _Mesh->GetComponentRotation();
	if (meshRotation.Equals(_TargetRotation, 1.0f))
	{
		_TargetRotation.Yaw = FMath::RandRange(0.0f, 360.0f);
		_TargetRotation.Pitch = FMath::RandRange(0.0f, 360.0f);
		_TargetRotation.Roll = FMath::RandRange(0.0f, 360.0f);
	}

	const FRotator newRotation = FMath::RInterpTo(meshRotation, _TargetRotation, DeltaSeconds, _RotationSpeed);
	_Mesh->SetWorldRotation(newRotation);*/

	const float minRotation = 0.1f;
	const float maxRotation = 0.25f;
	const FRotator randRotation(FMath::RandRange(minRotation, maxRotation), FMath::RandRange(minRotation, maxRotation), FMath::RandRange(minRotation, maxRotation));

	_Mesh->AddWorldRotation(randRotation);
}

/////////////////////////////////////////////////////
void APickUp::SetMagazineStack(const FMagazineStack& Stack)
{
	_MagazineStack = FMagazineStack(Stack);
}

void APickUp::SetMagazineStack(TSubclassOf<AMagazine> MagClass, int32 NumMags)
{
	_MagazineStack = FMagazineStack(MagClass, NumMags);
}

/////////////////////////////////////////////////////
			/* Interactable interface */
/////////////////////////////////////////////////////
void APickUp::OnBeginInteract_Implementation(APawn* InteractingPawn, UPrimitiveComponent* HitComponent)
{
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
		else if (freeSpace != 0)
		{
			inventory->AddMagazineToInventory(_MagazineStack.magClass, freeSpace);
			_MagazineStack.stackSize -= freeSpace;
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
