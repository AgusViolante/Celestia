// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/RangedEnemy/RangedEnemy.h"
#include "Characters/RangedEnemy/MagicProjectile.h"
#include "Components/SkeletalMeshComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Character.h"

ARangedEnemy::ARangedEnemy()
{
	EnemyType = EEnemyClassType::Ranged;

	bUseControllerRotationYaw = true;

	GetCharacterMovement()->bOrientRotationToMovement = false;
}

void ARangedEnemy::PerformRangedAttack()
{
	if (bAlreadyDied) return;

	UAnimInstance* AnimInst = (GetMesh() ? GetMesh()->GetAnimInstance() : nullptr);
	if (AttackMontage && AnimInst)
	{
		AnimInst->Montage_Play(AttackMontage);
	}
}

void ARangedEnemy::FireMagic()
{
	if (!ProjectileClass || bAlreadyDied) return;

	FVector ForwardOffset = GetActorForwardVector() * 100.f; // 1 metro hacia adelante
	FVector SpawnLocation = GetMesh()->GetSocketLocation(MuzzleSocketName) + ForwardOffset;


	FRotator SpawnRotation = GetActorRotation();

	ACharacter* PlayerChar = UGameplayStatics::GetPlayerCharacter(GetWorld(), 0);
	if (PlayerChar)
	{
		
		FVector TargetLocation = PlayerChar->GetActorLocation();

		SpawnRotation = UKismetMathLibrary::FindLookAtRotation(SpawnLocation, TargetLocation);
	}

	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = this;
	SpawnParams.Instigator = this;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	AMagicProjectile* SpawnedProj = GetWorld()->SpawnActor<AMagicProjectile>(ProjectileClass, SpawnLocation, SpawnRotation, SpawnParams);

	if (SpawnedProj)
	{
		SpawnedProj->CollisionComp->IgnoreActorWhenMoving(this, true);
	}
}
