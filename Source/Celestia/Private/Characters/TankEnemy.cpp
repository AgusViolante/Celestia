// Fill out your copyright notice in the Description page of Project Settings.
#include "Characters/TankEnemy.h"
#include "GameFramework/CharacterMovementComponent.h"

ATankEnemy::ATankEnemy()
{
	EnemyType = EEnemyClassType::Tank;
	StunVFXHeightOffset = 180.0f;

	if (UCharacterMovementComponent* MoveComp = GetCharacterMovement())
	{
		MoveComp->MaxWalkSpeed = 250.f;

		MoveComp->bOrientRotationToMovement = false;

		MoveComp->bUseControllerDesiredRotation = true;

		MoveComp->RotationRate = FRotator(0.f, 300.f, 0.f);
		MoveComp->Mass = 10000.f;
		MoveComp->bPushForceScaledToMass = true;
	}

	if (DamageSphere)
	{
		DamageSphere->SetSphereRadius(140.f);
	}


}

void ATankEnemy::BeginPlay()
{
	Super::BeginPlay();

	DamageInterval = 2.5f;
}