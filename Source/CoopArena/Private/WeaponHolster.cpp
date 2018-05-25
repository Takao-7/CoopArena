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

	FVector gunForwardV = GunToAttach->GetMesh()->GetSocketLocation("AttachPoint_Right");
	gunForwardV.Normalize();
	FVector holsterForwardV = GetForwardVector();
	UE_LOG(LogTemp, Warning, TEXT("Gun forward vector: %s. Holster forward vector: %s"), *gunForwardV.ToString(), *holsterForwardV.ToString());

	FVector normalForward = GetComponentLocation();
	normalForward.Normalize();
	UE_LOG(LogTemp, Warning, TEXT("Holster vector normalized: %s"), *normalForward.ToString());

	float dotProduct = FMath::Acos(FVector::DotProduct(gunForwardV, holsterForwardV));
	dotProduct = FMath::RadiansToDegrees(dotProduct);
	FVector crossProduct = FVector::CrossProduct(gunForwardV, holsterForwardV);
	crossProduct.Normalize();
	FQuat newRotation = FQuat(crossProduct, dotProduct);

	GunToAttach->SetActorRelativeRotation(newRotation);

	FAttachmentTransformRules rules = FAttachmentTransformRules(EAttachmentRule::SnapToTarget, EAttachmentRule::KeepWorld, EAttachmentRule::KeepWorld, false);
	GunToAttach->AttachToComponent(this, rules);

	/*FVector locationOffset = GunToAttach->GetActorLocation() - GunToAttach->GetMesh()->GetSocketLocation("AttachPoint_Right");
	GunToAttach->SetActorRelativeLocation(locationOffset);*/

	UAnimInstance* AnimInstance;
	AnimInstance = owner->GetMesh()->GetAnimInstance();
	AnimInstance->Montage_Play(_HolsteringAnimations, -1.0f);
}