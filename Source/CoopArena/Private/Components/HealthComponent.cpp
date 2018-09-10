// Fill out your copyright notice in the Description page of Project Settings.

#include "HealthComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Humanoid.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Gun.h"
<<<<<<< HEAD
=======
#include "UnrealNetwork.h"
>>>>>>> 17f86cef60dd7dd576fc030497f09716282c8ed8


// Sets default values for this component's properties
UHealthComponent::UHealthComponent()
{
<<<<<<< HEAD
	MaxHealth = 100.0f;
	CurrentHealth = MaxHealth;
}


=======
	_MaxHealth = 100.0f;
	_CurrentHealth = _MaxHealth;
	
	bReplicates = true;
}

/////////////////////////////////////////////////////
>>>>>>> 17f86cef60dd7dd576fc030497f09716282c8ed8
FORCEINLINE bool UHealthComponent::IsAlive() const
{
	return !bAlreadyDied;
}

<<<<<<< HEAD

=======
/////////////////////////////////////////////////////
>>>>>>> 17f86cef60dd7dd576fc030497f09716282c8ed8
void UHealthComponent::Kill()
{
	OnDeath();
}

<<<<<<< HEAD

// Called when the game starts
=======
/////////////////////////////////////////////////////
>>>>>>> 17f86cef60dd7dd576fc030497f09716282c8ed8
void UHealthComponent::BeginPlay()
{
	Super::BeginPlay();

	GetOwner()->OnTakePointDamage.AddDynamic(this, &UHealthComponent::HandlePointDamage);
	_compOwner = GetOwnerAsHumanoid();
}

<<<<<<< HEAD

void UHealthComponent::OnDeath()
{
	if (bAlreadyDied)
	{
		return;
	}
=======
/////////////////////////////////////////////////////
void UHealthComponent::OnDeath()
{
	if (!bAlreadyDied && GetOwner()->HasAuthority())
	{
		bAlreadyDied = true;
		Multicast_OnDeath();
	}
}

/////////////////////////////////////////////////////
void UHealthComponent::Multicast_OnDeath_Implementation()
{
>>>>>>> 17f86cef60dd7dd576fc030497f09716282c8ed8
	bAlreadyDied = true;

	DeactivateCollisionCapsuleComponent();
	SetPhysicsOnMesh();
<<<<<<< HEAD
	_compOwner->SetComponentIsBlockingFiring(true, this);

=======
	_compOwner->Set_ComponentIsBlockingFiring(true, this);

	if (_compOwner->IsLocallyControlled())
	{
		_compOwner->DisableInput(nullptr);
	}

	_compOwner->GetEquippedGun()->SetActorEnableCollision(true);
	
>>>>>>> 17f86cef60dd7dd576fc030497f09716282c8ed8
	FTimerDelegate delegate;
	delegate.BindLambda([this]
	{
		if (_compOwner->GetEquippedGun())
		{
			_compOwner->GetEquippedGun()->OnUnequip(true);
		}
	});
	FTimerHandle handle;
	GetWorld()->GetTimerManager().SetTimer(handle, delegate, 0.5f, false);
<<<<<<< HEAD
}


=======

}

/////////////////////////////////////////////////////
>>>>>>> 17f86cef60dd7dd576fc030497f09716282c8ed8
void UHealthComponent::SetPhysicsOnMesh()
{
	_compOwner->GetMesh()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	_compOwner->GetMesh()->SetSimulatePhysics(true);
}

<<<<<<< HEAD

=======
/////////////////////////////////////////////////////
>>>>>>> 17f86cef60dd7dd576fc030497f09716282c8ed8
AHumanoid* UHealthComponent::GetOwnerAsHumanoid()
{
	AHumanoid* compOwner = Cast<AHumanoid>(GetOwner());
	if (compOwner == nullptr)
	{
		UE_LOG(LogTemp, Fatal, TEXT("%s, owner of %s is not a humanoid!"), *GetOwner()->GetName(), *GetName());
	}
	return compOwner;
}

<<<<<<< HEAD

=======
/////////////////////////////////////////////////////
>>>>>>> 17f86cef60dd7dd576fc030497f09716282c8ed8
void UHealthComponent::DeactivateCollisionCapsuleComponent()
{
	UCapsuleComponent* capsule = _compOwner->GetCapsuleComponent();
	capsule->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	capsule->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	capsule->SetGenerateOverlapEvents(false);
}

<<<<<<< HEAD

void UHealthComponent::HandlePointDamage(AActor* DamagedActor, float Damage, class AController* InstigatedBy, FVector HitLocation, class UPrimitiveComponent* FHitComponent, FName BoneName, FVector ShotFromDirection, const class UDamageType* DamageType, AActor* DamageCauser)
{
	CurrentHealth -= Damage;
	if (CurrentHealth <= 0.0f)
	{
		OnDeath();
	}
=======
/////////////////////////////////////////////////////
void UHealthComponent::HandlePointDamage(AActor* DamagedActor, float Damage, class AController* InstigatedBy, FVector HitLocation, class UPrimitiveComponent* FHitComponent, 
										FName BoneName, FVector ShotFromDirection, const class UDamageType* DamageType, AActor* DamageCauser)
{
	_CurrentHealth -= Damage;
	if (_CurrentHealth <= 0.0f)
	{
		OnDeath();
	}
}

/////////////////////////////////////////////////////
void UHealthComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UHealthComponent, _CurrentHealth);
>>>>>>> 17f86cef60dd7dd576fc030497f09716282c8ed8
}