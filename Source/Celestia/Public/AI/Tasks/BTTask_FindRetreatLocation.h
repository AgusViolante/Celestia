// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_FindRetreatLocation.generated.h"

/**
 * 
 */
UCLASS()
class CELESTIA_API UBTTask_FindRetreatLocation : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UBTTask_FindRetreatLocation();

protected:
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

	UPROPERTY(EditAnywhere, Category = "Retreat Logic")
	float MinimumDistanceToRetreat = 400.0f;

	UPROPERTY(EditAnywhere, Category = "Retreat Logic")
	float RetreatDistance = 600.0f;
};