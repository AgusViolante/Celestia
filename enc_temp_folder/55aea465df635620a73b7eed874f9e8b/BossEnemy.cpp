// Fill out your copyright notice in the Description page of Project Settings.


// Fill out your copyright notice in the Description page of Project Settings.

#include "Characters/BossEnemy.h"
#include "Components/HealthComponent.h"
#include "Components/StatsComponent.h"
#include "Components/SphereComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "NavigationSystem.h"
#include "Characters/RangedEnemy/MagicProjectile.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "AIController.h"
#include "NiagaraFunctionLibrary.h"

ABossEnemy::ABossEnemy()
{
	
	EnemyType = EEnemyClassType::Boss;

	
	bUseControllerRotationYaw = false;

		if (GetCharacterMovement())
		{
			GetCharacterMovement()->bOrientRotationToMovement = false;

			GetCharacterMovement()->bUseControllerDesiredRotation = true;

			GetCharacterMovement()->RotationRate = FRotator(0.0f, 600.0f, 0.0f);
		}
	if (DamageSphere)
	{
		DamageSphere->SetSphereRadius(150.f);
	}
	if (HealthComponent)
	{
		HealthComponent->OnHealthChanged.AddDynamic(this, &ABossEnemy::OnBossHealthChanged);
	}

}
void ABossEnemy::EnterPhase2()
{
	CurrentPhase = 2;
	StopDamage();
	StopAnimMontage();

	if (PhaseTransitionMontage && GetMesh()->GetAnimInstance())
	{
		GetMesh()->GetAnimInstance()->Montage_Play(PhaseTransitionMontage, 1.0f);
	}

	if (PhaseTransitionVFX)
	{
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), PhaseTransitionVFX, GetActorLocation() - FVector(0.f, 0.f, 90.f), FRotator::ZeroRotator);
	}

	//Onda expansiva que empuja al jugador
	ACharacter* PlayerChar = UGameplayStatics::GetPlayerCharacter(GetWorld(), 0);
	if (PlayerChar)
	{
		float Distance = FVector::Dist(GetActorLocation(), PlayerChar->GetActorLocation());
		if (Distance < 800.f) 
		{
			FVector PushDirection = PlayerChar->GetActorLocation() - GetActorLocation();
			PushDirection.Z = 0.f; 
			PushDirection.Normalize();

			
			PlayerChar->LaunchCharacter((PushDirection * 2000.f) + FVector(0.f, 0.f, 800.f), true, true);
		}
	}

	if (AAIController* AICon = Cast<AAIController>(GetController()))
	{
		if (UBlackboardComponent* BB = AICon->GetBlackboardComponent())
		{
			BB->SetValueAsInt(FName("BossPhase"), CurrentPhase);
		}
	}
}

void ABossEnemy::LeapTowardsPlayer()
{
	ACharacter* PlayerChar = UGameplayStatics::GetPlayerCharacter(GetWorld(), 0);
	if (!PlayerChar || bAlreadyDied) return;

	FVector LeapDirection = PlayerChar->GetActorLocation() - GetActorLocation();
	LeapDirection.Z = 0.f;
	LeapDirection.Normalize();

	SetActorRotation(LeapDirection.Rotation());

	LaunchCharacter((LeapDirection * 1500.f) + FVector(0.f, 0.f, 600.f), true, true);
}


void ABossEnemy::TeleportAway()
{
	if (bAlreadyDied) return;

	ACharacter* PlayerChar = UGameplayStatics::GetPlayerCharacter(GetWorld(), 0);
	if (!PlayerChar) return;

	FVector AwayDirection = GetActorLocation() - PlayerChar->GetActorLocation();
	AwayDirection.Z = 0.f;
	AwayDirection.Normalize();

	FVector TargetLocation = GetActorLocation() + (AwayDirection * 1000.f);

	
	UNavigationSystemV1* NavSystem = UNavigationSystemV1::GetCurrent(GetWorld());
	if (NavSystem)
	{
		FNavLocation SafeLocation;
		if (NavSystem->ProjectPointToNavigation(TargetLocation, SafeLocation, FVector(500.f, 500.f, 500.f)))
		{
			
			if (PhaseTransitionVFX) UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), PhaseTransitionVFX, GetActorLocation(), FRotator::ZeroRotator);

			SetActorLocation(SafeLocation.Location);

			
			if (PhaseTransitionVFX) UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), PhaseTransitionVFX, SafeLocation.Location, FRotator::ZeroRotator);
		}
	}
}

