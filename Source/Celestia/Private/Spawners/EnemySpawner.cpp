#include "Spawners/EnemySpawner.h"
#include "Characters/EnemyBase.h"
#include "AI/EnemyAIController.h"
#include "Components/HealthComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Engine/Engine.h"
#include "Engine/TargetPoint.h"

AEnemySpawner::AEnemySpawner()
{
    PrimaryActorTick.bCanEverTick = false;
    bReplicates = false;
}

void AEnemySpawner::BeginPlay()
{
    Super::BeginPlay();

    if (!HasAuthority()) return;

    RespawnTimerHandles.Init(FTimerHandle(), SpawnPoints.Num());

    if (bSpawnInitially)
    {
        SpawnAllAtBeginPlay();
    }
    else
    {
        for (int32 i = 0; i < SpawnPoints.Num(); ++i)
        {
            if (SpawnPoints[i])
            {
                FTimerDelegate Del;
                Del.BindUFunction(this, FName("DoRespawnByIndex"), i);

                if (UWorld* W = GetWorld())
                {
                    W->GetTimerManager().SetTimer(RespawnTimerHandles[i], Del, RespawnDelay, false);
                }
            }
        }
    }
}

void AEnemySpawner::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    if (HasAuthority())
    {
        if (UWorld* W = GetWorld())
        {
            W->GetTimerManager().ClearAllTimersForObject(this);
        }
    }
    Super::EndPlay(EndPlayReason);
}

void AEnemySpawner::SpawnAllAtBeginPlay()
{
    if (!HasAuthority()) return;

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
    if (!HasAuthority()) return;

    if (!SpawnPoints.IsValidIndex(Index)) return;
    SpawnEnemyAtIndex(Index);
}

void AEnemySpawner::SpawnEnemyAtIndex(int32 SpawnIndex)
{
    if (!HasAuthority() || !EnemyClass) return;
    if (!SpawnPoints.IsValidIndex(SpawnIndex) || !SpawnPoints[SpawnIndex]) return;

    UWorld* W = GetWorld();
    if (!W) return;

    const FTransform SpawnTransform = SpawnPoints[SpawnIndex]->GetActorTransform();

    AEnemyBase* NewEnemy = W->SpawnActorDeferred<AEnemyBase>(
        EnemyClass,
        SpawnTransform,
        this,
        nullptr,
        ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn
    );

    if (!NewEnemy)
    {
        return;
    }

    NewEnemy->EnemyLevel = LevelToSpawn;
    NewEnemy->PatrolPoints = PatrolPointsToAssign;
    NewEnemy->bAlreadyDied = false;

    UGameplayStatics::FinishSpawningActor(NewEnemy, SpawnTransform);

    if (NewEnemy->HealthComponent)
    {
        NewEnemy->HealthComponent->OnDeath.AddUniqueDynamic(this, &AEnemySpawner::OnSpawnedEnemyDeath);
    }

    NewEnemy->SpawnDefaultController();

    EnemyToSpawnIndexMap.Add(NewEnemy, SpawnIndex);
}

void AEnemySpawner::OnSpawnedEnemyDeath(AActor* DeadOwner)
{
    if (!HasAuthority()) return;

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
        Del.BindUFunction(this, FName("DoRespawnByIndex"), FoundIndex);

        W->GetTimerManager().SetTimer(RespawnTimerHandles[FoundIndex], Del, RespawnDelay, false);
    }
    else
    {
        DoRespawnByIndex(FoundIndex);
    }
}

void AEnemySpawner::DoRespawnByIndex(int32 SpawnIndex)
{
    if (!HasAuthority()) return;

    if (!SpawnPoints.IsValidIndex(SpawnIndex)) return;

    if (UWorld* W = GetWorld())
    {
        W->GetTimerManager().ClearTimer(RespawnTimerHandles[SpawnIndex]);
    }

    SpawnEnemyAtIndex(SpawnIndex);
}