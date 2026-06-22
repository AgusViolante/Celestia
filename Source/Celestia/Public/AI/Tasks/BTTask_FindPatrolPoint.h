#pragma once
#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_FindPatrolPoint.generated.h"

UCLASS()
class CELESTIA_API UBTTask_FindPatrolPoint : public UBTTaskNode
{
	GENERATED_BODY()
public:
	UBTTask_FindPatrolPoint();
protected:
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
};