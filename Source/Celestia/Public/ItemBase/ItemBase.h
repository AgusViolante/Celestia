// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interfaces/I_PickUp.h" 
#include "ItemBase.generated.h"

class UBoxComponent;
class UStaticMeshComponent;

UCLASS()
class CELESTIA_API AItemBase : public AActor, public II_PickUp
{
	GENERATED_BODY()

public:
	AItemBase();

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item Data")
	int32 Amount = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item Data")
	FString ItemName = "BaseItem";

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UBoxComponent> BoxCollision;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UStaticMeshComponent> CubeMesh;

	UPROPERTY()
	bool bIsPickedUp = false;

public:
	// Implementación de la orden de interactuar que manda el jugador
	virtual void Interact_Implementation(AActor* Interactor) override;
};