// Fill out your copyright notice in the Description page of Project Settings.
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/TargetPoint.h"
#include "EnemySpawner.generated.h"

class AEnemyCharacter;
class UHealthComponent;

UCLASS()
class CELESTIA_API AEnemySpawner : public AActor
{
    GENERATED_BODY()

public:
    AEnemySpawner();


    UPROPERTY(EditAnywhere, Category = "Spawner")
    TSubclassOf<AEnemyCharacter> EnemyClass;


    UPROPERTY(EditAnywhere, Category = "Spawner")
    TArray<ATargetPoint*> SpawnPoints;

    UPROPERTY(EditAnywhere, Category = "Spawner")
    TArray<ATargetPoint*> PatrolPointsToAssign;


    UPROPERTY(EditAnywhere, Category = "Spawner")
    float DefaultPatrolAcceptanceRadius = 50.f;

    UPROPERTY(EditAnywhere, Category = "Spawner")
    float DefaultPatrolWaitTime = 2.f;

    UPROPERTY(EditAnywhere, Category = "Spawner")
    float RespawnDelay = 5.f;

    UPROPERTY(EditAnywhere, Category = "Spawner")
    bool bSpawnInitially = false;

protected:
    virtual void BeginPlay() override;

    void SpawnEnemyAtIndex(int32 SpawnIndex);

    void DoRespawnByIndex(int32 SpawnIndex);

    UFUNCTION()
    void OnSpawnedEnemyDeath(AActor* DeadOwner);

    TMap<int32, TWeakObjectPtr<AEnemyCharacter>> ActiveSpawnedMap;

public:
    UFUNCTION(BlueprintCallable, Category = "Spawner")
    void SpawnAllAtBeginPlay();

    UFUNCTION(BlueprintCallable, Category = "Spawner")
    void SpawnAtIndex(int32 Index);
};