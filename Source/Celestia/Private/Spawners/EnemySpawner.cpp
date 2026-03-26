// Fill out your copyright notice in the Description page of Project Settings.

#include "Spawners/EnemySpawner.h"
#include "Characters/EnemyBase.h"
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

    // Spawneamos usando la nueva base
    AEnemyBase* NewEnemy = W->SpawnActor<AEnemyBase>(EnemyClass, SpawnTransform, Params);
    if (!NewEnemy)
    {
        UE_LOG(LogTemp, Warning, TEXT("EnemySpawner: Failed to spawn at index %d"), SpawnIndex);
        return;
    }

    // Solo le pasamos los puntos, el Behavior Tree se encargará del radio y el tiempo
    NewEnemy->PatrolPoints = PatrolPointsToAssign;
    NewEnemy->bAlreadyDied = false;

    if (NewEnemy->HealthComponent)
    {
        NewEnemy->HealthComponent->InitializeAfterSpawn(true, NewEnemy->HealthComponent->RegenDelaySeconds, NewEnemy->HealthComponent->RegenPerSecond, NewEnemy->HealthComponent->RegenTickInterval);

        NewEnemy->HealthComponent->OnDeath.RemoveDynamic(this, &AEnemySpawner::OnSpawnedEnemyDeath);
        NewEnemy->HealthComponent->OnDeath.AddDynamic(this, &AEnemySpawner::OnSpawnedEnemyDeath);
    }

    // El cerebro arranca solo en el OnPossess del controlador, ya no llamamos a StartPatrol
    NewEnemy->SpawnDefaultController();

    EnemyToSpawnIndexMap.Add(NewEnemy, SpawnIndex);
}

void AEnemySpawner::OnSpawnedEnemyDeath(AActor* DeadOwner)
{
    AEnemyBase* DeadEnemy = Cast<AEnemyBase>(DeadOwner);
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

