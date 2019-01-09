// Fill out your copyright notice in the Description page of Project Settings.

#include "SimpleInventory.h"
#include "Weapons/Magazine.h"
#include "UnrealNetwork.h"
#include "Humanoid.h"
#include "Components/HealthComponent.h"
#include "Animation/AnimInstance.h"
#include "PickUp.h"
#include "Kismet/GameplayStatics.h"
#include "PlayerCharacter.h"
#include "CoopArena.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "DefaultHUD.h"


USimpleInventory::USimpleInventory()
{
	PrimaryComponentTick.bCanEverTick = false;
	bReplicates = true;
	bAutoActivate = true;
	_bDropInventoryOnDestroy = false;
	_bDropInventoryOnDeath = true;

	_DefaultMaxNumberOfMagazines = 5;
}

/////////////////////////////////////////////////////
void USimpleInventory::BeginPlay()
{
	Super::BeginPlay();
	
	_Owner = Cast<AHumanoid>(GetOwner());
	check(_Owner);
	_Owner->HolsterWeapon_Event.AddDynamic(this, &USimpleInventory::HandleOwnerHolsterWeapon);

	if (Cast<APlayerCharacter>(GetOwner()))
	{
		_bDropInventoryOnDeath = false;
	}

	if (GetOwner()->HasAuthority())
	{
		SetupDefaultMagazines();

		if (_bDropInventoryOnDeath)
		{
			UHealthComponent* healthComp =  Cast<UHealthComponent>(GetOwner()->GetComponentByClass(UHealthComponent::StaticClass()));
			ensureMsgf(healthComp, TEXT("'%s' does not have a UHealthComponent but it's USimpleInventoryComponent is set to drop it's content on death."));
			
			healthComp->OnDeath.AddDynamic(this, &USimpleInventory::OnOwnerDeath);
			_Owner->OnBeginInteract_Event.AddDynamic(this, &USimpleInventory::OnOwnerBeeingInteractedWith);
		}

		if (_bDropInventoryOnDestroy)
		{
			GetOwner()->OnDestroyed.AddDynamic(this, &USimpleInventory::OnOwnerDestroyed);
		}
	}

}

/////////////////////////////////////////////////////
void USimpleInventory::SetupDefaultMagazines()
{
	ensureMsgf(GetOwner()->HasAuthority(), TEXT("This is a server function. Do not call this on clients."));
	
	for (auto magazine : _MagazinesToSpawnWith)
	{
		ensureMsgf(magazine.Key, TEXT("'%s' has a magazine in the 'Magazines to spawn with' array without a set class."), *GetOwner()->GetName());
		ensureMsgf(magazine.Value >= -1, TEXT("%s: No negative numbers for the maximum magazine count."), *GetOwner()->GetName());

		const TSubclassOf<AMagazine> key = magazine.Key;
		const int32 maxMagazineCount = GetMaxMagazineCountForType(key);
		const int32 value = maxMagazineCount < magazine.Value ? maxMagazineCount : magazine.Value;

		_StoredMagazines.Add(FMagazineStack(key, value));

		if (value == -1)
		{
			_MaxNumberOfMagazines.Add(key, value);
		}
	}
}

/////////////////////////////////////////////////////
void USimpleInventory::OnOwnerDeath(AActor* DeadActor, AController* Controller, AController* Killer)
{
	SetOwnerInteractable(true);
}

void USimpleInventory::OnOwnerBeeingInteractedWith(APawn* InteractingPawn, UPrimitiveComponent* HitComponent)
{
	TransfereInventoryContent(InteractingPawn);
}

void USimpleInventory::OnOwnerDestroyed(AActor* DestroyedOwner)
{
	DropInventoryContent();
}

/////////////////////////////////////////////////////
void USimpleInventory::DropInventoryContent()
{
	for (FMagazineStack& magStack : _StoredMagazines)
	{
		FTransform spawnTransform = GetOwner()->GetActorTransform();
		spawnTransform.SetRotation(FQuat());

		FVector newLocation = spawnTransform.GetLocation();
		FVector2D randLocationOffset = FMath::RandPointInCircle(50.0f);
		newLocation.X += randLocationOffset.X;
		newLocation.Y += randLocationOffset.Y;
		spawnTransform.SetLocation(newLocation);

		APickUp* pickUp = GetWorld()->SpawnActorDeferred<APickUp>(APickUp::StaticClass(), spawnTransform, nullptr, nullptr, ESpawnActorCollisionHandlingMethod::AlwaysSpawn);
		if (pickUp)
		{
			const int32 stackSize = magStack.stackSize < 1 ? FMath::RandRange(1, 3) : magStack.stackSize;
			pickUp->SetMagazineStack(magStack.magClass, stackSize);
			UGameplayStatics::FinishSpawningActor(pickUp, spawnTransform);
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("PickUp couldn't be spawned at %s"), *newLocation.ToString());
		}
	}
}

