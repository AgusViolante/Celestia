// Fill out your copyright notice in the Description page of Project Settings.
#include "Characters/TankEnemy.h"
#include "GameFramework/CharacterMovementComponent.h"

ATankEnemy::ATankEnemy()
{
	
	EnemyType = EEnemyClassType::Tank;

	if (UCharacterMovementComponent* MoveComp = GetCharacterMovement())
	{
		MoveComp->MaxWalkSpeed = 250.f; 
	}
	StunVFXHeightOffset = 180.0f;
}

void ATankEnemy::BeginPlay()
{
	Super::BeginPlay();

	DamageInterval = 2.5f;
}