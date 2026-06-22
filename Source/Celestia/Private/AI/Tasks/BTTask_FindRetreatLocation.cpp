#include "AI/Tasks/BTTask_FindRetreatLocation.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "AIController.h"
#include "NavigationSystem.h" 
#include "GameFramework/Character.h"

UBTTask_FindRetreatLocation::UBTTask_FindRetreatLocation()
{
	NodeName = TEXT("Find Retreat Location");
}

EBTNodeResult::Type UBTTask_FindRetreatLocation::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AAIController* AIController = OwnerComp.GetAIOwner();
	UBlackboardComponent* Blackboard = OwnerComp.GetBlackboardComponent();

	if (!AIController || !Blackboard) return EBTNodeResult::Failed;

	APawn* AIPawn = AIController->GetPawn();
	AActor* TargetActor = Cast<AActor>(Blackboard->GetValueAsObject("TargetActor"));

	if (!AIPawn || !TargetActor) return EBTNodeResult::Failed;

	float DistanceToTarget = FVector::Dist(AIPawn->GetActorLocation(), TargetActor->GetActorLocation());

	if (DistanceToTarget > MinimumDistanceToRetreat)
	{
		return EBTNodeResult::Failed;
	}

	FVector DirectionAwayFromTarget = AIPawn->GetActorLocation() - TargetActor->GetActorLocation();
	DirectionAwayFromTarget.Z = 0.f;
	DirectionAwayFromTarget.Normalize();

	FVector TargetRetreatLocation = AIPawn->GetActorLocation() + (DirectionAwayFromTarget * RetreatDistance);

	UNavigationSystemV1* NavSystem = UNavigationSystemV1::GetCurrent(GetWorld());
	if (NavSystem)
	{
		FNavLocation ValidRetreatLocation;
		if (NavSystem->ProjectPointToNavigation(TargetRetreatLocation, ValidRetreatLocation, FVector(500.f, 500.f, 500.f)))
		{
			Blackboard->SetValueAsVector("RetreatLocation", ValidRetreatLocation.Location);
			return EBTNodeResult::Succeeded;
		}
	}
	return EBTNodeResult::Failed;
}	



