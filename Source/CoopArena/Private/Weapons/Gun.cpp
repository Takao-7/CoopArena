// Fill out your copyright notice in the Description page of Project Settings.

#include "Weapons/Gun.h"
#include "Humanoid.h"
#include "Components/SkeletalMeshComponent.h"

AGun::AGun()
{
	FireModes.SetNumUninitialized(3, false);

	Mesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Weapon Mesh"));
	Mesh->SetCollisionObjectType(ECC_WorldDynamic);
	Mesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	Mesh->SetCollisionResponseToAllChannels(ECR_Ignore);
	Mesh->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);
	Mesh->CastShadow = true;
	SetRootComponent(Mesh);

	CurrentState = EWeaponState::Idle;
}


FRotator AGun::AdjustAimRotation(FVector startLocation, FVector direction)
{
	return FRotator::ZeroRotator;
}


void AGun::OnFire()
{

}

void AGun::ContinousOnFire()
{

}

float AGun::GetRoundsPerMinute() const
{
	return 0.0f;
}


FVector AGun::GetMuzzleLocation() const
{
	FVector VecMuzzleLocation;
	FRotator MuzzleRotation;

	USkeletalMeshComponent* WeaponMesh = GetWeaponMesh();
	WeaponMesh->GetSocketWorldLocationAndRotation(MuzzleAttachPoint, VecMuzzleLocation, MuzzleRotation);

	return VecMuzzleLocation;
}


bool AGun::CanShoot() const
{
	bool bCanFire = MyPawn && MyPawn->CanFire();
	bool bStateOKToFire = ((CurrentState == EWeaponState::Idle) || (CurrentState == EWeaponState::Firing));
	return ((bCanFire == true) && (bStateOKToFire == true));
}
