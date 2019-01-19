// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interfaces/Interactable.h"
#include "Structs/ItemStructs.h"
#include "PickUp.generated.h"


class UStaticMeshComponent;
class UBoxComponent;


UCLASS()
class COOPARENA_API APickUp : public AActor, public IInteractable
{
	GENERATED_BODY()

private:
	bool _bCanBeInteractedWith;
	float _Yaw;
	float _Roll;
	float _Pitch;
	FRotator _RotationDelta;

	UStaticMeshComponent* _Mesh;

	/* The magazine stack that this pickup represents */
	FMagazineStack _MagazineStack;

protected:
	/* The magazine type that this pick up represents. */
	UPROPERTY(EditDefaultsOnly, Category = "Pickup")
	TSubclassOf<AMagazine> _MagazineType;

	UPROPERTY(EditDefaultsOnly, Category = "Pickup", meta = (ClampMin = 0))
	int32 _StackSize;

	/* How long, in seconds, this pick up exists. Set to 0 for infinite. */
	UPROPERTY(EditDefaultsOnly, Category = "Pickup", meta = (ClampMin = 0.0f))
	float _LiveSpan;

	/* How fast the mesh is rotating */
	UPROPERTY(EditDefaultsOnly, Category = "Pickup")
	float _RotationSpeed;

	/* The widget that will be displayed if a player looks at us */
	UPROPERTY(VisibleDefaultsOnly, Category = "Pickup")
	UUserWidget* _Widget;

	/* This pickup's volume where players can interact with it */
	UPROPERTY(VisibleDefaultsOnly, Category = "Pickup")
	UBoxComponent* _InteractionBox;

	/* This pickup's collision */
	UPROPERTY(VisibleDefaultsOnly, Category = "Pickup")
	UBoxComponent* _Collision;

	virtual void BeginPlay() override;
	
public:
	APickUp();

	virtual void Tick(float DeltaSeconds) override;

	void SetMagazineStack(TSubclassOf<AMagazine> MagClass, int32 NumMags);


	/////////////////////////////////////////////////////
			/* Interactable interface */
	/////////////////////////////////////////////////////
public:
	virtual void OnBeginInteract_Implementation(APawn* InteractingPawn, UPrimitiveComponent* HitComponent) override;
	//virtual void OnEndInteract_Implementation(APawn* InteractingPawn) override;
	virtual UUserWidget* OnBeginLineTraceOver_Implementation(APawn* Pawn, UPrimitiveComponent* HitComponent) override;
	virtual void OnEndLineTraceOver_Implementation(APawn* Pawn) override;
	virtual void SetCanBeInteractedWith_Implementation(bool bCanbeInteractedWith) override;
};