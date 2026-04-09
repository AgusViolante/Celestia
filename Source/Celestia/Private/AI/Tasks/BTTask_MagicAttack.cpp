// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/Tasks/BTTask_MagicAttack.h"
#include "AI/EnemyAIController.h"
#include "Characters/RangedEnemy/RangedEnemy.h"

UBTTask_MagicAttack::UBTTask_MagicAttack()
{
	
	NodeName = TEXT("Magic Attack");
}

EBTNodeResult::Type UBTTask_MagicAttack::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	
	AAIController* AIController = OwnerComp.GetAIOwner();
	if (AIController)
	{
		
		ARangedEnemy* RangedEnemy = Cast<ARangedEnemy>(AIController->GetPawn());

		if (RangedEnemy)
		{
			
			RangedEnemy->PerformRangedAttack();

			
			return EBTNodeResult::Succeeded;
		}
	}

	return EBTNodeResult::Failed;
}