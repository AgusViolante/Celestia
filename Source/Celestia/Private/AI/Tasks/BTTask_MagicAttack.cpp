// Fill out your copyright notice in the Description page of Project Settings.

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
	AAIController* AIController = OwnerComp.GetAIOwner();

	if (!AIController)
	{
		return EBTNodeResult::Failed;
	}

	// Declaramos el Pawn controlado de forma segura
	APawn* ControlledPawn = AIController->GetPawn();
	if (!ControlledPawn)
	{
		return EBTNodeResult::Failed;
	}

	// 1. Revisamos si es un RangedEnemy
	if (ARangedEnemy* RangedEnemy = Cast<ARangedEnemy>(ControlledPawn))
	{
		RangedEnemy->PerformRangedAttack();
		return EBTNodeResult::Succeeded;
	}

	// 2. Revisamos si es el Boss
	if (ABossEnemy* BossEnemy = Cast<ABossEnemy>(ControlledPawn))
	{
		BossEnemy->PerformRangedAttack();
		return EBTNodeResult::Succeeded;
	}

	return EBTNodeResult::Failed;
}