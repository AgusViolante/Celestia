#pragma once
#include "CoreMinimal.h"
#include "BehaviorTree/Decorators/BTDecorator_BlackboardBase.h"
#include "BTDecorator_DistanceCheck.generated.h"

UCLASS()
class CELESTIA_API UBTDecorator_DistanceCheck : public UBTDecorator_BlackboardBase
{
	GENERATED_BODY()
public:
	UBTDecorator_DistanceCheck();

	UPROPERTY(EditAnywhere, Category = "Condition")
	float DistanceThreshold = 800.f;

	UPROPERTY(EditAnywhere, Category = "Condition")
	bool bCheckGreater = true;
protected:
	virtual bool CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const override;
};