// --- 4. DISPARO EN ABANICO (Fase 2) ---
void ABossEnemy::FireMagicSpread()
{
	// 1. Verificamos si realmente asignaste el BP
	if (!ProjectileClass)
	{
		if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("ERROR: PROJECTILE CLASS VACIO EN EL BP"));
		return;
	}

	if (bAlreadyDied) return;

	// 2. PARCHE SIN ANIMACIÓN: Disparamos desde el centro del jefe, pero 150 unidades hacia adelante y 80 hacia arriba (altura del pecho/cabeza) para que no choque con el piso.
	FVector SpawnLocation = GetActorLocation() + (GetActorForwardVector() * 150.f) + FVector(0.f, 0.f, 80.f);

	ACharacter* PlayerChar = UGameplayStatics::GetPlayerCharacter(GetWorld(), 0);
	if (!PlayerChar) return;

	FRotator BaseRotation = UKismetMathLibrary::FindLookAtRotation(SpawnLocation, PlayerChar->GetActorLocation());

	TArray<float> AngleOffsets = { -20.f, 0.f, 20.f };

	for (float Offset : AngleOffsets)
	{
		FRotator SpawnRotation = BaseRotation;
		SpawnRotation.Yaw += Offset;

		FActorSpawnParameters SpawnParams;
		SpawnParams.Owner = this;
		SpawnParams.Instigator = this;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

		AMagicProjectile* SpawnedProj = GetWorld()->SpawnActor<AMagicProjectile>(ProjectileClass, SpawnLocation, SpawnRotation, SpawnParams);

		// 3. Verificamos si se creó o si se destruyó al instante
		if (SpawnedProj)
		{
			SpawnedProj->CollisionComp->IgnoreActorWhenMoving(this, true);
			if (StatsComponent) SpawnedProj->DamageAmount = StatsComponent->GetStatValue(ERPGStatType::MagicAttack);
			if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Green, TEXT("¡ÉXITO! Proyectil spawneado."));
		}
		else
		{
			if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Red, TEXT("FALLO: El proyectil chocó y se destruyó al nacer."));
		}
	}
}
void ABossEnemy::PerformRangedAttack()
{
	if (bAlreadyDied) return;

	UAnimInstance* AnimInst = (GetMesh() ? GetMesh()->GetAnimInstance() : nullptr);

	if (RangedAttackMontage && AnimInst)
	{
		AnimInst->Montage_Play(RangedAttackMontage);
	}
	else
	{
		FireMagic();
	}
}
void ABossEnemy::FireMagic()
{
	if (!ProjectileClass || bAlreadyDied) return;

	FVector ForwardOffset = GetActorForwardVector() * 100.f;
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

		if (StatsComponent)
		{
			float FinalMagicDamage = StatsComponent->GetStatValue(ERPGStatType::MagicAttack);
			float CritChance = StatsComponent->GetStatValue(ERPGStatType::MagicCrit);
			float RandomRoll = FMath::RandRange(0.0f, 100.0f);

			if (RandomRoll <= CritChance)
			{
				FinalMagicDamage *= 1.5f;
				if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Purple, TEXT("¡Hechizo Crítico del Jefe!"));
			}

			SpawnedProj->DamageAmount = FinalMagicDamage;
		}
	}
}
void ABossEnemy::OnBossHealthChanged(UHealthComponent* InHealthComp, float NewHealth, float MaxHealth, float HealthDelta)
{
	if (HealthDelta >= 0.f || bAlreadyDied || CurrentPhase != 1) return;

	if (MaxHealth > 0.0f)
	{
		float HealthPercent = NewHealth / MaxHealth;

		if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Yellow, FString::Printf(TEXT("Vida del Jefe: %.1f %%"), HealthPercent * 100.f));

		if (HealthPercent <= Phase2HealthThreshold)
		{
			if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 4.0f, FColor::Purple, TEXT("¡UMBRAL CRUZADO! Pasando a Fase 2..."));
			EnterPhase2();
		}
	}
}