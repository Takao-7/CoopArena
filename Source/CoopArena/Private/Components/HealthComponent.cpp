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
#include "GameModes/CoopArenaGameMode.h"
#include "PlayerCharacter.h"


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
void UHealthComponent::Kill(AController* Killer)
{
	if (GetOwner()->HasAuthority() && !bAlreadyDied)
	{
		_CurrentHealth = 0.0f;
		OnDeathEvent_Multicast(GetOwner(), GetOwner()->GetInstigatorController(), Killer);
	}	
}

/////////////////////////////////////////////////////
void UHealthComponent::OnDeathEvent_Multicast_Implementation(AActor* DeadActor, AController* Controller, AController* Killer)
{
	OnDeath.Broadcast(GetOwner(), Controller, Killer);
}

/////////////////////////////////////////////////////
void UHealthComponent::BeginPlay()
{
	Super::BeginPlay();

	AHumanoid* owner = Cast<AHumanoid>(GetOwner());
	ensureMsgf(owner, TEXT("The owner is not a humanoid."));

	GetOwner()->OnTakePointDamage.AddDynamic(this, &UHealthComponent::HandlePointDamage);
	OnDeath.AddDynamic(this, &UHealthComponent::HandleDeath);
}

/////////////////////////////////////////////////////
void UHealthComponent::HandleDeath(AActor* Owner, AController* Controller, AController* Killer)
{
	if (!bAlreadyDied && GetOwner()->HasAuthority())
	{
		bAlreadyDied = true;

		ACoopArenaGameMode* gameMode = GetWorld()->GetAuthGameMode<ACoopArenaGameMode>();
		ensureMsgf(gameMode, TEXT("The game mode is not a subclass of ACoopArenaGameMode"));
		
		const bool bIsPlayer = GetOwner()->GetInstigatorController() && GetOwner()->GetInstigatorController()->IsPlayerController();
		bIsPlayer ? gameMode->OnPlayerDeath_Event.Broadcast(Cast<APlayerCharacter>(Owner), Killer) : gameMode->OnBotDeath_Event.Broadcast(Owner, Killer);

		HandleDeath_Multicast();
	}
}

/////////////////////////////////////////////////////
void UHealthComponent::HandleDeath_Multicast_Implementation()
{
	bAlreadyDied = true;

	DeactivateCollisionCapsuleComponent();
	SetPhysicsOnMesh();
	
	AHumanoid* owner = Cast<AHumanoid>(GetOwner());
	owner->SetComponentIsBlockingFiring(true, this);
	if (owner->IsLocallyControlled())
	{
		owner->DisableInput(nullptr);
	}

	FTimerDelegate delegate;
	delegate.BindLambda([this, owner]
	{
		if (owner->GetEquippedGun())
		{
			owner->UnequipWeapon(true, false);
		}
	});
	FTimerHandle handle;
	GetWorld()->GetTimerManager().SetTimer(handle, delegate, 0.5f, false);
}

/////////////////////////////////////////////////////
void UHealthComponent::SetPhysicsOnMesh()
{
	AHumanoid* owner = Cast<AHumanoid>(GetOwner());
	owner->GetMesh()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	owner->GetMesh()->SetSimulatePhysics(true);
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
	UCapsuleComponent* capsule = Cast<AHumanoid>(GetOwner())->GetCapsuleComponent();
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
		OnDeathEvent_Multicast(GetOwner(), GetOwner()->GetInstigatorController(), InstigatedBy);
	}
}

/////////////////////////////////////////////////////
void UHealthComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UHealthComponent, _CurrentHealth);
}