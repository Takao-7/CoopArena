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

	UAnimInstance* AnimInstance;
	APlayerCharacter* owner = Cast<APlayerCharacter>(GetOwner());
	AnimInstance = owner->GetMesh()->GetAnimInstance();
	float time = AnimInstance->Montage_Play(_HolsteringAnimations, 1.0f);

	FTimerHandle timerHandle;
	FTimerDelegate timerDelegate;
	timerDelegate.BindUFunction(this, "AttachGunToHolster", gunToAdd);
	GetWorld()->GetTimerManager().SetTimer(timerHandle, timerDelegate, time, false);
	_CarriedGun = gunToAdd;

	return true;
}


bool UWeaponHolster::RemoveItem_Implementation(int32 itemIndexToRemove, FItemStats& outItemStats)
{
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
	return true;
}


void UWeaponHolster::AttachGunToHolster(AGun* GunToAttach)
{
	APlayerCharacter* owner = Cast<APlayerCharacter>(GetOwner());

	FQuat rotationOffset = GunToAttach->GetActorRotation().Quaternion() - FQuat::FindBetweenVectors(GetComponentLocation(), GunToAttach->GetMesh()->GetComponentLocation());
	GunToAttach->SetActorRotation(rotationOffset);

	GunToAttach->AttachToComponent(this, FAttachmentTransformRules::KeepWorldTransform);	

	/*FVector positionOffset = GetComponentLocation() - GunToAttach->GetMesh()->GetSocketLocation("AttachPoint_Right");
	GunToAttach->SetActorRelativeLocation(positionOffset);*/

	UAnimInstance* AnimInstance;
	AnimInstance = owner->GetMesh()->GetAnimInstance();
	AnimInstance->Montage_Play(_HolsteringAnimations, -1.0f);	
}