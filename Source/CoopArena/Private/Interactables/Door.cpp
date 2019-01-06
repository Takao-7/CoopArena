// Fill out your copyright notice in the Description page of Project Settings.

#include "Interactables/Door.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/Pawn.h"
#include "Components/ArrowComponent.h"
#include "Components/BoxComponent.h"
#include "UnrealNetwork.h"
#include "CoopArena.h"
#include "Bot.h"


ADoor::ADoor()
{
	PrimaryActorTick.bCanEverTick = true;

	_DoorMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Door"));
	_DoorMesh->SetCollisionResponseToAllChannels(ECR_Block);
	_DoorMesh->SetCollisionResponseToChannel(ECC_Visibility, ECR_Ignore);
	_DoorMesh->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);
	_DoorMesh->SetCanEverAffectNavigation(false);
	RootComponent = _DoorMesh;

	_Front = CreateDefaultSubobject <UArrowComponent>(TEXT("Front"));
	_Front->SetupAttachment(RootComponent);
	_Front->SetRelativeLocation(FVector(0.0f, 0.0f, 50.0f));

	_InteractionBox = CreateDefaultSubobject<UBoxComponent>(TEXT("InteractionBox"));
	_InteractionBox->SetCollisionResponseToAllChannels(ECR_Ignore);
	_InteractionBox->SetCollisionResponseToChannel(ECC_Interactable, ECR_Block);
	_InteractionBox->SetupAttachment(RootComponent);

	_BotInteractionVolume = CreateDefaultSubobject<UBoxComponent>(TEXT("Bot interaction volume"));
	_BotInteractionVolume->SetCollisionResponseToAllChannels(ECR_Ignore);
	_BotInteractionVolume->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	_BotInteractionVolume->SetGenerateOverlapEvents(true);
	_BotInteractionVolume->SetupAttachment(RootComponent);
	_BotInteractionVolume->SetRelativeLocation(FVector(45.0f, 0.0f, 85.0f));
	_BotInteractionVolume->SetBoxExtent(FVector(100.0f, 100.0f, 150.0f));

	_OpeningAngle = 90.0f;
	_OpeningSpeed = 5.0f;

	_DoorMesh->CustomDepthStencilValue = 253;

	SetReplicates(true);
}

/////////////////////////////////////////////////////
void ADoor::HandleOnPawnOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult & SweepResult)
{
	const bool bIsBot = Cast<ABot>(OtherActor);
	if (_bIsOpen || !bIsBot)
	{
		return;
	}

	HandleInteract_Server(Cast<APawn>(OtherActor), nullptr);
}

/////////////////////////////////////////////////////
void ADoor::BeginPlay()
{
	Super::BeginPlay();
	PrimaryActorTick.SetTickFunctionEnable(false);
	_BotInteractionVolume->OnComponentBeginOverlap.AddDynamic(this, &ADoor::HandleOnPawnOverlap);
}

/////////////////////////////////////////////////////
void ADoor::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	const float yaw = _DoorMesh->GetRelativeTransform().GetRotation().Rotator().Yaw;
	if (FMath::IsNearlyEqual(yaw, _TargetAngle, 0.01f))
	{
		if (_TargetAngle == 0.0f)
		{
			_bIsOpen = false;
		}
		PrimaryActorTick.SetTickFunctionEnable(false);
	}
	else
	{
		const float newAngle = FMath::FInterpTo(yaw, _TargetAngle, DeltaSeconds, _OpeningSpeed);
		_DoorMesh->SetRelativeRotation(FRotator(0.f, newAngle, 0.f), true);
	}
}

/////////////////////////////////////////////////////
void ADoor::OnBeginInteract_Implementation(APawn* InteractingPawn, UPrimitiveComponent* HitComponent)
{
	HandleInteract_Server(InteractingPawn, HitComponent);
}

UUserWidget* ADoor::OnBeginLineTraceOver_Implementation(APawn* Pawn, UPrimitiveComponent* HitComponent)
{	
	if (_DoorMesh)
	{
		_DoorMesh->SetRenderCustomDepth(true);
	}
	return nullptr;
}

void ADoor::OnEndLineTraceOver_Implementation(APawn* Pawn)
{
	if (_DoorMesh)
	{
		_DoorMesh->SetRenderCustomDepth(false);
	}
}

/////////////////////////////////////////////////////
void ADoor::HandleInteract_Server_Implementation(APawn* InteractingPawn, UPrimitiveComponent* HitComponent)
{
	if (_bIsOpen)
	{
		_bIsOpen = false;
		_TargetAngle = 0.0f;
	}
	else
	{
		_bIsOpen = true;
		if (_bTwoSidedOpening)
		{
			const FVector pawnLocation = InteractingPawn->GetActorLocation();
			const FVector relativePawnPosition = GetActorTransform().InverseTransformPosition(pawnLocation);

			_TargetAngle = relativePawnPosition.X > 0.0f ? -_OpeningAngle : _OpeningAngle;
		}
		else
		{
			_TargetAngle = _OpeningAngle;
		}
	}
	EnableTickFunction_Multicast(_TargetAngle, _bIsOpen);
}

bool ADoor::HandleInteract_Server_Validate(APawn* InteractingPawn, UPrimitiveComponent* HitComponent)
{
	return true;
}

/////////////////////////////////////////////////////
void ADoor::EnableTickFunction_Multicast_Implementation(float TargetAngle, bool bIsOpen)
{
	_TargetAngle = TargetAngle;
	_bIsOpen = bIsOpen;
	PrimaryActorTick.SetTickFunctionEnable(true);
}