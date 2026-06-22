#include "AI/Tasks/BTTask_BossLeap.h"
#include "AIController.h"
#include "Characters/BossEnemy.h"

UBTTask_BossLeap::UBTTask_BossLeap() { NodeName = TEXT("Boss Leap to Player"); }

EBTNodeResult::Type UBTTask_BossLeap::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	if (AAIController* AICon = OwnerComp.GetAIOwner())
	{
		if (ABossEnemy* Boss = Cast<ABossEnemy>(AICon->GetPawn()))
		{
			Boss->LeapTowardsPlayer();
			return EBTNodeResult::Succeeded;
		}
	}
	return EBTNodeResult::Failed;
}