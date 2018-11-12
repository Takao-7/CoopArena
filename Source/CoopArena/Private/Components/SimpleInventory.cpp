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

		int32& newValue = m_StoredMagazines.Add(key, value);
	}
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
	const int32* pointer = m_StoredMagazines.Find(MagazineType);
	return pointer ? *pointer : 0;
}

/////////////////////////////////////////////////////
bool USimpleInventory::HasSpaceForMagazine(TSubclassOf<AMagazine> MagazineType, int32 NumMagazinesToStore /*= 1*/) const
{
	ensureMsgf(NumMagazinesToStore > 0, TEXT("The number of magazines to store in this inventory must be greater than 0"));

	const int32* numMags_Pointer = m_StoredMagazines.Find(MagazineType);
	const int32 numberOfMagsInInventory = numMags_Pointer ? *numMags_Pointer : 0;

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

	AddMagazineToInventory_Server(MagazineType, NumMagazinesToStore);
	return true;
}

void USimpleInventory::AddMagazineToInventory_Server_Implementation(TSubclassOf<AMagazine> MagazineType, int32 NumMagazinesToStore /*= 1*/)
{
	int32* pointer = m_StoredMagazines.Find(MagazineType);
	pointer ? *pointer += NumMagazinesToStore : m_StoredMagazines.Add(MagazineType, NumMagazinesToStore);
}

bool USimpleInventory::AddMagazineToInventory_Server_Validate(TSubclassOf<AMagazine> MagazineType, int32 NumMagazinesToStore /*= 1*/)
{
	return HasSpaceForMagazine(MagazineType, NumMagazinesToStore);
}

/////////////////////////////////////////////////////
bool USimpleInventory::GetMagazineFromInventory(TSubclassOf<AMagazine> MagazineType, int32 NumMagazinesToRemove /*= 1*/)
{
	const bool bHasEnoughMagazines = HasMagazines(MagazineType, NumMagazinesToRemove);
	if (!bHasEnoughMagazines)
	{
		return false;
	}

	GetMagazineFromInventory_Server(MagazineType, NumMagazinesToRemove);
	return true;
}

void USimpleInventory::GetMagazineFromInventory_Server_Implementation(TSubclassOf<AMagazine> MagazineType, int32 NumMagazinesToRemove /*= 1*/)
{
	int32* pointer = m_StoredMagazines.Find(MagazineType);
	*pointer -= NumMagazinesToRemove;
}

bool USimpleInventory::GetMagazineFromInventory_Server_Validate(TSubclassOf<AMagazine> MagazineType, int32 NumMagazinesToRemove /*= 1*/)
{
	return HasMagazines(MagazineType, NumMagazinesToRemove);
}

/////////////////////////////////////////////////////
void USimpleInventory::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(USimpleInventory, m_StoredMagazines, COND_OwnerOnly);
}