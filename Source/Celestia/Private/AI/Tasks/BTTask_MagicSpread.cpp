#include "AI/Tasks/BTTask_MagicSpread.h"
#include "AIController.h"
#include "Characters/BossEnemy.h"

UBTTask_MagicSpread::UBTTask_MagicSpread()
{
	NodeName = TEXT("Boss Magic Spread Attack");
}

EBTNodeResult::Type UBTTask_MagicSpread::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	if (AAIController* AIController = OwnerComp.GetAIOwner())
	{
		if (ABossEnemy* BossEnemy = Cast<ABossEnemy>(AIController->GetPawn()))
		{
			BossEnemy->FireMagicSpread();
			return EBTNodeResult::Succeeded;
		}
	}
	return EBTNodeResult::Failed;
}