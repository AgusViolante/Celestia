#include "Characters/TankEnemy.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/SphereComponent.h"

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

void ATankEnemy::OnRep_IsStunned()
{
	Super::OnRep_IsStunned();

	if (DamageSphere)
	{
		if (bIsStunned)
		{
			DamageSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		}
		else
		{
			DamageSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
		}
	}
}