/////////////////////////////////////////////////////
FMagazineStack* USimpleInventory::FindMagazineStack(const TSubclassOf<AMagazine>& MagazineType)
{
	return _StoredMagazines.FindByPredicate([&](FMagazineStack Stack) {return Stack == MagazineType; });
}

const FMagazineStack* USimpleInventory::FindMagazineStack(const TSubclassOf<AMagazine>& MagazineType) const
{
	return _StoredMagazines.FindByPredicate([&](FMagazineStack Stack) {return Stack == MagazineType; });
}

/////////////////////////////////////////////////////
void USimpleInventory::SetOwnerInteractable_Implementation(bool bCanBeInteractedWith)
{
	IInteractable::Execute_SetCanBeInteractedWith(_Owner, bCanBeInteractedWith);
	_Owner->GetMesh()->SetCollisionResponseToChannel(ECC_Interactable, bCanBeInteractedWith ? ECR_Block : ECR_Ignore);
}

/////////////////////////////////////////////////////
void USimpleInventory::TransfereInventoryContent(APawn* InteractingPawn)
{
	ensureMsgf(GetOwner()->HasAuthority(), TEXT("Only call this function as the server!"));

	USimpleInventory* inventory = Cast<USimpleInventory>(InteractingPawn->GetComponentByClass(USimpleInventory::StaticClass()));
	if (inventory)
	{
		bool bIsEmpty = true;
		for(FMagazineStack& magStack : _StoredMagazines)
		{
			int32 stackSize = magStack.stackSize;
			if (magStack.stackSize == -1)
			{
				stackSize = FMath::RandRange(1, 5);
				magStack.stackSize = 0;
			}
			
			int32 freeSpace;
			const bool bHasRoom = inventory->HasSpaceForMagazine(magStack.magClass, freeSpace, stackSize);
			if (freeSpace == -1)
			{
				continue;	// If the interacting character has an infinite amount of magazines for a type, then we don't add any more to the inventory.
			}

			if (bHasRoom)
			{
				inventory->AddMagazineToInventory(magStack.magClass, stackSize);
			}
			else if (freeSpace != 0)
			{
				inventory->AddMagazineToInventory(magStack.magClass, freeSpace);
				magStack.stackSize -= freeSpace;
			}

			if (magStack.stackSize != 0)
			{
				bIsEmpty = false;
			}
		}

		_StoredMagazines.RemoveAllSwap([](FMagazineStack& magStack) { return magStack.stackSize == 0; });

		if (bIsEmpty)
		{
			SetOwnerInteractable(false);
		}
	}
}

/////////////////////////////////////////////////////
void USimpleInventory::RefreshHud_Implementation()
{
	APlayerController* pc = Cast<APlayerController>(_Owner->GetInstigatorController());
	if (pc)
	{
		ADefaultHUD* hud = Cast<ADefaultHUD>(pc->GetHUD());
		hud->RefreshHud();
	}
}

/////////////////////////////////////////////////////
void USimpleInventory::OnWeaponHolstering()
{
	const bool bAttachToHolster = _Owner->GetEquippedGun() ? true : false;
	if (bAttachToHolster)
	{
		AGun* gun = _Owner->GetEquippedGun();
		_Owner->UnequipWeapon(false, false);

		AGun* holsteredGun = _WeaponAttachPoints[_AttachPointIndex].GetAttachedWeapon();
		if (holsteredGun)
		{
			_WeaponAttachPoints[_AttachPointIndex].DetachWeapon();
			_Owner->EquipWeapon(holsteredGun, false);
		}

		_WeaponAttachPoints[_AttachPointIndex].AttachWeapon(gun, _Owner->GetMesh());
	}
	else
	{
		AGun* gun = _WeaponAttachPoints[_AttachPointIndex].DetachWeapon();
		_Owner->EquipWeapon(gun, false);
	}
}

/////////////////////////////////////////////////////
AGun* USimpleInventory::GetGunAtAttachPoint(int32 AttachPointIndex)
{
	const bool bValidIndex = _WeaponAttachPoints.IsValidIndex(AttachPointIndex);
	return bValidIndex ? _WeaponAttachPoints[AttachPointIndex].GetAttachedWeapon() : nullptr;
}

