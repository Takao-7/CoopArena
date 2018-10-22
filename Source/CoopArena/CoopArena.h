// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

/* Line trace channels */

#define ECC_Projectile				ECC_GameTraceChannel1
/* Used to find intractable actors. Actors that are interactables have*/
#define ECC_Interactable			ECC_GameTraceChannel2
/* Used to find actors or meshes that can be damaged or passed through by projectiles. */
#define ECC_ProjectilePenetration	ECC_GameTraceChannel3

/* Surface types */

#define SurfaceType_Flesh			SurfaceType1
#define SurfaceType_Steel			SurfaceType2
#define SurfaceType_Stone			SurfaceType3
#define SurfaceType_Wood			SurfaceType4
#define SurfaceType_Dirt			SurfaceType5