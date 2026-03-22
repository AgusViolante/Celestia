// Fill out your copyright notice in the Description page of Project Settings.

#include "Spawners/EnemySpawner.h"
#include "Characters/EnemyCharacter.h"
#include "AI/EnemyAIController.h"
#include "Components/HealthComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Engine/Engine.h"

AEnemySpawner::AEnemySpawner()
{
    PrimaryActorTick.bCanEverTick = false;
}

void AEnemySpawner::BeginPlay()
{
    Super::BeginPlay();

    if (bSpawnInitially)
    {
        SpawnAllAtBeginPlay();
    }
}

void AEnemySpawner::SpawnAllAtBeginPlay()
{
    for (int32 i = 0; i < SpawnPoints.Num(); ++i)
    {
        if (SpawnPoints[i])
        {
            SpawnEnemyAtIndex(i);
        }
    }
}

void AEnemySpawner::SpawnAtIndex(int32 Index)
{
    if (!SpawnPoints.IsValidIndex(Index)) return;
    SpawnEnemyAtIndex(Index);
}

void AEnemySpawner::SpawnEnemyAtIndex(int32 SpawnIndex)
{
    if (!EnemyClass) return;
    if (!SpawnPoints.IsValidIndex(SpawnIndex)) return;

    UWorld* W = GetWorld();
    if (!W) return;

    const FTransform SpawnTransform = SpawnPoints[SpawnIndex]->GetActorTransform();

    FActorSpawnParameters Params;
    Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

    AEnemyCharacter* NewEnemy = W->SpawnActor<AEnemyCharacter>(EnemyClass, SpawnTransform, Params);
    if (!NewEnemy)
    {
        UE_LOG(LogTemp, Warning, TEXT("EnemySpawner: Failed to spawn at index %d"), SpawnIndex);
        return;
    }


    NewEnemy->PatrolPoints = PatrolPointsToAssign;
    NewEnemy->PatrolAcceptanceRadius = DefaultPatrolAcceptanceRadius;
    NewEnemy->PatrolWaitTime = DefaultPatrolWaitTime;


    NewEnemy->bAlreadyDied = false;


    if (NewEnemy->HealthComponent)
    {
       
        NewEnemy->HealthComponent->InitializeAfterSpawn(true, NewEnemy->HealthComponent->RegenDelaySeconds, NewEnemy->HealthComponent->RegenPerSecond, NewEnemy->HealthComponent->RegenTickInterval);

        NewEnemy->HealthComponent->OnDeath.RemoveDynamic(this, &AEnemySpawner::OnSpawnedEnemyDeath);
        NewEnemy->HealthComponent->OnDeath.AddDynamic(this, &AEnemySpawner::OnSpawnedEnemyDeath);
    }

    if (NewEnemy && NewEnemy->HealthComponent)
    {
        UE_LOG(LogTemp, Log, TEXT("Spawner: spawned %s. RegenTimerActive=%d bAutoRegen=%d Delay=%.2f"),
            *GetNameSafe(NewEnemy),
            NewEnemy->HealthComponent->IsRegenTimerActive() ? 1 : 0,
            NewEnemy->HealthComponent->bAutoRegen ? 1 : 0,
            NewEnemy->HealthComponent->RegenDelaySeconds);
    }

    NewEnemy->SpawnDefaultController();
    if (AController* C = NewEnemy->GetController())
    {
        if (AEnemyAIController* AICon = Cast<AEnemyAIController>(C))
        {
            AICon->StartPatrol();
        }
    }


    EnemyToSpawnIndexMap.Add(NewEnemy, SpawnIndex);
}

void AEnemySpawner::OnSpawnedEnemyDeath(AActor* DeadOwner)
{
    AEnemyCharacter* DeadEnemy = Cast<AEnemyCharacter>(DeadOwner);
    if (!DeadEnemy) return;

    int32* FoundIndexPtr = EnemyToSpawnIndexMap.Find(DeadEnemy);
    if (!FoundIndexPtr)
    {
        return;
    }

    int32 FoundIndex = *FoundIndexPtr;
    EnemyToSpawnIndexMap.Remove(DeadEnemy);

    if (UWorld* W = GetWorld())
    {
        FTimerDelegate Del;
        Del.BindUObject(this, &AEnemySpawner::DoRespawnByIndex, FoundIndex);

        FTimerHandle Handle;
        W->GetTimerManager().SetTimer(Handle, Del, RespawnDelay, false);
    }
    else
    {
        DoRespawnByIndex(FoundIndex);
    }
}


void AEnemySpawner::DoRespawnByIndex(int32 SpawnIndex)
{
    if (!SpawnPoints.IsValidIndex(SpawnIndex)) return;

    SpawnEnemyAtIndex(SpawnIndex);
}

