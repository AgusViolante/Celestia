// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/MeleeEnemy.h"
#include "Components/SphereComponent.h"
#include "Components/HealthComponent.h"
#include "Components/StatsComponent.h"
#include "TimerManager.h"
#include "../CelestiaCharacter.h" 

AMeleeEnemy::AMeleeEnemy()
{
	DamageSphere = CreateDefaultSubobject<USphereComponent>(TEXT("DamageSphere"));
	DamageSphere->SetupAttachment(RootComponent);
	DamageSphere->SetSphereRadius(75.f);
	DamageSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	DamageSphere->SetCollisionResponseToAllChannels(ECR_Ignore);
	DamageSphere->SetCollisionProfileName(TEXT("Trigger"));
	DamageSphere->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);

	StunVFXHeightOffset = 180.0f;
}

void AMeleeEnemy::BeginPlay()
{
	Super::BeginPlay();

	if (DamageSphere)
	{
		DamageSphere->OnComponentBeginOverlap.AddDynamic(this, &AMeleeEnemy::OnHitPlayerBeginOverlap);
		DamageSphere->OnComponentEndOverlap.AddDynamic(this, &AMeleeEnemy::OnHitPlayerEndOverlap);
	}
}

void AMeleeEnemy::OnHitPlayerBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (!OtherActor || OtherActor == this) return;

	if (ACelestiaCharacter* Player = Cast<ACelestiaCharacter>(OtherActor))
	{
		StartDamage(Player);
	}
}

void AMeleeEnemy::OnHitPlayerEndOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (!OtherActor || OtherActor == this) return;

	if (Cast<ACelestiaCharacter>(OtherActor))
	{
		StopDamage();
	}
}

void AMeleeEnemy::StartDamage(AActor* TargetActor)
{
	if (!TargetActor || (DamageTarget == TargetActor && GetWorldTimerManager().IsTimerActive(DamageTimerHandle))) return;

	DamageTarget = TargetActor;
	float Interval = FMath::Max(0.01f, DamageInterval);
	GetWorldTimerManager().SetTimer(DamageTimerHandle, this, &AMeleeEnemy::ApplyDamage, Interval, true, Interval);
}

void AMeleeEnemy::StopDamage()
{
	GetWorldTimerManager().ClearTimer(DamageTimerHandle);
	DamageTarget = nullptr;
}

void AMeleeEnemy::ApplyDamage()
{
	if (!DamageTarget || bIsStunned) return;

	if (AttackMontage && GetMesh()->GetAnimInstance())
	{
		if (!GetMesh()->GetAnimInstance()->Montage_IsPlaying(AttackMontage))
		{
			GetMesh()->GetAnimInstance()->Montage_Play(AttackMontage, 1.0f);
		}
	}
}

void AMeleeEnemy::ExecuteMeleeHit()
{
	if (!DamageTarget || bIsStunned) return;

	if (UHealthComponent* TargetHealthComp = DamageTarget->FindComponentByClass<UHealthComponent>())
	{
		float EnemyDamageAmount = 20.0f;
		bool bWasCriticalHit = false;

		if (StatsComponent)
		{
			EnemyDamageAmount = StatsComponent->GetStatValue(ERPGStatType::MeleeAttack);
			float CritChance = StatsComponent->GetStatValue(ERPGStatType::MeleeCrit);
			float RandomRoll = FMath::RandRange(0.0f, 100.0f);

			if (RandomRoll <= CritChance)
			{
				EnemyDamageAmount *= 1.5f;
				bWasCriticalHit = true;
			}
		}

		if (bWasCriticalHit && GEngine)
		{
			GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Yellow, TEXT("¡El Enemigo asestó un GOLPE CRÍTICO FÍSICO!"));
		}

		TargetHealthComp->TakeDamage(EnemyDamageAmount * DamageInterval, bWasCriticalHit);

		if (TargetHealthComp->IsDead()) StopDamage();
	}
}
void AMeleeEnemy::Die_Implementation()
{
	StopDamage();
	if (DamageSphere) DamageSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	Super::Die_Implementation();
}
