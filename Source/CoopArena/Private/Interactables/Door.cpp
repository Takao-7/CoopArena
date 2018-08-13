// Fill out your copyright notice in the Description page of Project Settings.

#include "Interactables/Door.h"
#include "Components/StaticMeshComponent.h"


// Sets default values
ADoor::ADoor()
{
	PrimaryActorTick.bCanEverTick = true;

	DoorFrame = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("DoorFrame"));
	Door = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Door"));

	RootComponent = DoorFrame;
	Door->SetupAttachment(RootComponent);
	Door->SetRelativeLocation(FVector(0.0f, 45.0f, 0.0f));

	OpeningAngle = 90.0f;
	OpeningSpeed = 5.0f;

	Door->CustomDepthStencilValue = 253;
}


// Called when the game starts or when spawned
void ADoor::BeginPlay()
{
	Super::BeginPlay();

	PrimaryActorTick.SetTickFunctionEnable(false);
}


void ADoor::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	float newAngle = FMath::FInterpTo(Door->GetRelativeTransform().GetRotation().Rotator().Yaw, _TargetAngle, DeltaSeconds, OpeningSpeed);
	Door->SetRelativeRotation(FRotator(0.f, newAngle, 0.f), true);

	if (FMath::IsNearlyEqual(Door->GetRelativeTransform().GetRotation().Rotator().Yaw, _TargetAngle, 0.1F))
	{
		PrimaryActorTick.SetTickFunctionEnable(false);
	}
}


/////////////////////////////////////////////////////
void ADoor::OnBeginInteract_Implementation(APawn* InteractingPawn)
{
	if (_TargetAngle == 0.0f)
	{
		_TargetAngle = OpeningAngle;		
		_LastOpeningAngle = _TargetAngle;
	}
	else
	{
		_TargetAngle = 0.0f;
		_LastOpeningAngle = 0.0f;
	}
	PrimaryActorTick.SetTickFunctionEnable(true);	
}


UUserWidget* ADoor::OnBeginLineTraceOver_Implementation(APawn* Pawn)
{
	if (Door)
	{
		Door->SetRenderCustomDepth(true);
	}

	return nullptr;
}


void ADoor::OnEndLineTraceOver_Implementation(APawn* Pawn)
{
	if (Door)
	{
		Door->SetRenderCustomDepth(false);
	}
}