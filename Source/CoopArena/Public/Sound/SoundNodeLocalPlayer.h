// Fill out your copyright notice in the Description page of Project Settings.

#pragma once
#include "CoreMinimal.h"
#include "Sound/SoundNode.h"
#include "SoundNodeLocalPlayer.generated.h"

/**
* Choose different branch for sounds attached to locally controlled player
* Originates from ShooterGame project by Epic Games.
*/
UCLASS(hidecategories = Object, editinlinenew)
class COOPARENA_API USoundNodeLocalPlayer : public USoundNode
{
	GENERATED_BODY()
	
	virtual void ParseNodes(FAudioDevice* AudioDevice, const UPTRINT NodeWaveInstanceHash, FActiveSound& ActiveSound, const FSoundParseParameters& ParseParams, TArray<FWaveInstance*>& WaveInstances) override;

	virtual void CreateStartingConnectors() override;

	virtual int32 GetMaxChildNodes() const override;

	virtual int32 GetMinChildNodes() const override;

#if WITH_EDITOR
	virtual FText GetInputPinName(int32 PinIndex) const override;
#endif

public:
	static TMap<uint32, bool>& GetLocallyControlledActorCache()
	{
		check(IsInAudioThread());
		return LocallyControlledActorCache;
	}

private:
	static TMap<uint32, bool> LocallyControlledActorCache;
};