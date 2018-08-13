// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interfaces/Interactable.h"
#include "Door.generated.h"


class UStaticMeshComponent;

UCLASS()
class COOPARENA_API ADoor : public AActor, public IInteractable
{
	GENERATED_BODY()


protected:
	UPROPERTY(VisibleDefaultsOnly, Category = Door)
	UStaticMeshComponent* DoorFrame;

	UPROPERTY(VisibleDefaultsOnly, Category = Door)
	UStaticMeshComponent* Door;

	/* The angle that the opened door will have. Negative values mean that the door will open to the back. */
	UPROPERTY(EditDefaultsOnly, Category = Door, meta = (ClampMax = 135.0, ClampMin = -135.0))
	float OpeningAngle;

	/* How fast the door will open */
	UPROPERTY(EditDefaultsOnly, Category = Door, meta = (ClampMax = 10.0, ClampMin = 1.0))
	float OpeningSpeed;

	float _TargetAngle;
	float _LastOpeningAngle;
	
public:	
	// Sets default values for this actor's properties
	ADoor();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	void Tick(float DeltaSeconds) override;

	/* IInteractable interface */
	virtual void OnBeginInteract_Implementation(APawn* InteractingPawn) override;
	virtual UUserWidget* OnBeginLineTraceOver_Implementation(APawn* Pawn) override;
	virtual void OnEndLineTraceOver_Implementation(APawn* Pawn) override;
	/* IInteractable interface end */	
};
