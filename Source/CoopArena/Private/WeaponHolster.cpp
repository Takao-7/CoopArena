// Fill out your copyright notice in the Description page of Project Settings.

#include "WeaponHolster.h"
#include "Gun.h"
#include "Animation/AnimInstance.h"
#include "Humanoid.h"
#include "Components/MeshComponent.h"
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
	_NewGun = gunToAdd;
	PlayEquipAnimation();
	return true;
}


void UWeaponHolster::PlayEquipAnimation()
{
	UAnimInstance* AnimInstance = _Owner->GetMesh()->GetAnimInstance();
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
	outItemStats = _CarriedGun->GetItemStats();
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


void UWeaponHolster::OnBeginInteract_Implementation(APawn* InteractingPawn)
{

}

void UWeaponHolster::OnEndInteract_Implementation(APawn* InteractingPawn)
{

}

UUserWidget* UWeaponHolster::OnBeginLineTraceOver_Implementation(APawn* Pawn)
{
	return nullptr;
}

void UWeaponHolster::OnEndLineTraceOver_Implementation(APawn* Pawn)
{

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
	bIsWhiteList = true;
	_WeaponTypesToCarry.Add(EWEaponType::Rifle);
	_WeaponTypesToCarry.Add(EWEaponType::Shotgun);
	_WeaponTypesToCarry.Add(EWEaponType::Launcher);
}


void UWeaponHolster::SwapWeapon()
{
	if (_NewGun)
	{
		AttachGunToHolster();
	}
	else
	{
		DetachGunFromHolster();
	}
}


void UWeaponHolster::AttachGunToHolster()
{	
	_CarriedGun = _NewGun;
	_NewGun = nullptr;
	_CarriedGun->AttachToComponent(_Owner->GetMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, _AttachPoint);
	ToggleIsInUse();
}


void UWeaponHolster::DetachGunFromHolster()
{
	_CarriedGun->OnEquip(_Owner);
	_CarriedGun = nullptr;
	ToggleIsInUse();
}


void UWeaponHolster::BeginPlay()
{
	Super::BeginPlay();

	_Owner = Cast<AHumanoid>(GetOwner());
	_Owner->EquipWeapon_Event.AddDynamic(this, &UWeaponHolster::HandleEquipWeaponEvent);
}


void UWeaponHolster::HandleEquipWeaponEvent()
{
	AGun* OwnersGun = _Owner->GetEquippedGun();
	if (OwnersGun)
	{
		if ((!_WeaponTypesToCarry.Contains(OwnersGun->GetWeaponType()) && bIsWhiteList) || (_WeaponTypesToCarry.Contains(OwnersGun->GetWeaponType()) && !bIsWhiteList))
		{
			return;
		}
	}

	if (OwnersGun && _CarriedGun)
	{
		OwnersGun->OnUnequip(true);
		FItemStats outStats;
		RemoveItem_Implementation(0, outStats);
	}
	else if (OwnersGun && !_CarriedGun)
	{
		AddItem_Implementation(OwnersGun);
	}
	else if (_CarriedGun)
	{
		FItemStats outStats;
		RemoveItem_Implementation(0, outStats);
	}
}