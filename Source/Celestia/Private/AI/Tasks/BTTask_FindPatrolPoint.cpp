// Fill out your copyright notice in the Description page of Project Settings.

#include "AI/Tasks/BTTask_FindPatrolPoint.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "AIController.h"
#include "Characters/EnemyBase.h"
#include "Engine/TargetPoint.h"

UBTTask_FindPatrolPoint::UBTTask_FindPatrolPoint()
{
	// Este es el nombre que verás en el editor
	NodeName = TEXT("Find Next Patrol Point");
}

EBTNodeResult::Type UBTTask_FindPatrolPoint::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AAIController* AICon = OwnerComp.GetAIOwner();
	UBlackboardComponent* Blackboard = OwnerComp.GetBlackboardComponent();

	if (!AICon || !Blackboard) return EBTNodeResult::Failed;

	AEnemyBase* Enemy = Cast<AEnemyBase>(AICon->GetPawn());
	if (!Enemy || Enemy->PatrolPoints.Num() == 0) return EBTNodeResult::Failed;

	// Leemos en qué índice de la patrulla vamos (empieza en 0 por defecto)
	int32 CurrentIndex = Blackboard->GetValueAsInt(FName("PatrolIndex"));

	// Si el índice se salió de rango por algún motivo, lo reseteamos
	if (!Enemy->PatrolPoints.IsValidIndex(CurrentIndex))
	{
		CurrentIndex = 0;
	}

	ATargetPoint* TargetPoint = Enemy->PatrolPoints[CurrentIndex];
	if (TargetPoint)
	{
		// 1. Guardamos la ubicación de ese punto en el Blackboard
		Blackboard->SetValueAsVector(FName("PatrolLocation"), TargetPoint->GetActorLocation());

		// 2. Calculamos el siguiente índice y lo guardamos para la próxima vez
		int32 NextIndex = (CurrentIndex + 1) % Enemy->PatrolPoints.Num();
		Blackboard->SetValueAsInt(FName("PatrolIndex"), NextIndex);

		return EBTNodeResult::Succeeded;
	}

	return EBTNodeResult::Failed;
}