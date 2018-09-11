#include "AdequateAnimationSystem.h"


UAdequateAnimationSystem::UAdequateAnimationSystem()
{
	PrimaryComponentTick.bCanEverTick = true;

}


void UAdequateAnimationSystem::BeginPlay()
{
	Super::BeginPlay();

	
}


void UAdequateAnimationSystem::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

}

