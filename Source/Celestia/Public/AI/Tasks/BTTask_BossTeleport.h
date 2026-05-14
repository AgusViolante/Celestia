// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_BossTeleport.generated.h"

UCLASS()
class CELESTIA_API UBTTask_BossTeleport : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UBTTask_BossTeleport();

protected:
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
};