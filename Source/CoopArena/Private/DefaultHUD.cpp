// Fill out your copyright notice in the Description page of Project Settings.

#include "DefaultHUD.h"
#include "PlayerCharacter.h"
#include "HUDInterface.h"
#include "Gun.h"
#include "Magazine.h"
#include "SimpleInventory.h"
#include "Blueprint/UserWidget.h"
#include "TimerManager.h"
#include "WidgetBlueprintLibrary.h"
#include "WidgetLayoutLibrary.h"


ADefaultHUD::ADefaultHUD()
{
	_TimeToHideHUD = 4.0f;
}

/////////////////////////////////////////////////////
void ADefaultHUD::Init(APlayerCharacter* PlayerCharacter)
{
	verify(PlayerCharacter);
	if (PlayerCharacter != _Character)
	{
		if (_Character != nullptr)
		{
			_Character->OnReloadFinished.RemoveDynamic(this, &ADefaultHUD::HandleOnReloadingFinished);
			_Character->OnWeaponFire.RemoveDynamic(this, &ADefaultHUD::HandleOnWeaponFire);
			_Character->OnFireModeChanged.RemoveDynamic(this, &ADefaultHUD::HandleOnFireModeChanged);
			_Character->OnWeaponEquipped.RemoveDynamic(this, &ADefaultHUD::HandleOnWeaponEquipped);
		}

		_Character = PlayerCharacter;
		_Character->OnReloadFinished.AddDynamic(this, &ADefaultHUD::HandleOnReloadingFinished);
		_Character->OnWeaponFire.AddDynamic(this, &ADefaultHUD::HandleOnWeaponFire);
		_Character->OnFireModeChanged.AddDynamic(this, &ADefaultHUD::HandleOnFireModeChanged);
		_Character->OnWeaponEquipped.AddDynamic(this, &ADefaultHUD::HandleOnWeaponEquipped);
	}
}

/////////////////////////////////////////////////////
void ADefaultHUD::HandleOnWeaponEquipped(AHumanoid* Player, AGun* Gun)
{
	HandleOnFireModeChanged(Player, Gun->GetCurrentFireMode());
	HandleOnReloadingFinished(Player, Gun);
}

/////////////////////////////////////////////////////
void ADefaultHUD::BeginPlay()
{
	Super::BeginPlay();

	UWidgetLayoutLibrary::RemoveAllWidgets(PlayerOwner);	
	DisplayHUD();

	UWidgetBlueprintLibrary::SetInputMode_GameOnly(PlayerOwner);
	PlayerOwner->bShowMouseCursor = false;
}

/////////////////////////////////////////////////////
void ADefaultHUD::RefreshHud()
{
	AHumanoid* owner = Cast<AHumanoid>(GetOwningPawn());
	HandleOnReloadingFinished(owner, owner->GetEquippedGun());
	DisplayHUD();
}

/////////////////////////////////////////////////////
void ADefaultHUD::HandleOnReloadingFinished(AHumanoid* Character, AGun* Gun)
{
	verify(Character);
	verify(Gun);

	AMagazine* magazine = Gun->GetMagazine();
	if (magazine == nullptr)
	{
		return;
	}

	int32 numMagazines = 0;
	int32 numRounds = magazine->RoundsLeft();

	USimpleInventory* inventory = Cast<USimpleInventory>(Character->GetComponentByClass(USimpleInventory::StaticClass()));
	numMagazines = inventory->GetNumberOfMagazinesForType(magazine->GetClass());

	HandleOnReloadingFinished_K2(numMagazines, numRounds);

	DisplayHUD();
}

/////////////////////////////////////////////////////
void ADefaultHUD::HandleOnWeaponFire(AHumanoid* Character, AGun* Gun)
{
	verify(Character);
	verify(Gun);

	int32 roundsLeft = Gun->GetMagazine()->RoundsLeft();
	HandleOnWeaponFire_K2(roundsLeft);

	DisplayHUD();
}

/////////////////////////////////////////////////////
void ADefaultHUD::HandleOnFireModeChanged(AHumanoid* Character, EFireMode NewFireMode)
{
	HandleOnFireModeChanged_K2(Character, NewFireMode);
	DisplayHUD();
}

/////////////////////////////////////////////////////
void ADefaultHUD::DisplayHUD()
{
	if (_AmmoStatusWidget)
	{
		_AmmoStatusWidget->SetVisibility(ESlateVisibility::Visible);

		GetWorld()->GetTimerManager().SetTimer(_Timer, [&]()
		{
			_AmmoStatusWidget->SetVisibility(ESlateVisibility::Hidden);
		}, _TimeToHideHUD, false);
	}
}

/////////////////////////////////////////////////////
void ADefaultHUD::GetAmmoStatus(int32& Out_NumMagazines, int32& Out_NumRoundsLeft, EFireMode& Out_FireMode)
{
	AHumanoid* myowner = Cast<AHumanoid>(GetOwningPawn());	
	AGun* gun = myowner ? myowner->GetEquippedGun() : nullptr;
	AMagazine* magazine = gun ? gun->GetMagazine() : nullptr;

	if (!myowner || !gun || !magazine)
	{
		return;
	}

	Out_NumRoundsLeft = magazine->RoundsLeft();	

	Out_FireMode = gun->GetCurrentFireMode();

	USimpleInventory* inventory = Cast<USimpleInventory>(myowner->GetComponentByClass(USimpleInventory::StaticClass()));
	Out_NumMagazines = inventory->GetNumberOfMagazinesForType(magazine->GetClass());	
}