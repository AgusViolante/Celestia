#pragma once
#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_BossLeap.generated.h"

UCLASS()
class CELESTIA_API UBTTask_BossLeap : public UBTTaskNode
{
	GENERATED_BODY()
public:
	UBTTask_BossLeap();
protected:
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
};