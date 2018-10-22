// Fill out your copyright notice in the Description page of Project Settings.

#include "Interactables/Door.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/Pawn.h"
#include "Components/ArrowComponent.h"


// Sets default values
ADoor::ADoor()
{
	PrimaryActorTick.bCanEverTick = true;

	Door = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Door"));
	Door->SetCollisionResponseToAllChannels(ECR_Block);
	Door->SetCollisionResponseToChannel(ECC_Visibility, ECR_Ignore);
	Door->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);
	RootComponent = Door;

	Front = CreateDefaultSubobject <UArrowComponent>(TEXT("Front"));
	Front->SetupAttachment(RootComponent);
	Front->SetRelativeLocation(FVector(0.0f, 0.0f, 50.0f));

	OpeningAngle = 90.0f;
	OpeningSpeed = 5.0f;

	Door->CustomDepthStencilValue = 253;
}


/////////////////////////////////////////////////////
void ADoor::BeginPlay()
{
	Super::BeginPlay();

	PrimaryActorTick.SetTickFunctionEnable(false);
}


/////////////////////////////////////////////////////
void ADoor::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	float newAngle = FMath::FInterpTo(Door->GetRelativeTransform().GetRotation().Rotator().Yaw, _TargetAngle, DeltaSeconds, OpeningSpeed);
	Door->SetRelativeRotation(FRotator(0.f, newAngle, 0.f), true);

	if (FMath::IsNearlyEqual(Door->GetRelativeTransform().GetRotation().Rotator().Yaw, _TargetAngle, 0.01f))
	{
		if (_TargetAngle == 0.0f)
		{
			bIsOpen = false;
		}
		PrimaryActorTick.SetTickFunctionEnable(false);
	}
}


/////////////////////////////////////////////////////
void ADoor::OnBeginInteract_Implementation(APawn* InteractingPawn, UPrimitiveComponent* HitComponent)
{
	if (bIsOpen)
	{
		bIsOpen = false;
		_TargetAngle = 0.0f;
	}
	else
	{
		bIsOpen = true;
		if (bTwoSidedOpening)
		{
			FVector pawnLocation = InteractingPawn->GetActorLocation();
			FVector relativePawnPosition = GetActorTransform().InverseTransformPosition(pawnLocation);

			if (relativePawnPosition.X > 0.0f)
			{
				_TargetAngle = -OpeningAngle;
			}
			else
			{
				_TargetAngle = OpeningAngle;
			}
		}
		else
		{
			_TargetAngle = OpeningAngle;
		}
	}
	PrimaryActorTick.SetTickFunctionEnable(true);
}


UUserWidget* ADoor::OnBeginLineTraceOver_Implementation(APawn* Pawn, UPrimitiveComponent* HitComponent)
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