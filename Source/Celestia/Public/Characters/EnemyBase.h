// Fill out your copyright notice in the Description page of Project Settings.
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Interfaces/DeathInterface.h"
#include "Interfaces/StunnableInterface.h"
#include "EnemyTypes.h"
#include "EnemyBase.generated.h"

class UHealthComponent;
class UStatsComponent;
class UAnimMontage;
class ATargetPoint;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FEnemyDeathSignature, AEnemyBase*, Enemy);

UCLASS()
class CELESTIA_API AEnemyBase : public ACharacter, public IDeathInterface, public IStunnableInterface
{
	GENERATED_BODY()

public:
	AEnemyBase();

	// --- COMPONENTES BASE ---
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UHealthComponent* HealthComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UStatsComponent* StatsComponent;

	// --- EVENTOS Y ESTADO ---
	UPROPERTY(BlueprintAssignable, Category = "Enemy | Events")
	FEnemyDeathSignature OnDeath;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Enemy | State")
	bool bAlreadyDied = false;

	// --- ANIMACIONES BASE ---
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy | Animation")
	UAnimMontage* Death_A_Montage;

	// --- SISTEMA DE LOOT ---
	UPROPERTY(EditDefaultsOnly, Category = "Enemy | Drops")
	TSubclassOf<AActor> PotionDropClass;

	UPROPERTY(EditDefaultsOnly, Category = "Enemy | Drops", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float DropChance = 1.0f;

	virtual void Die_Implementation() override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI | Patrol")
	TArray<ATargetPoint*> PatrolPoints;

	virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser) override;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Enemy | State")
	EEnemyClassType EnemyType;

	// --- SISTEMA DE EXPERIENCIA ---
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy | Progression", meta = (ExposeOnSpawn = "true"))
	int32 EnemyLevel = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy | Progression")
	float BaseXPReward = 50.0f;

	UFUNCTION(BlueprintCallable, Category = "Enemy | Progression")
	float CalculateXPReward() const;

	UFUNCTION(BlueprintCallable, Category = "Enemy | Stats")
	void InitializeEnemyStats();

	virtual void ApplyStun_Implementation(float Duration) override;

protected:
	virtual void BeginPlay() override;

	UFUNCTION()
	virtual void OnHealthComponentDeath(AActor* DeadOwner);

	UFUNCTION()
	void SpawnPotionDrop();

	void ReleaseStun();

	FTimerHandle StunTimerHandle;

	bool bIsStunned = false;
	UPROPERTY(EditAnywhere, Category = "Combat | Stun")
	class UNiagaraSystem* StunVFX;

	UPROPERTY()
	class UNiagaraComponent* ActiveStunVFX;

	UPROPERTY(EditAnywhere, Category = "Combat | Stun")
	float StunVFXHeightOffset = 100.0f;
};