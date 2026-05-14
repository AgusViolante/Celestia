// Fill out your copyright notice in the Description page of Project Settings.

// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Characters/MeleeEnemy.h" // Heredamos de MeleeEnemy para tener los ataques físicos gratis
#include "BossEnemy.generated.h"

class AMagicProjectile;

UCLASS()
class CELESTIA_API ABossEnemy : public AMeleeEnemy
{
	GENERATED_BODY()

public:
	ABossEnemy();

	// --- FASES DEL JEFE ---
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Boss | Phase")
	int32 CurrentPhase = 1;

	// Porcentaje de vida (0.0 a 1.0) en el que el jefe pasa a Fase 2
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Boss | Phase", meta = (ClampMin = "0.1", ClampMax = "0.9"))
	float Phase2HealthThreshold = 0.5f;

	// --- COMBATE A DISTANCIA (Mágico) ---
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat | Magic")
	UAnimMontage* RangedAttackMontage;

	UPROPERTY(EditDefaultsOnly, Category = "Combat | Magic")
	TSubclassOf<AMagicProjectile> ProjectileClass;

	UPROPERTY(EditDefaultsOnly, Category = "Combat | Magic")
	FName MuzzleSocketName = TEXT("RightHandSocket");

	UFUNCTION(BlueprintCallable, Category = "Combat | Magic")
	void PerformRangedAttack();

	// Se llama desde el AnimNotify (igual que en RangedEnemy)
	UFUNCTION(BlueprintCallable, Category = "Combat | Magic")
	void FireMagic();

	// --- EVENTO DE CAMBIO DE FASE ---
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss | Phase")
	UAnimMontage* PhaseTransitionMontage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss | Phase")
	class UNiagaraSystem* PhaseTransitionVFX;

	UFUNCTION(BlueprintCallable, Category = "Boss | Phase")
	void EnterPhase2();

	// Sobrescribimos TakeDamage para leer constantemente la vida del jefe
	virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser) override;

	// --- MECÁNICAS DE FASE 1 ---
	UFUNCTION(BlueprintCallable, Category = "Boss | Combat")
	void LeapTowardsPlayer();

	// --- MECÁNICAS DE FASE 2 ---
	UFUNCTION(BlueprintCallable, Category = "Boss | Combat")
	void TeleportAway();

	// Disparo en abanico (3 proyectiles)
	UFUNCTION(BlueprintCallable, Category = "Combat | Magic")
	void FireMagicSpread();

};