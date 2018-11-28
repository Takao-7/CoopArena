// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interfaces/Interactable.h"
#include "Door.generated.h"


class UStaticMeshComponent;
class UArrowComponent;
class UBoxComponent;


UCLASS()
class COOPARENA_API ADoor : public AActor, public IInteractable
{
	GENERATED_BODY()

protected:
	/* If true, the door will open to both sides (away from the interacting pawn). Otherwise only to the front. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Door")
	bool _bTwoSidedOpening;

	/* The angle that the opened door will have. A negative values mean that the door will open to the back. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = _DoorMesh, meta = (ClampMax = 135.0, ClampMin = -135.0f))
	float _OpeningAngle;

	UPROPERTY(VisibleAnywhere, Category = "Door")
	UArrowComponent* _Front;

	UPROPERTY(VisibleAnywhere, Category = "Door")
	UStaticMeshComponent* _DoorMesh;

	UPROPERTY(VisibleAnywhere, Category = "Door")
	UBoxComponent* _InteractionBox;

private:
	/* How fast the door will open */
	UPROPERTY(EditAnywhere, Category = "Door", meta = (ClampMax = 10.0, ClampMin = 1.0, DisplayName = "Opening speed"))
	float _OpeningSpeed;
	
	/* The angle to which the door will move, when being interacted with. Can be 0 or the opening angle. */
	float _TargetAngle;

	bool _bIsOpen;

protected:
	virtual void BeginPlay() override;

public:	
	ADoor();

	void Tick(float DeltaSeconds) override;


	/////////////////////////////////////////////////////
			/* IInteractable interface */
	/////////////////////////////////////////////////////
public:
	virtual void OnBeginInteract_Implementation(APawn* InteractingPawn, UPrimitiveComponent* HitComponent) override;
	virtual UUserWidget* OnBeginLineTraceOver_Implementation(APawn* Pawn, UPrimitiveComponent* HitComponent) override;
	virtual void OnEndLineTraceOver_Implementation(APawn* Pawn) override;


	/////////////////////////////////////////////////////
				/* Networking */
	/////////////////////////////////////////////////////
private:
	UFUNCTION(Server, WithValidation, Reliable)
	void HandleInteract_Server(APawn* InteractingPawn, UPrimitiveComponent* HitComponent);
	void HandleInteract_Server_Implementation(APawn* InteractingPawn, UPrimitiveComponent* HitComponent);
	bool HandleInteract_Server_Validate(APawn* InteractingPawn, UPrimitiveComponent* HitComponent);

	/* Function to enable the tick function, in order to open or close the door. The opening angle is being replicated and set in 'HandleInteract_Server'. */
	UFUNCTION(NetMulticast, Reliable)
	void EnableTickFunction_Multicast(float TargetAngle, bool bIsOpen);
	void EnableTickFunction_Multicast_Implementation(float TargetAngle, bool bIsOpen);
};
