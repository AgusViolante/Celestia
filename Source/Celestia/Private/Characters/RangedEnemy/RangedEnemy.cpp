#include "Characters/RangedEnemy/RangedEnemy.h"
#include "Components/StatsComponent.h"
#include "Characters/RangedEnemy/MagicProjectile.h"
#include "Components/SkeletalMeshComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Character.h"
#include "Components/SphereComponent.h"
#include "GameFramework/CharacterMovementComponent.h"

ARangedEnemy::ARangedEnemy()
{
	EnemyType = EEnemyClassType::Ranged;
	bUseControllerRotationYaw = true;
	if (GetCharacterMovement())
	{
		GetCharacterMovement()->bOrientRotationToMovement = false;
	}
}

void ARangedEnemy::PerformRangedAttack()
{
	if (bAlreadyDied || !HasAuthority()) return;
	Multicast_PlayRangedMontage();
}

void ARangedEnemy::Multicast_PlayRangedMontage_Implementation()
{
	UAnimInstance* AnimInst = (GetMesh() ? GetMesh()->GetAnimInstance() : nullptr);
	if (AttackMontage && AnimInst)
	{
		AnimInst->Montage_Play(AttackMontage);
	}
}

void ARangedEnemy::FireMagic()
{
	if (!HasAuthority() || !ProjectileClass || bAlreadyDied) return;

	FVector ForwardOffset = GetActorForwardVector() * 100.f;
	FVector SpawnLocation = GetMesh()->GetSocketLocation(MuzzleSocketName) + ForwardOffset;
	FRotator SpawnRotation = GetActorRotation();

	AActor* Target = GetAITarget();
	if (!Target) Target = LastAttacker;

	if (Target)
	{
		FVector TargetLocation = Target->GetActorLocation();
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

		if (StatsComponent)
		{
			float FinalMagicDamage = StatsComponent->GetStatValue(ERPGStatType::MagicAttack);
			float CritChance = StatsComponent->GetStatValue(ERPGStatType::MagicCrit);

			if (FMath::RandRange(0.0f, 100.0f) <= CritChance)
			{
				FinalMagicDamage *= 1.5f;
			}
			SpawnedProj->DamageAmount = FinalMagicDamage;
		}
	}
}