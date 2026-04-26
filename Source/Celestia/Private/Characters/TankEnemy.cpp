// Fill out your copyright notice in the Description page of Project Settings.
#include "Characters/TankEnemy.h"
#include "GameFramework/CharacterMovementComponent.h"

ATankEnemy::ATankEnemy()
{
	// Le asignamos su arquetipo para que calcule los stats correctamente
	EnemyType = EEnemyClassType::Tank;

	// Hacemos que sea más lento modificando su componente de movimiento
	if (UCharacterMovementComponent* MoveComp = GetCharacterMovement())
	{
		MoveComp->MaxWalkSpeed = 250.f; // Un enemigo normal suele tener 600.f
	}

}

void ATankEnemy::BeginPlay()
{
	Super::BeginPlay();

	// Si quieres que el tanque ataque más lento (ej: cada 2.5 segundos)
	// Como DamageInterval es Protected en AMeleeEnemy, podemos cambiarlo aquí
	DamageInterval = 2.5f;
}