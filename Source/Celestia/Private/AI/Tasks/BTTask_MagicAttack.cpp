#include "AI/Tasks/BTTask_MagicAttack.h"
#include "AI/EnemyAIController.h"
#include "Characters/RangedEnemy/RangedEnemy.h"
#include "Characters/BossEnemy.h" 

UBTTask_MagicAttack::UBTTask_MagicAttack()
{
	NodeName = TEXT("Magic Attack");
}

EBTNodeResult::Type UBTTask_MagicAttack::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	if (AAIController* AIController = OwnerComp.GetAIOwner())
	{
		if (APawn* ControlledPawn = AIController->GetPawn())
		{
			if (ARangedEnemy* RangedEnemy = Cast<ARangedEnemy>(ControlledPawn))
			{
				RangedEnemy->PerformRangedAttack();
				return EBTNodeResult::Succeeded;
			}
			if (ABossEnemy* BossEnemy = Cast<ABossEnemy>(ControlledPawn))
			{
				BossEnemy->PerformRangedAttack();
				return EBTNodeResult::Succeeded;
			}
		}
	}
	return EBTNodeResult::Failed;
}