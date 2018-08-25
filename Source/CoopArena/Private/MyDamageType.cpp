// Fill out your copyright notice in the Description page of Project Settings.

#include "MyDamageType.h"
#include "MyPhysicalMaterial.h"
#include "Particles/ParticleSystem.h"


UParticleSystem* UMyDamageType::GetHitEffect(EPhysicalSurface Surface) const
{
	UParticleSystem*const* pointer = _HitEffects.Find(Surface);	
	if (pointer == nullptr)
	{
		return nullptr;
	}
	else
	{
		return *pointer;
	}
}
