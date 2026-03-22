// Fill out your copyright notice in the Description page of Project Settings.

#include "AI/EnemyAIController.h"
#include "NavigationSystem.h"
#include "GameFramework/Actor.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/Pawn.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Characters/EnemyCharacter.h"
#include "Engine/TargetPoint.h"
#include "TimerManager.h"
#include "Kismet/GameplayStatics.h"

AEnemyAIController::AEnemyAIController()
{
    PrimaryActorTick.bCanEverTick = false;
}

void AEnemyAIController::OnPossess(APawn* InPawn)
{
    Super::OnPossess(InPawn);

    ControlledEnemy = Cast<AEnemyCharacter>(InPawn);
    CurrentPatrolIndex = 0;

    if (ControlledEnemy && !bIsChasing && ControlledEnemy->PatrolPoints.Num() > 0)
    {
        MoveToNextPatrolPoint();
    }
}

void AEnemyAIController::OnUnPossess()
{
    GetWorld()->GetTimerManager().ClearTimer(WaitTimerHandle);
    GetWorld()->GetTimerManager().ClearTimer(ResumePatrolTimerHandle);
    GetWorld()->GetTimerManager().ClearTimer(ChaseReissueTimerHandle);

    ControlledEnemy = nullptr;
    bIsChasing = false;
    ChaseTarget = nullptr;

    Super::OnUnPossess();
}

void AEnemyAIController::MoveToNextPatrolPoint()
{
    if (!ControlledEnemy || bIsChasing) return; 

    const TArray<ATargetPoint*>& Points = ControlledEnemy->PatrolPoints;
    if (Points.Num() == 0) return;

    ATargetPoint* Destination = Points.IsValidIndex(CurrentPatrolIndex) ? Points[CurrentPatrolIndex] : nullptr;
    if (!Destination)
    {
        CurrentPatrolIndex = (CurrentPatrolIndex + 1) % Points.Num();
        return;
    }

    MoveToActor(Destination, ControlledEnemy->PatrolAcceptanceRadius);
}

void AEnemyAIController::OnMoveCompleted(FAIRequestID RequestID, const FPathFollowingResult& Result)
{
    Super::OnMoveCompleted(RequestID, Result);


    if (bIsChasing)
    {
        if (ChaseTarget.IsValid())
        {

            if (IsWithinContactRange(ChaseTarget.Get()))
            {
                if (ControlledEnemy)
                {
                    ControlledEnemy->StartDamage(ChaseTarget.Get());
                }

            
                const float ReissueDelay = 0.12f;
                GetWorld()->GetTimerManager().ClearTimer(ChaseReissueTimerHandle);
                GetWorld()->GetTimerManager().SetTimer(ChaseReissueTimerHandle, this, &AEnemyAIController::ReissueChaseAfterDelay, ReissueDelay, false);

                UE_LOG(LogTemp, Verbose, TEXT("[AI] In contact range: started damage and scheduled reissue"));
            }
            else
            {
                const float ReissueDelay = 0.12f;
                GetWorld()->GetTimerManager().ClearTimer(ChaseReissueTimerHandle);
                GetWorld()->GetTimerManager().SetTimer(ChaseReissueTimerHandle, this, &AEnemyAIController::ReissueChaseAfterDelay, ReissueDelay, false);

                UE_LOG(LogTemp, Verbose, TEXT("[AI] OnMoveCompleted while chasing - scheduling reissue"));
            }
        }
        return;
    }

    if (!bIsChasing && ControlledEnemy)
    {
        if (Result.IsSuccess())
        {
            CurrentPatrolIndex = (CurrentPatrolIndex + 1) % ControlledEnemy->PatrolPoints.Num();
            GetWorld()->GetTimerManager().SetTimer(WaitTimerHandle, this, &AEnemyAIController::OnWaitOver, ControlledEnemy->PatrolWaitTime, false);
        }
        else
        {
            GetWorld()->GetTimerManager().SetTimer(WaitTimerHandle, this, &AEnemyAIController::OnWaitOver, 0.5f, false);
        }
    }
}

void AEnemyAIController::StartPatrol()
{
    // Si no hay pawn/controlado o si estamos en chase, no hacemos nada.
    if (!ControlledEnemy || bIsChasing) return;

    // Empezamos desde el índice actual (normalmente 0)
    if (ControlledEnemy->PatrolPoints.Num() == 0) return;

    // Si el current index está fuera de rango, lo normalizamos
    if (!ControlledEnemy->PatrolPoints.IsValidIndex(CurrentPatrolIndex))
    {
        CurrentPatrolIndex = 0;
    }

    MoveToNextPatrolPoint();
}

void AEnemyAIController::RestartPatrol()
{
    if (!ControlledEnemy || bIsChasing) return;

    CurrentPatrolIndex = 0;
    MoveToNextPatrolPoint();
}

void AEnemyAIController::OnWaitOver()
{
    if (!bIsChasing)
    {
        MoveToNextPatrolPoint();
    }
}

void AEnemyAIController::StartChasing(AActor* TargetActor, float AcceptanceRadius)
{
    if (!TargetActor || !ControlledEnemy) return;

    bIsChasing = true;
    ChaseTarget = TargetActor;


    ChaseAcceptanceRadius = FMath::Clamp(AcceptanceRadius, 0.f, 2000.f);


    GetWorld()->GetTimerManager().ClearTimer(WaitTimerHandle);


    MoveToActor(TargetActor, ChaseAcceptanceRadius, true, true, true, 0, true);

    UE_LOG(LogTemp, Log, TEXT("[AI] StartChasing %s (acceptance %.2f)"), *GetNameSafe(TargetActor), ChaseAcceptanceRadius);
}

void AEnemyAIController::StopChasing()
{
    if (!bIsChasing) return;

    bIsChasing = false;
    ChaseTarget = nullptr;

    StopMovement();


    const float DelayBeforeResume = 0.5f;
    GetWorld()->GetTimerManager().ClearTimer(ResumePatrolTimerHandle);
    GetWorld()->GetTimerManager().SetTimer(ResumePatrolTimerHandle, this, &AEnemyAIController::ResumePatrol, DelayBeforeResume, false);
}

void AEnemyAIController::ResumePatrol()
{
    if (!ControlledEnemy || bIsChasing) return;
    if (ControlledEnemy->PatrolPoints.Num() == 0) return;

    MoveToNextPatrolPoint();
}

void AEnemyAIController::ReissueChaseAfterDelay()
{
    if (!bIsChasing || !ChaseTarget.IsValid()) return;

    MoveToActor(ChaseTarget.Get(), ChaseAcceptanceRadius);
}

bool AEnemyAIController::IsWithinContactRange(AActor* Target) const
{
    if (!ControlledEnemy || !Target) return false;

    UCapsuleComponent* EnemyCapsule = ControlledEnemy->GetCapsuleComponent();
    UCapsuleComponent* TargetCapsule = Cast<APawn>(Target)->FindComponentByClass<UCapsuleComponent>();
    if (!EnemyCapsule || !TargetCapsule) return false;

    const float EnemyRadius = EnemyCapsule->GetScaledCapsuleRadius();
    const float TargetRadius = TargetCapsule->GetScaledCapsuleRadius();
    const float Distance = FVector::Dist(ControlledEnemy->GetActorLocation(), Target->GetActorLocation());
    const float Epsilon = 5.0f;
    return Distance <= (EnemyRadius + TargetRadius + Epsilon);
}