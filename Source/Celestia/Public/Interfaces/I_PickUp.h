// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "I_PickUp.generated.h"

UINTERFACE(MinimalAPI)
class UI_PickUp : public UInterface
{
	GENERATED_BODY()
};

class CELESTIA_API II_PickUp
{
	GENERATED_BODY()

public:

	
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Interaction")
	void Interact(AActor* Interactor);

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Interaction")
	void ReceiveItem(int32 Amount, const FString& ItemName);
};
