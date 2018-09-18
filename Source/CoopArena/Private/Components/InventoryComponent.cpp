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

	m_Owner = Cast<AHumanoid>(GetOwner());
	check(m_Owner);

	m_Owner->HolsterWeapon_Event.AddDynamic(this, &UInventoryComponent::OnOwnerWeaponHolster);

	bReplicates = true;
	bAutoActivate = true;
}


void UInventoryComponent::OnOwnerWeaponHolster(AGun* Gun)
{
	if (m_WeaponAttachPoint.socket == "none")
	{
		return;
	}

	if (Gun)
	{
		// We want to holster our currently equipped weapon		
		bool bCanAttach = m_WeaponAttachPoint.CanAttachWeapon(Gun);
		if (bCanAttach)
		{
			UAnimInstance* animInstance = m_Owner->GetMesh()->GetAnimInstance();
			if (animInstance && m_WeaponAttachPoint.holsterAnimation)
			{
				animInstance->Montage_Play(m_WeaponAttachPoint.holsterAnimation);
			}
			else
			{
				Gun->OnUnequip(false);
				m_WeaponAttachPoint.AttachWeapon(Gun, m_Owner->GetMesh());
			}
		}		
	}
	else
	{
		// We want to equip our holstered weapon
		if (m_WeaponAttachPoint.GetAttachedWeapon() == nullptr)
		{
			return;
		}

		UAnimInstance* animInstance = m_Owner->GetMesh()->GetAnimInstance();
		if (animInstance && m_WeaponAttachPoint.holsterAnimation)
		{
			animInstance->Montage_Play(m_WeaponAttachPoint.holsterAnimation);
		}
		else
		{
			AGun* gun = m_WeaponAttachPoint.DetachWeapon();
			gun->OnEquip(m_Owner);
		}
	}
}


void UInventoryComponent::OnWeaponHolstering()
{
	bool bAttachToHolster;
	m_Owner->GetEquippedGun() ? bAttachToHolster = true : bAttachToHolster = false;
	if (bAttachToHolster)
	{
		AGun* gun = m_Owner->GetEquippedGun();

		gun->OnUnequip(false);
		m_WeaponAttachPoint.AttachWeapon(gun, m_Owner->GetMesh());
	}
	else
	{
		AGun* gun = m_WeaponAttachPoint.DetachWeapon();
		gun->OnEquip(m_Owner);
	}
}
