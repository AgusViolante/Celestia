// Fill out your copyright notice in the Description page of Project Settings.
#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "Navigation/PathFollowingComponent.h"
#include "EnemyAIController.generated.h"

class AEnemyCharacter;
class ATargetPoint;


UCLASS()
class CELESTIA_API AEnemyAIController : public AAIController
{
    GENERATED_BODY()

public:
    AEnemyAIController();

    virtual void OnPossess(APawn* InPawn) override;
    virtual void OnUnPossess() override;

public:
    UFUNCTION(BlueprintCallable)
    void StartPatrol();

    /** Forzar a reanudar la patrulla (útil si querés reiniciar índice o forzar movimiento) */
    UFUNCTION(BlueprintCallable)
    void RestartPatrol();

    UFUNCTION(BlueprintCallable, Category = "AI")
    void StartChasing(AActor* TargetActor, float AcceptanceRadius = 50.f);

    UFUNCTION(BlueprintCallable, Category = "AI")
    void StopChasing();

protected:

    virtual void OnMoveCompleted(FAIRequestID RequestID, const FPathFollowingResult& Result) override;

private:

    AEnemyCharacter* ControlledEnemy = nullptr;

    int32 CurrentPatrolIndex = 0;

    FTimerHandle WaitTimerHandle;

    FTimerHandle ResumePatrolTimerHandle;

    FTimerHandle ChaseReissueTimerHandle;

    void MoveToNextPatrolPoint();

    void OnWaitOver();

    bool bIsChasing = false;

    bool IsWithinContactRange(AActor* Target) const;

    TWeakObjectPtr<AActor> ChaseTarget;

    float ChaseAcceptanceRadius = 50.f;

    void ResumePatrol();

    void ReissueChaseAfterDelay();

};