// Fill out your copyright notice in the Description page of Project Settings.

#include "Magazine.h"
#include "Weapons/Projectile.h"
#include "Components/StaticMeshComponent.h"
#include "Components/BoxComponent.h"
#include "CoopArena.h"
<<<<<<< HEAD
=======
#include "UnrealNetwork.h"
>>>>>>> 17f86cef60dd7dd576fc030497f09716282c8ed8


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
}

<<<<<<< HEAD

AMagazine::AMagazine()
{
	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	_Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	_Mesh->SetupAttachment(RootComponent);

	_InteractionVolume = CreateDefaultSubobject<UBoxComponent>(TEXT("InteractionBox"));
	SetUpInteractionVolume();
}


=======
/////////////////////////////////////////////////////
>>>>>>> 17f86cef60dd7dd576fc030497f09716282c8ed8
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

<<<<<<< HEAD

UMeshComponent* AMagazine::GetMesh() const
{
	return _Mesh;
=======
/////////////////////////////////////////////////////
UMeshComponent* AMagazine::GetMesh() const
{
	return _Mesh;
}

/////////////////////////////////////////////////////
void AMagazine::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AMagazine, _RoundsLeft);
>>>>>>> 17f86cef60dd7dd576fc030497f09716282c8ed8
}