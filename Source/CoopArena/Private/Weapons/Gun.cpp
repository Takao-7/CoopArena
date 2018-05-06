// Fill out your copyright notice in the Description page of Project Settings.

#include "Weapons/Gun.h"
#include "Humanoid.h"
#include "GameFramework/PlayerController.h"
#include "Components/SkeletalMeshComponent.h"

AGun::AGun()
{
	FireModes.Add(EFireMode::Single);

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


bool AGun::CanShoot() const
{
	bool bCanFire = MyOwner && MyOwner->CanFire();
	bool bStateOKToFire = ((CurrentState == EWeaponState::Idle) || (CurrentState == EWeaponState::Firing));
	return ((bCanFire == true) && (bStateOKToFire == true));
}


/////////////////////////////////////////////////////
void AGun::OnPickUp(AActor* NewOwner)
{
	SetOwningPawn(NewOwner);
	AttachMeshToPawn();
}


void AGun::OnDrop()
{
	SetOwningPawn(nullptr);
	DetachMeshFromPawn();
}


/////////////////////////////////////////////////////
UUserWidget* AGun::OnBeginTraceCastOver(APawn* TracingPawn)
{
	if (!TracingPawn || TracingPawn->IsLocallyControlled())
	{
		return nullptr;
	}
	
	return InfoWidget;
}


/////////////////////////////////////////////////////
void AGun::SetOwningPawn(AActor* NewOwner)
{
	AHumanoid* humanoid = Cast<AHumanoid>(NewOwner);
	if (humanoid && GetOwner() != NewOwner)
	{
		Instigator = humanoid;
		SetOwner(humanoid);
		MyOwner = humanoid;
	}
}


/////////////////////////////////////////////////////
void AGun::AttachMeshToPawn()
{
	if (MyOwner)
	{
		DetachMeshFromPawn();

		FName AttachPoint = MyOwner->GetWeaponAttachPoint();
		USkeletalMeshComponent* PawnMesh = MyOwner->GetMesh();

		if (Mesh)
		{
			Mesh->AttachToComponent(PawnMesh, FAttachmentTransformRules::SnapToTargetNotIncludingScale, AttachPoint);
			Mesh->SetSimulatePhysics(false);
		}
	}
}


void AGun::DetachMeshFromPawn()
{
	if (Mesh)
	{
		Mesh->DetachFromComponent(FDetachmentTransformRules::KeepWorldTransform);
		Mesh->SetSimulatePhysics(true);
	}
}


/////////////////////////////////////////////////////
float AGun::GetCooldownTime() const
{
	return Cooldown;
}


float AGun::GetRoundsPerMinute() const
{
	return 60 / GetCooldownTime();
}


FVector AGun::GetMuzzleLocation() const
{
	FVector VecMuzzleLocation;
	FRotator MuzzleRotation;

	if (Mesh)
	{
		Mesh->GetSocketWorldLocationAndRotation(MuzzleAttachPoint, VecMuzzleLocation, MuzzleRotation);
	}
	return VecMuzzleLocation;
}