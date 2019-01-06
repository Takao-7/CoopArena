#include "GameModes/Deathmatch.h"
#include "Humanoid.h"
#include "GameplayTagContainer.h"


ADeathmatch::ADeathmatch()
{
	m_CurrentTeamIndex = 1;
}

/////////////////////////////////////////////////////
void ADeathmatch::SetTeamName(AController* Controller)
{
	FString teamTag = CheckForTeamTag(*Controller);

	if (teamTag.IsEmpty())
	{
		teamTag = "Team" + FString::FromInt(m_CurrentTeamIndex++);
		Controller->Tags.AddUnique(FName(*teamTag));
	}
	
	AHumanoid* humanoid = Cast<AHumanoid>(Controller->GetPawn());
	if (humanoid)
	{
		humanoid->SetTeamName(teamTag);
	}
}