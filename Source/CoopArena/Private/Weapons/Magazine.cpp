// Fill out your copyright notice in the Description page of Project Settings.

#include "Magazine.h"
#include "Weapons/Projectile.h"
#include "Components/StaticMeshComponent.h"
#include "Components/BoxComponent.h"
#include "CoopArena.h"
#include "UnrealNetwork.h"
#include "Engine/World.h"
#include "TimerManager.h"


AMagazine::AMagazine()
{
	_Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	RootComponent = _Mesh;

	_InteractionVolume = CreateDefaultSubobject<UBoxComponent>(TEXT("InteractionBox"));
	SetUpInteractionVolume();
}

/////////////////////////////////////////////////////
void AMagazine::BeginPlay()
{
	Super::BeginPlay();	
	_RoundsLeft = _Capacity;

	if (HasAuthority())
	{
		OnDestroyed.AddDynamic(this, &AMagazine::HandleOnDestroyed);
	}
}

/////////////////////////////////////////////////////
void AMagazine::HandleOnDestroyed(AActor* DestroyedActor)
{
	GetWorld()->GetTimerManager().ClearAllTimersForObject(DestroyedActor);
}

/////////////////////////////////////////////////////
int32 AMagazine::RoundsLeft() const
{
	return _RoundsLeft;
}

/////////////////////////////////////////////////////
int32 AMagazine::GetCapacity() const
{
	return _Capacity;
}

/////////////////////////////////////////////////////
bool AMagazine::RemoveRound(int32 numRounds /*= 1*/)
{
	if (_RoundsLeft - numRounds < 0 && _Capacity != -1)
	{
		return false;
	}
	else if (_Capacity != -1)
	{
		_RoundsLeft -= numRounds;
	}
	return true;
}

/////////////////////////////////////////////////////
TSubclassOf<AProjectile> AMagazine::GetProjectileClass() const
{
	return _ProjectileType;
}

/////////////////////////////////////////////////////
UMeshComponent* AMagazine::GetMesh() const
{
	return _Mesh;
}

/////////////////////////////////////////////////////
bool AMagazine::IsFull() const
{
	return GetCapacity() == RoundsLeft();
}

/////////////////////////////////////////////////////
void AMagazine::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AMagazine, _RoundsLeft);
}