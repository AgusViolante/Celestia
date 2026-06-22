#pragma once

#include "CoreMinimal.h"
#include "Characters/MeleeEnemy.h" 
#include "BossEnemy.generated.h"

class AMagicProjectile;
class UAnimMontage;
class UNiagaraSystem;
class USphereComponent;
class UUserWidget;

UCLASS()
class CELESTIA_API ABossEnemy : public AMeleeEnemy
{
	GENERATED_BODY()

public:
	ABossEnemy();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, ReplicatedUsing = OnRep_CurrentPhase, Category = "Boss | Phase")
	int32 CurrentPhase = 1;

	UFUNCTION()
	void OnRep_CurrentPhase();

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Boss | Phase", meta = (ClampMin = "0.1", ClampMax = "0.9"))
	float Phase2HealthThreshold = 0.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat | Magic")
	TObjectPtr<UAnimMontage> RangedAttackMontage;

	UPROPERTY(EditDefaultsOnly, Category = "Combat | Magic")
	TSubclassOf<AMagicProjectile> ProjectileClass;

	UPROPERTY(EditDefaultsOnly, Category = "Combat | Magic")
	FName MuzzleSocketName = TEXT("RightHandSocket");

	UFUNCTION(BlueprintCallable, Category = "Combat | Magic")
	void PerformRangedAttack();

	UFUNCTION(BlueprintCallable, Category = "Combat | Magic")
	void FireMagic();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss | Phase")
	TObjectPtr<UAnimMontage> PhaseTransitionMontage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss | Phase")
	TObjectPtr<UNiagaraSystem> PhaseTransitionVFX;

	UFUNCTION(BlueprintCallable, Category = "Boss | Phase")
	void EnterPhase2();

	UFUNCTION(BlueprintCallable, Category = "Boss | Combat")
	void LeapTowardsPlayer();

	UFUNCTION(BlueprintCallable, Category = "Boss | Combat")
	void TeleportAway();

	UFUNCTION(BlueprintCallable, Category = "Combat | Magic")
	void FireMagicSpread();

	UFUNCTION()
	void OnBossHealthChanged(UHealthComponent* InHealthComp, float NewHealth, float MaxHealth, float HealthDelta);

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "UI")
	TObjectPtr<USphereComponent> ProximitySphere;

	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<UUserWidget> BossHUDClass;

	virtual void Die_Implementation() override;

protected:
	virtual void BeginPlay() override;

	UFUNCTION(NetMulticast, Unreliable)
	void Multicast_PlayRangedMontage();

	UFUNCTION()
	void OnProximityOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void OnProximityOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);
};