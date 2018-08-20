// Fill out your copyright notice in the Description page of Project Settings.

#include "HealthComponent.h"
#include "GameFramework/Actor.h"
#include "Kismet/GameplayStatics.h"


// Sets default values for this component's properties
UHealthComponent::UHealthComponent()
{
	MaxHealth = 100.0f;
	CurrentHealth = MaxHealth;
}


// Called when the game starts
void UHealthComponent::BeginPlay()
{
	Super::BeginPlay();

	GetOwner()->OnTakePointDamage.AddDynamic(this, &UHealthComponent::HandlePointDamage);
}


void UHealthComponent::HandlePointDamage(AActor* DamagedActor, float Damage, class AController* InstigatedBy, FVector HitLocation, class UPrimitiveComponent* FHitComponent, FName BoneName, FVector ShotFromDirection, const class UDamageType* DamageType, AActor* DamageCauser)
{
	UE_LOG(LogTemp, Warning, TEXT("%s recieved %f point damage from %s."), *GetOwner()->GetName(), Damage, *InstigatedBy->GetName());
}