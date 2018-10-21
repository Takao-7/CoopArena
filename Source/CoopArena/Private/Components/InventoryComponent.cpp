// Fill out your copyright notice in the Description page of Project Settings.

#include "InventoryComponent.h"
#include "Gameframework/Pawn.h"
#include "Engine/World.h"
#include "Components/InputComponent.h"
#include "GameFramework/PlayerController.h"
#include "Humanoid.h"
#include "Animation/AnimInstance.h"
#include "ItemBase.h"


/////////////////////////////////////////////////////
void UInventoryComponent::BeginPlay()
{
	Super::BeginPlay();

	m_Owner = Cast<AHumanoid>(GetOwner());
	check(m_Owner);

	m_Owner->HolsterWeapon_Event.AddDynamic(this, &UInventoryComponent::OnOwnerHolsterWeapon);
}

/////////////////////////////////////////////////////
void UInventoryComponent::OnOwnerHolsterWeapon(AGun* GunToHolster, int32 AttachPointIndex)
{
	if (GetOwner()->HasAuthority() == false)
	{
		OnOwnerHolsterWeapon_Server(GunToHolster, AttachPointIndex);
		return;
	}

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

	UAnimMontage* holsterAnimation = m_WeaponAttachPoints[index].holsterAnimation;
	UAnimInstance* animInstance = m_Owner->GetMesh()->GetAnimInstance();

	if (GunToHolster)
	{
		if (animInstance && holsterAnimation)
		{
			PlayHolsteringAnimation_Multicast(holsterAnimation);
		}
		else
		{
			UnequipAndAttachWeapon_Multicast(index, GunToHolster);
		}
	}
	else
	{
		if (animInstance && holsterAnimation)
		{
			PlayHolsteringAnimation_Multicast(holsterAnimation);
		}
		else
		{
			DetachAndEquipWeapon_Multicast(index);
		}
	}
}

void UInventoryComponent::OnOwnerHolsterWeapon_Server_Implementation(AGun* GunToHolster, int32 AttachPointIndex)
{
	OnOwnerHolsterWeapon(GunToHolster, AttachPointIndex);
}

bool UInventoryComponent::OnOwnerHolsterWeapon_Server_Validate(AGun* GunToHolster, int32 AttachPointIndex)
{
	return true;
}

/////////////////////////////////////////////////////
void UInventoryComponent::OnWeaponHolstering()
{
	const bool bAttachToHolster = m_Owner->GetEquippedGun() ? true : false;
	if (bAttachToHolster)
	{
		AGun* gun = m_Owner->GetEquippedGun();
		m_Owner->UnequipWeapon(false, false);
		m_WeaponAttachPoints[m_AttachPointIndex].AttachWeapon(gun, m_Owner->GetMesh());
	}
	else
	{
		AGun* gun = m_WeaponAttachPoints[m_AttachPointIndex].DetachWeapon();
		m_Owner->EquipWeapon(gun, false);
	}
}

/////////////////////////////////////////////////////
void UInventoryComponent::PlayHolsteringAnimation_Multicast_Implementation(UAnimMontage* HolsterAnimationToPlay)
{
	UAnimInstance* animInstance = m_Owner->GetMesh()->GetAnimInstance();
	animInstance->Montage_Play(HolsterAnimationToPlay, 1.0f, EMontagePlayReturnType::MontageLength, 0.0f, false);
}

/////////////////////////////////////////////////////
void UInventoryComponent::UnequipAndAttachWeapon_Multicast_Implementation(int32 AttachPointIndex, AGun* Gun)
{
	FWeaponAttachPoint& attachPoint = m_WeaponAttachPoints[AttachPointIndex];
	m_Owner->UnequipWeapon(false, false); // We un-equip the gun in case we are currently holding it.	
	attachPoint.AttachWeapon(Gun, m_Owner->GetMesh());
}

/////////////////////////////////////////////////////
void UInventoryComponent::DetachAndEquipWeapon_Multicast_Implementation(int32 AttachPointIndex)
{
	FWeaponAttachPoint& attachPoint = m_WeaponAttachPoints[AttachPointIndex];
	AGun* gun = attachPoint.DetachWeapon();
	m_Owner->EquipWeapon(gun, false);
}