// Fill out your copyright notice in the Description page of Project Settings.

#include "Magazine.h"
#include "Weapons/Projectile.h"


// Sets default values
AMagazine::AMagazine()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

}

// Called when the game starts or when spawned
void AMagazine::BeginPlay()
{
	Super::BeginPlay();
	
}


int32 AMagazine::RoundsLeft()
{
	return m_Capacity;
}


TSubclassOf<AProjectile> AMagazine::GetProjectileClass(bool reduceCapacity /*= true*/)
{
	if (m_Capacity == 0)
	{
		return nullptr;
	}

	if (reduceCapacity && m_Capacity != -1)
	{
		m_Capacity--;
	}
	return m_ProjectileType;
}

