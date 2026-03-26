// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Characters/EnemyBase.h"
#include "MeleeEnemy.generated.h"

class USphereComponent;

UCLASS()
class CELESTIA_API AMeleeEnemy : public AEnemyBase
{
	GENERATED_BODY()

public:
	AMeleeEnemy();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat")
	USphereComponent* DamageSphere;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat | Animation")
	UAnimMontage* AttackMontage;

	UFUNCTION(BlueprintCallable, Category = "Combat")
	void StartDamage(AActor* TargetActor);

	UFUNCTION(BlueprintCallable, Category = "Combat")
	void StopDamage();

protected:
	virtual void BeginPlay() override;
	virtual void Die_Implementation() override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat | Settings")
	float DamageInterval = 1.f;

	UFUNCTION()
	void OnHitPlayerBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void OnHitPlayerEndOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

private:
	void ApplyDamage();

	UPROPERTY()
	AActor* DamageTarget = nullptr;
	FTimerHandle DamageTimerHandle;
};