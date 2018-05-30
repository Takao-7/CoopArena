// Fill out your copyright notice in the Description page of Project Settings.

#include "Magazine.h"
#include "Weapons/Projectile.h"


// Called when the game starts or when spawned
void AMagazine::BeginPlay()
{
	Super::BeginPlay();	
	_RoundsLeft = _Capacity;
}


int32 AMagazine::RoundsLeft() const
{
	return _RoundsLeft;
}


int32 AMagazine::GetCapacity() const
{
	return _Capacity;
}


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


TSubclassOf<AProjectile> AMagazine::GetProjectileClass() const
{
	return _ProjectileType;
}

