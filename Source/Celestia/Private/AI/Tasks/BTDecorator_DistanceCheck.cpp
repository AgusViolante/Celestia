#include "AI/Tasks/BTDecorator_DistanceCheck.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"

UBTDecorator_DistanceCheck::UBTDecorator_DistanceCheck()
{
	NodeName = "Custom Distance Check";
}

bool UBTDecorator_DistanceCheck::CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const
{
	AAIController* AICon = OwnerComp.GetAIOwner();
	UBlackboardComponent* BB = OwnerComp.GetBlackboardComponent();

	if (!AICon || !BB) return false;

	AActor* Target = Cast<AActor>(BB->GetValueAsObject(GetSelectedBlackboardKey()));
	if (!Target) return false;

	float Distance = FVector::Dist(AICon->GetPawn()->GetActorLocation(), Target->GetActorLocation());
	return bCheckGreater ? (Distance > DistanceThreshold) : (Distance < DistanceThreshold);
}