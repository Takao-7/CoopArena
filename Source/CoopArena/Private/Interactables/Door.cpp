// Fill out your copyright notice in the Description page of Project Settings.

#include "Interactables/Door.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/Pawn.h"
#include "Components/ArrowComponent.h"
#include "Components/BoxComponent.h"
#include "UnrealNetwork.h"
#include "CoopArena.h"


ADoor::ADoor()
{
	PrimaryActorTick.bCanEverTick = true;

	Door = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Door"));
	Door->SetCollisionResponseToAllChannels(ECR_Block);
	Door->SetCollisionResponseToChannel(ECC_Visibility, ECR_Ignore);
	Door->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);
	Door->SetCanEverAffectNavigation(false);
	RootComponent = Door;

	Front = CreateDefaultSubobject <UArrowComponent>(TEXT("Front"));
	Front->SetupAttachment(RootComponent);
	Front->SetRelativeLocation(FVector(0.0f, 0.0f, 50.0f));

	InteractionBox = CreateDefaultSubobject<UBoxComponent>("InteractionBox");
	InteractionBox->SetCollisionResponseToAllChannels(ECR_Ignore);
	InteractionBox->SetCollisionResponseToChannel(ECC_Interactable, ECR_Block);
	InteractionBox->SetupAttachment(RootComponent);

	OpeningAngle = 90.0f;
	m_OpeningSpeed = 5.0f;

	Door->CustomDepthStencilValue = 253;

	SetReplicates(true);
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

	const float yaw = Door->GetRelativeTransform().GetRotation().Rotator().Yaw;
	if (FMath::IsNearlyEqual(yaw, m_TargetAngle, 0.01f))
	{
		if (m_TargetAngle == 0.0f)
		{
			m_bIsOpen = false;
		}
		PrimaryActorTick.SetTickFunctionEnable(false);
	}
	else
	{
		const float newAngle = FMath::FInterpTo(yaw, m_TargetAngle, DeltaSeconds, m_OpeningSpeed);
		Door->SetRelativeRotation(FRotator(0.f, newAngle, 0.f), true);
	}
}

/////////////////////////////////////////////////////
void ADoor::OnBeginInteract_Implementation(APawn* InteractingPawn, UPrimitiveComponent* HitComponent)
{
	HandleInteract_Server(InteractingPawn, HitComponent);
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

/////////////////////////////////////////////////////
void ADoor::HandleInteract_Server_Implementation(APawn* InteractingPawn, UPrimitiveComponent* HitComponent)
{
	if (m_bIsOpen)
	{
		m_bIsOpen = false;
		m_TargetAngle = 0.0f;
	}
	else
	{
		m_bIsOpen = true;
		if (bTwoSidedOpening)
		{
			const FVector pawnLocation = InteractingPawn->GetActorLocation();
			const FVector relativePawnPosition = GetActorTransform().InverseTransformPosition(pawnLocation);

			m_TargetAngle = relativePawnPosition.X > 0.0f ? -OpeningAngle : OpeningAngle;
		}
		else
		{
			m_TargetAngle = OpeningAngle;
		}
	}
	EnableTickFunction_Multicast(m_TargetAngle, m_bIsOpen);
}

bool ADoor::HandleInteract_Server_Validate(APawn* InteractingPawn, UPrimitiveComponent* HitComponent)
{
	return true;
}

/////////////////////////////////////////////////////
void ADoor::EnableTickFunction_Multicast_Implementation(float TargetAngle, bool bIsOpen)
{
	m_TargetAngle = TargetAngle;
	m_bIsOpen = bIsOpen;
	PrimaryActorTick.SetTickFunctionEnable(true);
}