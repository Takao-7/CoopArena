// Fill out your copyright notice in the Description page of Project Settings.

#include "InventoryComponent.h"
#include "Gameframework/Pawn.h"
#include "Engine/World.h"
#include "Components/InputComponent.h"
#include "GameFramework/PlayerController.h"
#include "Humanoid.h"
#include "Animation/AnimInstance.h"
#include "ItemBase.h"


void UInventoryComponent::BeginPlay()
{
	Super::BeginPlay();

	m_Owner = Cast<AHumanoid>(GetOwner());
	check(m_Owner);

	m_Owner->HolsterWeapon_Event.AddDynamic(this, &UInventoryComponent::OnOwnerHolsterWeapon);

	bReplicates = true;
	bAutoActivate = true;
}


void UInventoryComponent::OnOwnerHolsterWeapon(AGun* GunToHolster, int32 AttachPointIndex)
{
	bool bCanAttach = AttachPointIndex > -1 ? m_WeaponAttachPoints[AttachPointIndex].CanAttachWeapon(GunToHolster) : false;
	if(AttachPointIndex < 0 && !bCanAttach && GunToHolster)
	{
		for (int32 i = 0; i < m_WeaponAttachPoints.Num(); ++i)
		{
			bCanAttach = m_WeaponAttachPoints[i].CanAttachWeapon(GunToHolster);
			if (bCanAttach)
			{
				AttachPointIndex = i;
				break;
			}
		}
	}

	const int32 index = m_WeaponAttachPoints.Num() == 1 ? 0 : AttachPointIndex;
	
	if ((bCanAttach == false && GunToHolster) || m_WeaponAttachPoints.IsValidIndex(index) == false)
	{
		return;
	}

	FWeaponAttachPoint& attachPoint = m_WeaponAttachPoints[index];
	UAnimInstance* animInstance = m_Owner->GetMesh()->GetAnimInstance();

	if (GunToHolster)
	{
		if (animInstance && attachPoint.holsterAnimation)
		{
			animInstance->Montage_Play(attachPoint.holsterAnimation, 1.0f, EMontagePlayReturnType::MontageLength, 0.0f, false);
		}
		else
		{
			GunToHolster->OnUnequip(false); // We un-equip the gun in case we are currently holding it.
			attachPoint.AttachWeapon(GunToHolster, m_Owner->GetMesh());
		}
	}
	else
	{
		if (animInstance && attachPoint.holsterAnimation)
		{
			animInstance->Montage_Play(attachPoint.holsterAnimation, 1.0f, EMontagePlayReturnType::MontageLength, 0.0f, false);
		}
		else
		{
			AGun* gun = attachPoint.DetachWeapon();
			m_Owner->EquipWeapon(gun);
		}
	}
}


void UInventoryComponent::OnWeaponHolstering()
{
	const bool bAttachToHolster = m_Owner->GetEquippedGun() ? true : false;
	if (bAttachToHolster)
	{
		AGun* gun = m_Owner->GetEquippedGun();
		m_Owner->UnequipWeapon(false);
		m_WeaponAttachPoints[m_AttachPointIndex].AttachWeapon(gun, m_Owner->GetMesh());
	}
	else
	{
		AGun* gun = m_WeaponAttachPoints[m_AttachPointIndex].DetachWeapon();
		m_Owner->EquipWeapon(gun);
	}
}
