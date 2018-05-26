// Fill out your copyright notice in the Description page of Project Settings.

#include "WeaponHolster.h"
#include "Gun.h"
#include "Animation/AnimInstance.h"
#include "PlayerCharacter.h"
#include "Engine/World.h"
#include "Components/ArrowComponent.h"


bool UWeaponHolster::AddItem_Implementation(class AItemBase* itemToAdd)
{
	AGun* gunToAdd = Cast<AGun>(itemToAdd);
	if (gunToAdd == nullptr || _CarriedGun)
	{
		return false;
	}
	ToggleIsInUse();
	gunToAdd->OnUnequip(false);
	_CarriedGun = gunToAdd;

	PlayEquipAnimation();

	return true;
}


void UWeaponHolster::PlayEquipAnimation()
{
	APlayerCharacter* owner = Cast<APlayerCharacter>(GetOwner());
	UAnimInstance* AnimInstance = owner->GetMesh()->GetAnimInstance();
	if (AnimInstance)
	{
		AnimInstance->Montage_Play(_HolsteringAnimations, 1.0f);
	}
}


bool UWeaponHolster::RemoveItem_Implementation(int32 itemIndexToRemove, FItemStats& outItemStats)
{
	if (!_CarriedGun)
	{
		return false;
	}	
	ToggleIsInUse();
	PlayEquipAnimation();	
	return true;
}


bool UWeaponHolster::HasSpaceForItem_Implementation(FItemStats& item) const
{
	return !_CarriedGun;
}


int32 UWeaponHolster::GetInventorySize_Implementation() const
{
	return 1;
}


bool UWeaponHolster::WeaponCanReloadFrom_Implementation() const
{
	return false;
}


void UWeaponHolster::ToggleIsInUse()
{
	if (bIsInUse)
	{
		bIsInUse = false;
	}
	else
	{
		bIsInUse = true;
	}
}


UWeaponHolster::UWeaponHolster()
{
	bIsInUse = false;
}


void UWeaponHolster::OnAttachAndDetach()
{
	if (_CarriedGun)
	{
		DetachGunFromHolster();
	}
	else
	{
		AttachGunToHolster();
	}
}


void UWeaponHolster::AttachGunToHolster()
{	
	ACharacter* owner = Cast<ACharacter>(GetOwner());	
	_CarriedGun->AttachToComponent(owner->GetMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, _AttachPoint);
	ToggleIsInUse();
}


void UWeaponHolster::DetachGunFromHolster()
{
	AHumanoid* owner = Cast<AHumanoid>(GetOwner());
	_CarriedGun->OnEquip(owner);
	_CarriedGun = nullptr;
	ToggleIsInUse();
}
