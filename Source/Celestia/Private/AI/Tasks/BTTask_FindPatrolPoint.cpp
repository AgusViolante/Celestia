#include "AI/Tasks/BTTask_FindPatrolPoint.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "AIController.h"
#include "Characters/EnemyBase.h"
#include "Engine/TargetPoint.h"

UBTTask_FindPatrolPoint::UBTTask_FindPatrolPoint()
{
	NodeName = TEXT("Find Next Patrol Point");
}

EBTNodeResult::Type UBTTask_FindPatrolPoint::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AAIController* AICon = OwnerComp.GetAIOwner();
	UBlackboardComponent* Blackboard = OwnerComp.GetBlackboardComponent();

	if (!AICon || !Blackboard) return EBTNodeResult::Failed;

	AEnemyBase* Enemy = Cast<AEnemyBase>(AICon->GetPawn());
	if (!Enemy || Enemy->PatrolPoints.Num() == 0) return EBTNodeResult::Failed;

	int32 CurrentIndex = Blackboard->GetValueAsInt(FName("PatrolIndex"));

	if (!Enemy->PatrolPoints.IsValidIndex(CurrentIndex))
	{
		CurrentIndex = 0;
	}

	if (ATargetPoint* TargetPoint = Enemy->PatrolPoints[CurrentIndex])
	{
		Blackboard->SetValueAsVector(FName("PatrolLocation"), TargetPoint->GetActorLocation());

		int32 NextIndex = (CurrentIndex + 1) % Enemy->PatrolPoints.Num();
		Blackboard->SetValueAsInt(FName("PatrolIndex"), NextIndex);

		return EBTNodeResult::Succeeded;
	}
	return EBTNodeResult::Failed;
}