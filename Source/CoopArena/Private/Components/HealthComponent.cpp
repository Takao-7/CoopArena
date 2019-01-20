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
#include "Engine/GameInstance.h"
#include "Engine/Engine.h"


UHealthComponent::UHealthComponent()
{
	_MaxHealth = 100.0f;
	
	bReplicates = true;
	bAutoActivate = true;
}

/////////////////////////////////////////////////////
bool UHealthComponent::IsAlive() const
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
	HandleDeath(DeadActor, Controller, Killer);
	OnDeath.Broadcast(GetOwner(), Controller, Killer);

	APlayerCharacter* player = Cast<APlayerCharacter>(DeadActor);
	if (player)
	{
		player->StopRunningSound();
	}
}

/////////////////////////////////////////////////////
void UHealthComponent::BeginPlay()
{
	Super::BeginPlay();

	AHumanoid* owner = Cast<AHumanoid>(GetOwner());
	ensureMsgf(owner, TEXT("The owner is not a humanoid."));

	GetOwner()->OnTakePointDamage.AddDynamic(this, &UHealthComponent::HandlePointDamage);

	_CurrentHealth = _MaxHealth;
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
		bIsPlayer ? gameMode->PlayerDeath_Event.Broadcast(Cast<APlayerCharacter>(Owner), Killer) : gameMode->BotDeath_Event.Broadcast(Owner, Killer);

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
	GetWorld()->GetTimerManager().SetTimer(handle, delegate, 0.2f, false);
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
void UHealthComponent::SetHealth(float NewHealth)
{
	_CurrentHealth = FMath::Clamp(NewHealth, 0.0f, _MaxHealth);
}

/////////////////////////////////////////////////////
void UHealthComponent::HandlePointDamage(AActor* DamagedActor, float Damage, class AController* InstigatedBy, FVector HitLocation, class UPrimitiveComponent* FHitComponent, 
										FName BoneName, FVector ShotFromDirection, const class UDamageType* DamageType, AActor* DamageCauser)
{
	if (_MaxHealth == -1.0f)
	{
		return;
	}
	_CurrentHealth -= Damage;

	//FString string = GetName() + TEXT(": Recieved ") + FString::SanitizeFloat(Damage) + TEXT(" damage. ") + FString::SanitizeFloat(_CurrentHealth) + TEXT(" health remaining.");
	//GetWorld()->GetGameInstance()->GetEngine()->AddOnScreenDebugMessage(0, 5.0f, FColor::Green, string);

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