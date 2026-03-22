// Fill out your copyright notice in the Description page of Project Settings.
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Engine/TargetPoint.h"
#include "Interfaces/DeathInterface.h"
#include "EnemyCharacter.generated.h"


class UHealthComponent;
class USphereComponent;
class UAnimMontage;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FEnemyDeathSignature, AEnemyCharacter*, Enemy);

UCLASS()
class CELESTIA_API AEnemyCharacter : public ACharacter, public IDeathInterface
{
    GENERATED_BODY()

public:
    AEnemyCharacter();


    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI|Patrol")
    TArray<ATargetPoint*> PatrolPoints;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI|Patrol")
    float PatrolAcceptanceRadius = 50.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI|Patrol")
    float PatrolWaitTime = 2.f;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Health", meta = (AllowPrivateAccess = "true"))
    UHealthComponent* HealthComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
    USphereComponent* DetectionSphere;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
    AActor* CurrentTarget;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat")
    USphereComponent* DamageSphere;

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void StartDamage(AActor* TargetActor);

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void StopDamage();

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Animation")
    UAnimMontage* Death_A_Montage;


    virtual void Die_Implementation() override;

    UFUNCTION(BlueprintCallable, Category = "AI")
    void InitializeFromSpawner(const TArray<ATargetPoint*>& InPatrolPoints, float InAcceptanceRadius, float InWaitTime);

    UPROPERTY(BlueprintAssignable, Category = "Enemy")
    FEnemyDeathSignature OnDeath;

    UPROPERTY()
    bool bAlreadyDied = false;


    UPROPERTY(EditDefaultsOnly, Category = "Drops")
    TSubclassOf<AActor> PotionDropClass;

    // Probabilidad (0.0 - 1.0) de soltar la pocion al morir
    UPROPERTY(EditDefaultsOnly, Category = "Drops", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float DropChance = 1.0f;

protected:
    virtual void BeginPlay() override;

    UFUNCTION()
    void OnDetectionBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
        UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

    UFUNCTION()
    void OnDetectionEndOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
        UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

    UFUNCTION()
    void OnHitPlayerBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
        UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

    UFUNCTION()
    void OnHitPlayerEndOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
        UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float DamagePerSecond = 10.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float DamageInterval = 1.f;

    UFUNCTION()
    void OnHealthComponentDeath(AActor* DeadOwner);

    UFUNCTION()
    void SpawnPotionDrop();


private:
    void ApplyDamage();

    AActor* DamageTarget = nullptr;

    FTimerHandle DamageTimerHandle;

public:
    virtual void Tick(float DeltaTime) override;
    virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

};