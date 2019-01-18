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
	_WeaponChangeTime = 1.0f;
	_SocketRifle = FName(TEXT("WeaponHolster_Rifle"));
	_SocketPistol = FName(TEXT("WeaponHolster_Pistol"));
}

/////////////////////////////////////////////////////
void USimpleInventory::BeginPlay()
{
	Super::BeginPlay();
	
	if (Cast<APlayerCharacter>(GetOwner()))
	{
		_bDropInventoryOnDeath = false;
	}

	_Owner = Cast<AHumanoid>(GetOwner());
	check(_Owner);

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
		if (magStack.stackSize == 0)
		{
			continue;
		}

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
			const int32 stackSize = magStack.stackSize == -1 ? FMath::RandRange(1, 3) : magStack.stackSize;
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

	USimpleInventory* otherInventory = Cast<USimpleInventory>(InteractingPawn->GetComponentByClass(USimpleInventory::StaticClass()));
	if (otherInventory == nullptr || _StoredMagazines.Num() == 0)
	{
		return;
	}

	bool bIsEmpty = true;
	for(FMagazineStack& magStack : _StoredMagazines)
	{
		int32& stackSize = magStack.stackSize;
		if (stackSize == 0)
		{
			continue;
		}
		else if (stackSize == -1)
		{
			stackSize = FMath::RandRange(1, 3);
		}
			
		int32 freeSpace;
		const bool bHasRoom = otherInventory->HasSpaceForMagazine(magStack.magClass, freeSpace, stackSize);
		if (freeSpace == -1)
		{
			continue;	// If the interacting character has an infinite amount of magazines for a type, then we don't add any more to the inventory.
		}

		if (bHasRoom) // Add the entire stack to the inventory
		{
			otherInventory->AddMagazineToInventory(magStack.magClass, stackSize);
		}
		else if (freeSpace != 0)
		{
			otherInventory->AddMagazineToInventory(magStack.magClass, freeSpace);
			stackSize -= freeSpace;
			bIsEmpty = false;
		}
		else
		{
			bIsEmpty = false;
		}
	}	

	_StoredMagazines.RemoveAllSwap([](FMagazineStack& stack) { return stack.stackSize == 0; });

	if (bIsEmpty)
	{
		SetOwnerInteractable(false);
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
	AGun* equippedGun = _Owner->GetEquippedGun();
	check(equippedGun);
	_Owner->UnequipWeapon(false, false);
	_Owner->EquipWeapon(_HolsteredWeapon, false);
	AttachGun(equippedGun);
}

/////////////////////////////////////////////////////
AGun* USimpleInventory::GetGunAtAttachPoint()
{
	return _HolsteredWeapon;
}

/////////////////////////////////////////////////////
void USimpleInventory::ChangeWeapon()
{
	UAnimMontage* holsterAnim = GetHolsterAnimation();
	if (holsterAnim)
	{
		if (GetOwner()->HasAuthority())
		{
			PlayHolsteringAnimation_Multicast();
		}
		else
		{
			PlayHolsteringAnimation_Server();
		}
	}
	else
	{
		GetWorld()->GetTimerManager().SetTimer(_ChangeWeaponTH, [&]() { OnWeaponHolstering(); }, _WeaponChangeTime, false);
	}
}

/////////////////////////////////////////////////////
UAnimMontage* USimpleInventory::GetHolsterAnimation()
{
	if (_Owner->GetEquippedGun())
	{
		return _Owner->GetEquippedGun()->GetWeaponType() == EWEaponType::Pistol ? _HolsterAnimation_Pistol : _HolsterAnimation_Rifle;
	}
	else
	{
		return nullptr;
	}
}

/////////////////////////////////////////////////////
void USimpleInventory::AttachGun(AGun* GunToAttach)
{
	check(GunToAttach);
	FName socket = GunToAttach->GetWeaponType() == EWEaponType::Pistol ? _SocketPistol : _SocketRifle;
	GunToAttach->AttachToComponent(_Owner->GetMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, socket);
	_HolsteredWeapon = GunToAttach;
}

/////////////////////////////////////////////////////
void USimpleInventory::SetHolsterWeapon_Multicast_Implementation(AGun* Gun)
{
	_HolsteredWeapon = Gun;
}

/////////////////////////////////////////////////////
void USimpleInventory::PlayHolsteringAnimation_Server_Implementation()
{
	PlayHolsteringAnimation_Multicast();
}

bool USimpleInventory::PlayHolsteringAnimation_Server_Validate()
{
	return true;
}

/////////////////////////////////////////////////////
void USimpleInventory::DetachAndEquipWeapon_Multicast_Implementation()
{
	_Owner->EquipWeapon(_HolsteredWeapon, false);
	_HolsteredWeapon = nullptr;
}

/////////////////////////////////////////////////////
void USimpleInventory::UnequipAndAttachWeapon_Multicast_Implementation(AGun* Gun)
{
	_Owner->UnequipWeapon(false, false); // We un-equip the gun in case we are currently holding it.	
	AttachGun(Gun);
}

/////////////////////////////////////////////////////
void USimpleInventory::PlayHolsteringAnimation_Multicast_Implementation()
{
	UAnimInstance* animInstance = _Owner->GetMesh()->GetAnimInstance();
	animInstance->Montage_Play(GetHolsterAnimation(), 1.0f, EMontagePlayReturnType::MontageLength, 0.0f, false);
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
FName USimpleInventory::GetSocket(EWEaponType WeaponType) const
{
	return WeaponType == EWEaponType::Pistol ? _SocketPistol : _SocketRifle;
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
	return Out_FreeSpace - NumMagazinesToStore >= 0;
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