#include "AI/Tasks/BTTask_MagicSpread.h"
#include "AIController.h"
#include "Characters/BossEnemy.h"

UBTTask_MagicSpread::UBTTask_MagicSpread()
{
	NodeName = TEXT("Boss Magic Spread Attack");
}

EBTNodeResult::Type UBTTask_MagicSpread::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AAIController* AIController = OwnerComp.GetAIOwner();
	if (!AIController)
	{
		return EBTNodeResult::Failed;
	}

	APawn* ControlledPawn = AIController->GetPawn();
	if (!ControlledPawn)
	{
		return EBTNodeResult::Failed;
	}

	if (ABossEnemy* BossEnemy = Cast<ABossEnemy>(ControlledPawn))
	{
		BossEnemy->FireMagicSpread();
		return EBTNodeResult::Succeeded;
	}

	return EBTNodeResult::Failed;
}