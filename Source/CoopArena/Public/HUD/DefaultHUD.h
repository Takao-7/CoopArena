// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "DefaultHUD.generated.h"


class UUserWidget;
class APlayerCharacter;
class AGun;
class AHumanoid;


UENUM(BlueprintType)
enum class EHUDState : uint8
{
	MainMenu,
	Playing,
	Spectating,
	MatchEnd
};


UCLASS()
class COOPARENA_API ADefaultHUD : public AHUD
{
	GENERATED_BODY()

protected:
	UPROPERTY(BlueprintReadOnly, Category = "HUD")
	APlayerCharacter* _Character;

	UPROPERTY(BlueprintReadWrite, Category = "HUD", meta = (DisplayName = "Ammo status widget"))
	UUserWidget* _AmmoStatusWidget;

	UPROPERTY(EditDefaultsOnly, Category = "HUD")
	TSubclassOf<UUserWidget> _AmmoStatusWidget_Class;

	UPROPERTY(BlueprintReadWrite, Category = "HUD", meta = (DisplayName = "Wave message"))
	UUserWidget* _WaveMessage;

	UPROPERTY(EditDefaultsOnly, Category = "HUD")
	TSubclassOf<UUserWidget> _WaveMessage_Class;

	/* The time, in seconds, after reloading, shooting or switching the weapon mode, the HUD will be hidden. */
	UPROPERTY(EditDefaultsOnly, Category = "HUD")
	float _TimeToHideHUD;

	UFUNCTION(BlueprintImplementableEvent, Category = "HUD", meta = (DisplayName = "HandleOnFireModeChanged"))
	void HandleOnFireModeChanged_K2(AHumanoid* Character, EFireMode NewFireMode);

	/* This event will be called each time the character has finished reloading his weapon. */
	UFUNCTION(BlueprintImplementableEvent, Category = "HUD", meta = (DisplayName = "HandleOnReloadingFinished"))
	void HandleOnReloadingFinished_K2(int32 NumMagazines, int32 NumRoundsInMagazine);

	/* This event will be called each time the character fires his weapon. */
	UFUNCTION(BlueprintImplementableEvent, Category = "HUD", meta = (DisplayName = "HandleOnWeaponFire"))
	void HandleOnWeaponFire_K2(int32 NumRoundsLeft);

	UFUNCTION(BlueprintCallable, Category = "HUD")
	void GetAmmoStatus(int32& Out_NumMagazines, int32& Out_NumRoundsLeft, EFireMode& Out_FireMode);

	virtual void Tick(float DeltaSeconds) override;

public:
	ADefaultHUD();

	UFUNCTION(BlueprintCallable, Category = "HUD")
	void Init(APlayerCharacter* PlayerCharacter);

	virtual void BeginPlay() override;

	UFUNCTION(BlueprintCallable, Category = "HUD")
	void RefreshHud();

	UFUNCTION(BlueprintNativeEvent, Category = "HUD")
	void SetState(EHUDState  State);
	void SetState_Implementation(EHUDState  State);

	UFUNCTION(BlueprintPure, Category = "HUD")
	EHUDState GetCurrentState() const { return _HUDState; };

private:
	FTimerHandle _Timer;

	EHUDState _HUDState;

	UFUNCTION()
	void HandleOnReloadingFinished(AHumanoid* Character, AGun* Gun);

	UFUNCTION()
	void HandleOnWeaponFire(AHumanoid* Character, AGun* Gun);

	UFUNCTION()
	void HandleOnFireModeChanged(AHumanoid* Character, EFireMode NewFireMode);
	
	UFUNCTION()
	void HandleOnWeaponEquipped(AHumanoid* Player, AGun* Gun);

	void DisplayHUD();
};
