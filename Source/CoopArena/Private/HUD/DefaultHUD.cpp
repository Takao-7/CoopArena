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
	UWidgetBlueprintLibrary::SetInputMode_GameOnly(PlayerOwner);
	PlayerOwner->bShowMouseCursor = false;

	OnDestroyed.AddDynamic(this, &ADefaultHUD::HandleOnDestroyed);
}

/////////////////////////////////////////////////////
void ADefaultHUD::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	APlayerCharacter* character = Cast<APlayerCharacter>(GetOwningPawn());
	if (character && PlayerOwner && character->GetEquippedGun())
	{
		Init(character);

		if (_AmmoStatusWidget_Class)
		{
			_AmmoStatusWidget = CreateWidget<UUserWidget>(PlayerOwner, _AmmoStatusWidget_Class, TEXT("Ammo status widget"));
			_AmmoStatusWidget->AddToViewport();
		}

		if (_WaveMessage_Class)
		{
			_WaveMessage = CreateWidget<UUserWidget>(PlayerOwner, _WaveMessage_Class, TEXT("Wave message widget"));
			_WaveMessage->AddToViewport();
		}

		RefreshHud();
		PrimaryActorTick.SetTickFunctionEnable(false);
	}
}

/////////////////////////////////////////////////////
void ADefaultHUD::HandleOnDestroyed(AActor* DestroyedActor)
{
	GetWorld()->GetTimerManager().ClearAllTimersForObject(DestroyedActor);
}

/////////////////////////////////////////////////////
void ADefaultHUD::RefreshHud()
{
	AGun* equippedGun = _Character->GetEquippedGun();
	HandleOnReloadingFinished(_Character, equippedGun);
	HandleOnFireModeChanged(_Character, equippedGun->GetCurrentFireMode());
}

/////////////////////////////////////////////////////
void ADefaultHUD::SetState_Implementation(EHUDState State)
{
	_HUDState = State;
}

/////////////////////////////////////////////////////
void ADefaultHUD::ToggleInGameMenu()
{
	if (_InGameMenu_Class)
	{
		if (_InGameMenu)
		{
			_InGameMenu->RemoveFromViewport();
			_InGameMenu = nullptr;
		}
		else
		{
			_InGameMenu = CreateWidget<UUserWidget>(PlayerOwner, _InGameMenu_Class, TEXT("Ingame menu"));
			_InGameMenu->AddToViewport(9);
		}
	}
}

/////////////////////////////////////////////////////
void ADefaultHUD::ToggleScoreBoard()
{
	if (_ScoreBoard_Class)
	{
		if (_ScoreBoard)
		{
			_ScoreBoard->RemoveFromViewport();
			_ScoreBoard = nullptr;
		}
		else
		{
			_ScoreBoard = CreateWidget<UUserWidget>(PlayerOwner, _ScoreBoard_Class, TEXT("Score board"));
			_ScoreBoard->AddToViewport(1);
		}
	}
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
			if (_AmmoStatusWidget)
			{
				_AmmoStatusWidget->SetVisibility(ESlateVisibility::Hidden);
			}
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
