// Fill out your copyright notice in the Description page of Project Settings.

#include "Weapons/Weapon.h"
#include "Humanoid.h"
#include "Components/SkeletalMeshComponent.h"


AWeapon::AWeapon()
{

}


/////////////////////////////////////////////////////
void AWeapon::Equip(AHumanoid* NewOwner)
{

}


void AWeapon::UnEquip()
{

}


/////////////////////////////////////////////////////
void AWeapon::Use()
{

}


void AWeapon::StopUsing()
{

}


/////////////////////////////////////////////////////
float AWeapon::GetCooldown() const
{
	return Cooldown;
}

bool AWeapon::CanShoot() const
{
	return true;
}

/////////////////////////////////////////////////////
void AWeapon::SetOwningPawn(AHumanoid* NewOwner)
{
	if (GetOwner() != NewOwner)
	{
		Instigator = NewOwner;
		SetOwner(NewOwner);
		MyPawn = NewOwner;
	}
}


USkeletalMeshComponent* AWeapon::GetWeaponMesh() const
{
	return Mesh;
}


/////////////////////////////////////////////////////
void AWeapon::AttachMeshToPawn()
{
	if (MyPawn)
	{
		DetachMeshFromPawn();

		FName AttachPoint = MyPawn->GetWeaponAttachPoint();
		USkeletalMeshComponent* PawnMesh = MyPawn->GetMesh();
		
		if (Mesh)
		{
			Mesh->AttachToComponent(PawnMesh, FAttachmentTransformRules::SnapToTargetNotIncludingScale, AttachPoint);
		}
	}
}


void AWeapon::DetachMeshFromPawn()
{
	if (Mesh)
	{
		Mesh->DetachFromComponent(FDetachmentTransformRules::KeepWorldTransform);
	}
}
