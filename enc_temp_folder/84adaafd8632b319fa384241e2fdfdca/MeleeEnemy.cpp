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
	DamageSphere->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
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
	if (!DamageTarget) return;

	if (UHealthComponent* TargetHealthComp = DamageTarget->FindComponentByClass<UHealthComponent>())
	{
		float EnemyDamageAmount = 20.0f; // Daño por defecto fallback
		bool bWasCriticalHit = false; // Bandera para saber si fue crítico

		if (StatsComponent)
		{
			// 1. Obtenemos el daño base
			EnemyDamageAmount = StatsComponent->GetStatValue(ERPGStatType::MeleeAttack);

			// 2. --- LÓGICA DE CRÍTICO ---
			float CritChance = StatsComponent->GetStatValue(ERPGStatType::MeleeCrit);
			float RandomRoll = FMath::RandRange(0.0f, 100.0f); // Tiramos un dado del 0 al 100

			// Si el dado saca menos o igual a nuestra probabilidad... ¡CRÍTICO!
			if (RandomRoll <= CritChance)
			{
				EnemyDamageAmount *= 1.5f; // Multiplicamos el daño x1.5 (puedes cambiar esto a x2.0)
				bWasCriticalHit = true;
			}
		}

		if (AttackMontage && GetMesh()->GetAnimInstance())
		{
			if (!GetMesh()->GetAnimInstance()->Montage_IsPlaying(AttackMontage))
			{
				GetMesh()->GetAnimInstance()->Montage_Play(AttackMontage, 1.0f);
			}
		}

		if (bWasCriticalHit && GEngine)
		{
			GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Yellow, TEXT("¡El Enemigo asestó un GOLPE CRÍTICO FÍSICO!"));
		}

		TargetHealthComp->TakeDamage(EnemyDamageAmount * DamageInterval, bWasCriticalHit);

		if (TargetHealthComp->IsDead()) StopDamage();
	}
	else
	{
		StopDamage();
	}
}
void AMeleeEnemy::Die_Implementation()
{
	StopDamage();
	if (DamageSphere) DamageSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	Super::Die_Implementation();
}
