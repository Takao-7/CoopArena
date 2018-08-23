// Fill out your copyright notice in the Description page of Project Settings.

#include "MyDamageType.h"
#include "MyPhysicalMaterial.h"
#include "Particles/ParticleSystem.h"


float UMyDamageType::GetDamageModAgainstMaterial(UMyPhysicalMaterial* Material)
{
	return 1.0;
}


UParticleSystem* UMyDamageType::GetHitEffect(EPhysicalSurface Surface) const
{
	return *_HitEffects.Find(Surface);
}
