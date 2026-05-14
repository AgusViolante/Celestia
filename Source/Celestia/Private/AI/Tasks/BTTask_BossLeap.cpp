// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/Tasks/BTTask_BossLeap.h"
#include "AIController.h"
#include "Characters/BossEnemy.h"

UBTTask_BossLeap::UBTTask_BossLeap() { NodeName = TEXT("Boss Leap to Player"); }

EBTNodeResult::Type UBTTask_BossLeap::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AAIController* AICon = OwnerComp.GetAIOwner();
	if (!AICon) return EBTNodeResult::Failed;

	if (ABossEnemy* Boss = Cast<ABossEnemy>(AICon->GetPawn()))
	{
		Boss->LeapTowardsPlayer();
		return EBTNodeResult::Succeeded;
	}
	return EBTNodeResult::Failed;
}