/////////////////////////////////////////////////////
void USimpleInventory::SetAttachPointIndex(int32 Index)
{
	_AttachPointIndex = Index;
}

/////////////////////////////////////////////////////
void USimpleInventory::HandleOwnerHolsterWeapon(AGun* GunToHolster, int32 AttachPointIndex)
{
	if (GetOwner()->HasAuthority() == false)
	{
		OnOwnerHolsterWeapon_Server(GunToHolster, AttachPointIndex);
		return;
	}

	bool bCanAttach = AttachPointIndex > -1 ? _WeaponAttachPoints[AttachPointIndex].CanAttachWeapon(GunToHolster) : false;
	if (AttachPointIndex < 0 && !bCanAttach && GunToHolster)
	{
		for (int32 i = 0; i < _WeaponAttachPoints.Num(); ++i)
		{
			bCanAttach = _WeaponAttachPoints[i].CanAttachWeapon(GunToHolster);
			if (bCanAttach)
			{
				AttachPointIndex = i;
				break;
			}
		}
	}
	_AttachPointIndex = _WeaponAttachPoints.Num() == 1 ? 0 : AttachPointIndex;

	if (!bCanAttach || _WeaponAttachPoints.IsValidIndex(_AttachPointIndex) == false)
	{
		return;
	}

	UAnimMontage* holsterAnimation = _WeaponAttachPoints[_AttachPointIndex].holsterAnimation;
	UAnimInstance* animInstance = _Owner->GetMesh()->GetAnimInstance();

	if (GunToHolster)
	{
		if (animInstance && holsterAnimation)
		{
			PlayHolsteringAnimation_Multicast(holsterAnimation);
		}
		else
		{
			UnequipAndAttachWeapon_Multicast(_AttachPointIndex, GunToHolster);
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
			DetachAndEquipWeapon_Multicast(_AttachPointIndex);
		}
	}
}

/////////////////////////////////////////////////////
void USimpleInventory::DetachAndEquipWeapon_Multicast_Implementation(int32 AttachPointIndex)
{
	FWeaponAttachPoint& attachPoint = _WeaponAttachPoints[AttachPointIndex];
	AGun* gun = attachPoint.DetachWeapon();
	_Owner->EquipWeapon(gun, false);
}

/////////////////////////////////////////////////////
void USimpleInventory::UnequipAndAttachWeapon_Multicast_Implementation(int32 AttachPointIndex, AGun* Gun)
{
	FWeaponAttachPoint& attachPoint = _WeaponAttachPoints[AttachPointIndex];
	_Owner->UnequipWeapon(false, false); // We un-equip the gun in case we are currently holding it.	
	attachPoint.AttachWeapon(Gun, _Owner->GetMesh());
}

/////////////////////////////////////////////////////
void USimpleInventory::PlayHolsteringAnimation_Multicast_Implementation(UAnimMontage* HolsterAnimationToPlay)
{
	UAnimInstance* animInstance = _Owner->GetMesh()->GetAnimInstance();
	float montageLength = animInstance->Montage_Play(HolsterAnimationToPlay, 1.0f, EMontagePlayReturnType::MontageLength, 0.0f, false);
	GetWorld()->GetTimerManager().SetTimer(_MontageFinishedTH, [&]() {OnHolsteringWeaponFinished.Broadcast(_Owner); }, montageLength, false);
}

/////////////////////////////////////////////////////
void USimpleInventory::OnOwnerHolsterWeapon_Server_Implementation(AGun* GunToHolster, int32 AttachPointIndex)
{
	HandleOwnerHolsterWeapon(GunToHolster, AttachPointIndex);
}

bool USimpleInventory::OnOwnerHolsterWeapon_Server_Validate(AGun* GunToHolster, int32 AttachPointIndex)
{
	return true;
}

/////////////////////////////////////////////////////
int32 USimpleInventory::GetMaxMagazineCountForType(TSubclassOf<AMagazine> MagazineType) const
{
	const int32* pointer = _MaxNumberOfMagazines.Find(MagazineType);
	return pointer ? *pointer : _DefaultMaxNumberOfMagazines;
}

/////////////////////////////////////////////////////
int32 USimpleInventory::GetNumberOfMagazinesForType(TSubclassOf<AMagazine> MagazineType) const
{
	const FMagazineStack* magStack = FindMagazineStack(MagazineType);
	return magStack ? magStack->stackSize : 0;
}

