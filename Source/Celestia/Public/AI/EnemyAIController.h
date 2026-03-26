// Fill out your copyright notice in the Description page of Project Settings.
#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "Perception/AIPerceptionTypes.h" // Para el FAIStimulus
#include "EnemyAIController.generated.h"

class UBehaviorTree;
class UAIPerceptionComponent;
class UAISenseConfig_Sight;

UCLASS()
class CELESTIA_API AEnemyAIController : public AAIController
{
    GENERATED_BODY()

public:
    AEnemyAIController();

protected:
    virtual void OnPossess(APawn* InPawn) override;

    // --- EL ÁRBOL DE COMPORTAMIENTO ---
    UPROPERTY(EditDefaultsOnly, Category = "AI | Logic")
    UBehaviorTree* BehaviorTreeAsset;

    // --- LOS SENTIDOS DEL ENEMIGO ---
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI | Perception")
    UAIPerceptionComponent* EnemyPerceptionComp;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI | Perception")
    UAISenseConfig_Sight* SightConfig;

    // Función que se dispara cuando el enemigo VE o DEJA DE VER algo
    UFUNCTION()
    void OnTargetDetected(AActor* Actor, FAIStimulus Stimulus);
};