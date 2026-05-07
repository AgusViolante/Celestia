// Fill out your copyright notice in the Description page of Project Settings.
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "QuestLocationTrigger.generated.h"

class UBoxComponent;

UCLASS()
class CELESTIA_API AQuestLocationTrigger : public AActor
{
	GENERATED_BODY()

public:
	AQuestLocationTrigger();

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Quest")
	FName LocationID;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UBoxComponent* TriggerBox;

	UFUNCTION()
	void OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
};