/////////////////////////////////////////////////////
bool USimpleInventory::HasSpaceForMagazine(TSubclassOf<AMagazine> MagazineType, int32& Out_FreeSpace, int32 NumMagazinesToStore /*= 1*/) const
{
	ensureMsgf(NumMagazinesToStore > 0, TEXT("The number of magazines to store in this inventory must be greater than 0"));

	const int32 maxMagCount = GetMaxMagazineCountForType(MagazineType);
	if (maxMagCount == -1)
	{
		Out_FreeSpace = -1;
		return true;
	}

	const FMagazineStack* magStack = FindMagazineStack(MagazineType);
	const int32 numberOfMagsInInventory = magStack ? magStack->stackSize : 0;

	Out_FreeSpace = maxMagCount - numberOfMagsInInventory;
	return numberOfMagsInInventory + NumMagazinesToStore <= Out_FreeSpace;
}

bool USimpleInventory::HasSpaceForMagazine(int32 NumMagazinesToStore, TSubclassOf<AMagazine> MagazineType) const
{
	int32 freeSpace;
	return HasSpaceForMagazine(MagazineType, freeSpace, NumMagazinesToStore);
}

/////////////////////////////////////////////////////
bool USimpleInventory::HasMagazines(TSubclassOf<AMagazine> MagazineType, int32 NumMagazines /*= 1*/) const
{
	const int32 numMagazinesInInventory = GetNumberOfMagazinesForType(MagazineType);
	const bool bHasInfiniteMagazines = numMagazinesInInventory == -1;

	return bHasInfiniteMagazines || numMagazinesInInventory >= NumMagazines;
}

/////////////////////////////////////////////////////
bool USimpleInventory::AddMagazineToInventory(TSubclassOf<AMagazine> MagazineType, int32 NumMagazinesToStore /*= 1*/)
{
	const bool bHasSpace = HasSpaceForMagazine(NumMagazinesToStore, MagazineType);
	if (!bHasSpace)
	{
		return false;
	}

	if (GetOwner()->HasAuthority())
	{
		FMagazineStack* magStack = FindMagazineStack(MagazineType);
		if (magStack && magStack->stackSize != -1)
		{
			magStack->stackSize += NumMagazinesToStore;
		}
		else
		{
			_StoredMagazines.Add(FMagazineStack(MagazineType, NumMagazinesToStore));
		}
		RefreshHud();
	}
	else
	{
		AddMagazineToInventory_Server(MagazineType, NumMagazinesToStore);
	}
	
	return true;
}

void USimpleInventory::AddMagazineToInventory_Server_Implementation(TSubclassOf<AMagazine> MagazineType, int32 NumMagazinesToStore /*= 1*/)
{
	AddMagazineToInventory(MagazineType, NumMagazinesToStore);
}

bool USimpleInventory::AddMagazineToInventory_Server_Validate(TSubclassOf<AMagazine> MagazineType, int32 NumMagazinesToStore /*= 1*/)
{
	return true;
}

/////////////////////////////////////////////////////
bool USimpleInventory::GetMagazineFromInventory(TSubclassOf<AMagazine> MagazineType, int32 NumMagazinesToRemove /*= 1*/)
{
	const bool bHasEnoughMagazines = HasMagazines(MagazineType, NumMagazinesToRemove);
	if (!bHasEnoughMagazines)
	{
		return false;
	}

	if (GetOwner()->HasAuthority())
	{
		if(GetMaxMagazineCountForType(MagazineType) != -1)
		{
			FMagazineStack* stack = FindMagazineStack(MagazineType);
			stack->stackSize -= NumMagazinesToRemove;
			RefreshHud();
		}
	}
	else
	{
		GetMagazineFromInventory_Server(MagazineType, NumMagazinesToRemove);
	}

	return true;
}

void USimpleInventory::GetMagazineFromInventory_Server_Implementation(TSubclassOf<AMagazine> MagazineType, int32 NumMagazinesToRemove /*= 1*/)
{
	GetMagazineFromInventory(MagazineType);
}

bool USimpleInventory::GetMagazineFromInventory_Server_Validate(TSubclassOf<AMagazine> MagazineType, int32 NumMagazinesToRemove /*= 1*/)
{
	return true;
}

/////////////////////////////////////////////////////
void USimpleInventory::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(USimpleInventory, _StoredMagazines, COND_OwnerOnly);
}