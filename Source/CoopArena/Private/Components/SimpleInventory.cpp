// Fill out your copyright notice in the Description page of Project Settings.

#include "SimpleInventory.h"
#include "Weapons/Magazine.h"
#include "UnrealNetwork.h"


USimpleInventory::USimpleInventory()
{
	PrimaryComponentTick.bCanEverTick = false;
	bReplicates = true;
	bAutoActivate = true;

	m_DefaultMaxNumberOfMagazines = 5;
}

/////////////////////////////////////////////////////
void USimpleInventory::BeginPlay()
{
	Super::BeginPlay();
	if (GetOwner()->HasAuthority())
	{
		SetupDefaultMagazines();
	}
}

/////////////////////////////////////////////////////
void USimpleInventory::SetupDefaultMagazines()
{
	for (auto magazine : m_MagazinesToSpawnWith)
	{
		ensureMsgf(magazine.Key, TEXT("'%s' has a magazine in the 'Magazines to spawn with' array without a set class."), *GetOwner()->GetName());
		ensureMsgf(magazine.Value >= 0, TEXT("%s: No negative numbers for the maximum magazine count."), *GetOwner()->GetName());
		ensureMsgf(GetOwner()->HasAuthority(), TEXT("This is a server function. Do not call this on clients."));

		const TSubclassOf<AMagazine> key = magazine.Key;
		const int32 maxMagazineCount = GetMaxMagazineCountForType(key);
		const int32 value = maxMagazineCount < magazine.Value ? maxMagazineCount : magazine.Value;

		m_StoredMagazines.Add(FMagazineStack(key, value));
	}
}

/////////////////////////////////////////////////////
FMagazineStack* USimpleInventory::FindMagazineStack(const TSubclassOf<AMagazine>& MagazineType)
{
	return m_StoredMagazines.FindByPredicate([&](FMagazineStack Stack) {return Stack == MagazineType; });
}

const FMagazineStack* USimpleInventory::FindMagazineStack(const TSubclassOf<AMagazine>& MagazineType) const
{
	return m_StoredMagazines.FindByPredicate([&](FMagazineStack Stack) {return Stack == MagazineType; });
}

/////////////////////////////////////////////////////
int32 USimpleInventory::GetMaxMagazineCountForType(TSubclassOf<AMagazine> MagazineType) const
{
	const int32* pointer = m_MaxNumberOfMagazines.Find(MagazineType);
	return pointer ? *pointer : m_DefaultMaxNumberOfMagazines;
}

/////////////////////////////////////////////////////
int32 USimpleInventory::GetNumberOfMagazinesForType(TSubclassOf<AMagazine> MagazineType) const
{
	const FMagazineStack* magStack = FindMagazineStack(MagazineType);
	return magStack ? magStack->stackSize : 0;
}

/////////////////////////////////////////////////////
bool USimpleInventory::HasSpaceForMagazine(TSubclassOf<AMagazine> MagazineType, int32 NumMagazinesToStore /*= 1*/) const
{
	ensureMsgf(NumMagazinesToStore > 0, TEXT("The number of magazines to store in this inventory must be greater than 0"));

	const FMagazineStack* magStack = FindMagazineStack(MagazineType);
	const int32 numberOfMagsInInventory = magStack ? magStack->stackSize : 0;

	const int32* maxNumMags_Pointer = m_MaxNumberOfMagazines.Find(MagazineType);
	const int32 maxNumberOfMagsInInventory = maxNumMags_Pointer ? *maxNumMags_Pointer : m_DefaultMaxNumberOfMagazines;

	return numberOfMagsInInventory + NumMagazinesToStore <= maxNumberOfMagsInInventory;
}

bool USimpleInventory::HasMagazines(TSubclassOf<AMagazine> MagazineType, int32 NumMagazines /*= 1*/) const
{
	return GetNumberOfMagazinesForType(MagazineType) >= NumMagazines;
}

/////////////////////////////////////////////////////
bool USimpleInventory::AddMagazineToInventory(TSubclassOf<AMagazine> MagazineType, int32 NumMagazinesToStore /*= 1*/)
{
	const bool bHasSpace = HasSpaceForMagazine(MagazineType, NumMagazinesToStore);
	if (!bHasSpace)
	{
		return false;
	}

	if (GetOwner()->HasAuthority())
	{
		FMagazineStack* magStack = FindMagazineStack(MagazineType);
		if (magStack)
		{
			magStack->stackSize += NumMagazinesToStore;
		}
		else
		{
			m_StoredMagazines.Add(FMagazineStack(MagazineType, NumMagazinesToStore));
		}
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
		FMagazineStack* stack = FindMagazineStack(MagazineType);
		stack->stackSize -= NumMagazinesToRemove;
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

	DOREPLIFETIME_CONDITION(USimpleInventory, m_StoredMagazines, COND_OwnerOnly);
}