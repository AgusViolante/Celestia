// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "StunnableInterface.generated.h"

UINTERFACE(MinimalAPI)
class UStunnableInterface : public UInterface
{
	GENERATED_BODY()
};

class CELESTIA_API IStunnableInterface
{
	GENERATED_BODY()

public:
	

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Combat | Stun")
	void ApplyStun(float Duration);
};