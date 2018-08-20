// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interfaces/Interactable.h"
#include "Door.generated.h"


class UStaticMeshComponent;
class UArrowComponent;


UCLASS()
class COOPARENA_API ADoor : public AActor, public IInteractable
{
	GENERATED_BODY()


protected:
	/*UPROPERTY(VisibleDefaultsOnly, Category = Door)
	UStaticMeshComponent* DoorFrame;*/

	UPROPERTY(VisibleDefaultsOnly, Category = Door)
	UStaticMeshComponent* Door;

	/* If true, the door will open to both sides (away from the interacting pawn). Otherwise only to the front. */
	UPROPERTY(EditAnywhere, Category = Door)
	bool bTwoSidedOpening;

	/* The angle that the opened door will have. A negative values mean that the door will open to the back. */
	UPROPERTY(EditAnywhere, Category = Door, meta = (ClampMax = 135.0, ClampMin = -135.0f))
	float OpeningAngle;

	/* How fast the door will open */
	UPROPERTY(EditAnywhere, Category = Door, meta = (ClampMax = 10.0, ClampMin = 1.0))
	float OpeningSpeed;
	
	UPROPERTY(BlueprintReadOnly, Category = Door)
	UArrowComponent* Front;

	UPROPERTY(BlueprintReadOnly, Category = Door)
	bool bIsOpen;

	/* The angle to which the door will move, when being interacted with. Can be 0 or the opening angle. */
	UPROPERTY(BlueprintReadOnly, Category = Door)
	float _TargetAngle;
	
public:	
	// Sets default values for this actor's properties
	ADoor();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	void Tick(float DeltaSeconds) override;

	/* IInteractable interface */
	virtual void OnBeginInteract_Implementation(APawn* InteractingPawn, UPrimitiveComponent* HitComponent) override;
	virtual UUserWidget* OnBeginLineTraceOver_Implementation(APawn* Pawn, UPrimitiveComponent* HitComponent) override;
	virtual void OnEndLineTraceOver_Implementation(APawn* Pawn) override;
	/* IInteractable interface end */	
};
