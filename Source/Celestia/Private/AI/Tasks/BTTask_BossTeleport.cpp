// Fill out your copyright notice in the Description page of Project Settings.

#include "AI/Tasks/BTTask_BossTeleport.h"
#include "AIController.h"
#include "Characters/BossEnemy.h"

UBTTask_BossTeleport::UBTTask_BossTeleport() { NodeName = TEXT("Boss Teleport Retreat"); }

EBTNodeResult::Type UBTTask_BossTeleport::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AAIController* AICon = OwnerComp.GetAIOwner();
	if (!AICon) return EBTNodeResult::Failed;

	if (ABossEnemy* Boss = Cast<ABossEnemy>(AICon->GetPawn()))
	{
		Boss->TeleportAway();
		return EBTNodeResult::Succeeded;
	}
	return EBTNodeResult::Failed;
}

