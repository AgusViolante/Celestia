// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ItemBase/Chest.h"
#include "LockedChest.generated.h"

UCLASS()
class CELESTIA_API ALockedChest : public AChest
{
	GENERATED_BODY()

public:
	ALockedChest();

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chest | Lock Settings")
	FString RequiredKeyName;

	UFUNCTION(BlueprintImplementableEvent, Category = "Chest | Lock Settings")
	bool CheckPlayerHasKey(AActor* Interactor, const FString& KeyName);

	// Función para que Blueprint elimine la llave del inventario
	UFUNCTION(BlueprintImplementableEvent, Category = "Chest | Lock Settings")
	void ConsumePlayerKey(AActor* Interactor, const FString& KeyName);

public:
	virtual void Interact_Implementation(AActor* Interactor) override;
};