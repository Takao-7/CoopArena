// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interactable.h"
#include "ItemBase.generated.h"


class UMeshComponent;
class UPaperSpriteComponent;

UCLASS(abstract)
class COOPARENA_API AItemBase : public AActor, public IInteractable
{
	GENERATED_BODY()

protected:
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = ItemBase)
	UPaperSpriteComponent* _InventoryIcon;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	AItemBase();

	// Called every frame
	virtual void Tick(float DeltaTime) override;
};