#pragma once
#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_MagicSpread.generated.h"

UCLASS()
class CELESTIA_API UBTTask_MagicSpread : public UBTTaskNode
{
	GENERATED_BODY()
public:
	UBTTask_MagicSpread();
protected:
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
};