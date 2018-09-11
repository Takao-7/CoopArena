// Fill out your copyright notice in the Description page of Project Settings.

#include "InventoryComponent.h"
#include "Gameframework/Pawn.h"
#include "Engine/World.h"
#include "Components/InputComponent.h"
#include "GameFramework/PlayerController.h"
#include "Humanoid.h"
#include "Animation/AnimInstance.h"
#include "ItemBase.h"


// Called when the game starts
void UInventoryComponent::BeginPlay()
{
	Super::BeginPlay();

	_Owner = Cast<AHumanoid>(GetOwner());
	check(_Owner);

	_Owner->HolsterWeapon_Event.AddDynamic(this, &UInventoryComponent::OnOwnerWeaponHolster);

	bReplicates = true;
	bAutoActivate = true;
}


void UInventoryComponent::OnOwnerWeaponHolster(AGun* Gun)
{
	if (_WeaponAttachPoint.socket == "none")
	{
		return;
	}

	if (Gun)
	{
		// We want to holster our currently equipped weapon		
		bool bCanAttach = _WeaponAttachPoint.CanAttachWeapon(Gun);
		if (bCanAttach)
		{
			UAnimInstance* animInstance = _Owner->GetMesh()->GetAnimInstance();
			if (animInstance && _WeaponAttachPoint.holsterAnimation)
			{
				animInstance->Montage_Play(_WeaponAttachPoint.holsterAnimation);
			}
			else
			{
				Gun->OnUnequip(false);
				_WeaponAttachPoint.AttachWeapon(Gun, _Owner->GetMesh());
			}
		}		
	}
	else
	{
		// We want to equip our holstered weapon
		if (_WeaponAttachPoint.GetAttachedWeapon() == nullptr)
		{
			return;
		}

		UAnimInstance* animInstance = _Owner->GetMesh()->GetAnimInstance();
		if (animInstance && _WeaponAttachPoint.holsterAnimation)
		{
			animInstance->Montage_Play(_WeaponAttachPoint.holsterAnimation);
		}
		else
		{
			AGun* gun = _WeaponAttachPoint.DetachWeapon();
			gun->OnEquip(_Owner);
		}
	}
}


void UInventoryComponent::OnWeaponHolstering()
{
	bool bAttachToHolster;
	_Owner->GetEquippedGun() ? bAttachToHolster = true : bAttachToHolster = false;
	if (bAttachToHolster)
	{
		AGun* gun = _Owner->GetEquippedGun();

		gun->OnUnequip(false);
		_WeaponAttachPoint.AttachWeapon(gun, _Owner->GetMesh());
	}
	else
	{
		AGun* gun = _WeaponAttachPoint.DetachWeapon();
		gun->OnEquip(_Owner);
	}
}
