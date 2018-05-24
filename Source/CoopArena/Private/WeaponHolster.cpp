// Fill out your copyright notice in the Description page of Project Settings.

#include "WeaponHolster.h"
#include "Gun.h"
#include "Animation/AnimInstance.h"
#include "PlayerCharacter.h"
#include "Engine/World.h"


// Sets default values for this component's properties
UWeaponHolster::UWeaponHolster()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}


bool UWeaponHolster::AddItem_Implementation(class AItemBase* itemToAdd)
{
	AGun* gunToAdd = Cast<AGun>(itemToAdd);
	if (!gunToAdd || _CarriedGun)
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
	return true;
}

int32 UWeaponHolster::GetInventorySize_Implementation() const
{
	return 0;
}

bool UWeaponHolster::WeaponCanReloadFrom_Implementation() const
{
	return true;
}


// Called when the game starts
void UWeaponHolster::BeginPlay()
{
	Super::BeginPlay();

	// ...
	
}


void UWeaponHolster::AttachGunToHolster(AGun* GunToAttach)
{
	APlayerCharacter* owner = Cast<APlayerCharacter>(GetOwner());
	GunToAttach->AttachToComponent(this, FAttachmentTransformRules::SnapToTargetNotIncludingScale);

	/*FTransform socketTransform = GunToAttach->GetMesh()->GetSocketTransform(AttachPoint) * GunToAttach->GetTransform().Inverse();
	FRotator newRotation = FRotator::ZeroRotator;
	FVector offset = newRotation.RotateVector(socketTransform.GetLocation());
	GunToAttach->SetActorRelativeRotation(newRotation);
	GunToAttach->SetActorRelativeLocation(offset);*/

	FVector offset = GunToAttach->GetActorLocation() - GunToAttach->GetMesh()->GetSocketLocation("AttachPoint_Right");
	GunToAttach->SetActorRelativeLocation(offset);

	UAnimInstance* AnimInstance;
	AnimInstance = owner->GetMesh()->GetAnimInstance();
	AnimInstance->Montage_Play(_HolsteringAnimations, -1.0f);

	
}
