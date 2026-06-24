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
class UNiagaraSystem;
class UNiagaraComponent;
class UWidgetComponent;
class AEnemyBase;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FEnemyDeathSignature, AEnemyBase*, Enemy);

UCLASS()
class CELESTIA_API AEnemyBase : public ACharacter, public IDeathInterface, public IStunnableInterface
{
	GENERATED_BODY()

public:
	typedef AEnemyBase ThisClass;

	AEnemyBase();

	virtual void Tick(float DeltaTime) override;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UHealthComponent> HealthComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UStatsComponent> StatsComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "UI")
	TObjectPtr<UWidgetComponent> OverheadWidgetComp;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy | Info")
	FText EnemyName;

	UPROPERTY(BlueprintAssignable, Category = "Enemy | Events")
	FEnemyDeathSignature OnDeath;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, ReplicatedUsing = OnRep_AlreadyDied, Category = "Enemy | State")
	bool bAlreadyDied = false;

	UFUNCTION()
	void OnRep_AlreadyDied();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy | Animation")
	TObjectPtr<UAnimMontage> Death_A_Montage;

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

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy | Progression", meta = (ExposeOnSpawn = "true"))
	int32 EnemyLevel = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy | Progression")
	float BaseXPReward = 50.0f;

	UFUNCTION(BlueprintCallable, Category = "Enemy | Progression")
	float CalculateXPReward() const;

	UFUNCTION(BlueprintCallable, Category = "Enemy | Stats")
	void InitializeEnemyStats();

	virtual void ApplyStun_Implementation(float Duration) override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quests")
	FName EnemyQuestID;

	UFUNCTION(BlueprintCallable, Category = "AI")
	AActor* GetAITarget() const;

protected:
	virtual void BeginPlay() override;

	UFUNCTION()
	virtual void OnHealthComponentDeath(AActor* DeadOwner);

	UFUNCTION()
	void OnHealthChangedUpdateUI(UHealthComponent* InHealthComp, float NewHealth, float MaxHealth, float HealthDelta);

	UFUNCTION()
	void SpawnPotionDrop();

	void ReleaseStun();

	FTimerHandle StunTimerHandle;

	UPROPERTY(ReplicatedUsing = OnRep_IsStunned)
	bool bIsStunned = false;

	UFUNCTION()
	void OnRep_IsStunned();

	UPROPERTY(EditAnywhere, Category = "Combat | Stun")
	TObjectPtr<UNiagaraSystem> StunVFX;

	UPROPERTY()
	TObjectPtr<UNiagaraComponent> ActiveStunVFX;

	UPROPERTY(EditAnywhere, Category = "Combat | Stun")
	float StunVFXHeightOffset = 100.0f;

	UPROPERTY()
	TObjectPtr<AActor> LastAttacker;
};