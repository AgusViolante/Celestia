#pragma once
#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_MagicAttack.generated.h"

UCLASS()
class CELESTIA_API UBTTask_MagicAttack : public UBTTaskNode
{
	GENERATED_BODY()
public:
	UBTTask_MagicAttack();
protected:
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
};