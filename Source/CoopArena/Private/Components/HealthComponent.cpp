// Fill out your copyright notice in the Description page of Project Settings.

#include "HealthComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Humanoid.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Gun.h"
#include "UnrealNetwork.h"


UHealthComponent::UHealthComponent()
{
	_MaxHealth = 100.0f;
	_CurrentHealth = _MaxHealth;
	
	bReplicates = true;
	bAutoActivate = true;
}

/////////////////////////////////////////////////////
FORCEINLINE bool UHealthComponent::IsAlive() const
{
	return !bAlreadyDied;
}

/////////////////////////////////////////////////////
void UHealthComponent::Kill()
{
	if (GetOwner()->HasAuthority() && !bAlreadyDied)
	{
		_CurrentHealth = 0.0f;
		OnDeathEvent_Multicast();
	}	
}

/////////////////////////////////////////////////////
void UHealthComponent::OnDeathEvent_Multicast_Implementation()
{
	OnDeathEvent.Broadcast();
}

/////////////////////////////////////////////////////
void UHealthComponent::BeginPlay()
{
	Super::BeginPlay();

	GetOwner()->OnTakePointDamage.AddDynamic(this, &UHealthComponent::HandlePointDamage);
	_compOwner = GetOwnerAsHumanoid();

	OnDeathEvent.AddDynamic(this, &UHealthComponent::HandleDeath);
}

/////////////////////////////////////////////////////
void UHealthComponent::HandleDeath()
{
	if (!bAlreadyDied && GetOwner()->HasAuthority())
	{
		bAlreadyDied = true;

		Multicast_HandleDeath();
	}
}

/////////////////////////////////////////////////////
void UHealthComponent::Multicast_HandleDeath_Implementation()
{
	bAlreadyDied = true;

	DeactivateCollisionCapsuleComponent();
	SetPhysicsOnMesh();
	_compOwner->SetComponentIsBlockingFiring(true, this);

	if (_compOwner->IsLocallyControlled())
	{
		_compOwner->DisableInput(nullptr);
	}

	FTimerDelegate delegate;
	delegate.BindLambda([this]
	{
		if (_compOwner->GetEquippedGun())
		{
			_compOwner->GetEquippedGun()->SetActorEnableCollision(true);
			_compOwner->GetEquippedGun()->OnUnequip(true);
		}
	});
	FTimerHandle handle;
	GetWorld()->GetTimerManager().SetTimer(handle, delegate, 0.5f, false);
}

/////////////////////////////////////////////////////
void UHealthComponent::SetPhysicsOnMesh()
{
	_compOwner->GetMesh()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	_compOwner->GetMesh()->SetSimulatePhysics(true);
}

/////////////////////////////////////////////////////
AHumanoid* UHealthComponent::GetOwnerAsHumanoid()
{
	AHumanoid* compOwner = Cast<AHumanoid>(GetOwner());
	ensure(compOwner);

	return compOwner;
}

/////////////////////////////////////////////////////
void UHealthComponent::DeactivateCollisionCapsuleComponent()
{
	UCapsuleComponent* capsule = _compOwner->GetCapsuleComponent();
	capsule->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	capsule->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	capsule->SetGenerateOverlapEvents(false);
}

/////////////////////////////////////////////////////
void UHealthComponent::HandlePointDamage(AActor* DamagedActor, float Damage, class AController* InstigatedBy, FVector HitLocation, class UPrimitiveComponent* FHitComponent, 
										FName BoneName, FVector ShotFromDirection, const class UDamageType* DamageType, AActor* DamageCauser)
{
	_CurrentHealth -= Damage;
	if (_CurrentHealth <= 0.0f)
	{
		OnDeathEvent_Multicast();
	}
}

/////////////////////////////////////////////////////
void UHealthComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UHealthComponent, _CurrentHealth);
}