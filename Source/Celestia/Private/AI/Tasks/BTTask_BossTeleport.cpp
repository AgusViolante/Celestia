#include "AI/Tasks/BTTask_BossTeleport.h"
#include "AIController.h"
#include "Characters/BossEnemy.h"

UBTTask_BossTeleport::UBTTask_BossTeleport() { NodeName = TEXT("Boss Teleport Retreat"); }

EBTNodeResult::Type UBTTask_BossTeleport::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	if (AAIController* AICon = OwnerComp.GetAIOwner())
	{
		if (ABossEnemy* Boss = Cast<ABossEnemy>(AICon->GetPawn()))
		{
			Boss->TeleportAway();
			return EBTNodeResult::Succeeded;
		}
	}
	return EBTNodeResult::Failed;
}