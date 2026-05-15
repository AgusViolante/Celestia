// Fill out your copyright notice in the Description page of Project Settings.


// Fill out your copyright notice in the Description page of Project Settings.

#include "Characters/BossEnemy.h"
#include "../../CelestiaCharacter.h"
#include "UI/UIPlayerHUD.h"
#include "Components/HealthComponent.h"
#include "Components/StatsComponent.h"
#include "Components/SphereComponent.h"
#include "Components/WidgetComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Blueprint/WidgetBlueprintLibrary.h"
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

	CurrentPhase = 1;

	EnemyLevel = 1;
	
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
	if (GetCapsuleComponent())
	{
		GetCapsuleComponent()->CanCharacterStepUpOn = ECB_No;
		GetCapsuleComponent()->SetWalkableSlopeOverride(FWalkableSlopeOverride(WalkableSlope_Unwalkable, 0.f));
	}
	if (GetMesh())
	{
		GetMesh()->CanCharacterStepUpOn = ECB_No;
	}

	ProximitySphere = CreateDefaultSubobject<USphereComponent>(TEXT("ProximitySphere"));
	ProximitySphere->SetupAttachment(RootComponent);
	ProximitySphere->SetSphereRadius(3000.f);
	ProximitySphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);

	ProximitySphere->SetCollisionResponseToAllChannels(ECR_Ignore);
	ProximitySphere->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
}

void ABossEnemy::BeginPlay()
{
	Super::BeginPlay();

	if (ProximitySphere)
	{
		ProximitySphere->OnComponentBeginOverlap.AddDynamic(this, &ABossEnemy::OnProximityOverlapBegin);
		ProximitySphere->OnComponentEndOverlap.AddDynamic(this, &ABossEnemy::OnProximityOverlapEnd);
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
	ACharacter* PlayerChar = UGameplayStatics::GetPlayerCharacter(GetWorld(), 0);
	if (PlayerChar)
	{
		FVector PushDirection = PlayerChar->GetActorLocation() - GetActorLocation();
		PushDirection.Z = 0.f;

		if (PushDirection.SizeSquared() < 1.f)
		{
			PushDirection = GetActorForwardVector();
		}
		PushDirection.Z = 0.f;
		PushDirection.Normalize();

		float ForceHorizontal = 5000.f;
		float ForceVertical = 0.f; 

		FVector FinalLaunchVelocity = (PushDirection * ForceHorizontal);

		PlayerChar->LaunchCharacter(FinalLaunchVelocity, true, true);
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

// --- 4. DISPARO EN AREA (Fase 2) ---
void ABossEnemy::FireMagicSpread()
{
	if (!ProjectileClass || bAlreadyDied) return;

	FVector SpawnLocation = GetActorLocation() + FVector(0.f, 0.f, 40.f);

	float AngleStep = 360.0f / 20.0f;

	for (int32 i = 0; i < 20; i++)
	{
		FRotator SpawnRotation = GetActorRotation();
		SpawnRotation.Yaw += (AngleStep * i);

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
				SpawnedProj->DamageAmount = StatsComponent->GetStatValue(ERPGStatType::MagicAttack);
			}
		}
	}
}void ABossEnemy::PerformRangedAttack()
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
	if (HealthDelta >= 0.f || bAlreadyDied) return;

	ACelestiaCharacter* PlayerChar = Cast<ACelestiaCharacter>(UGameplayStatics::GetPlayerCharacter(GetWorld(), 0));
	if (PlayerChar && PlayerChar->PlayerHUDInstance)
	{
		PlayerChar->PlayerHUDInstance->UpdateBossHealth(NewHealth, MaxHealth);
	}

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
void ABossEnemy::OnProximityOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Orange, TEXT("Jefe: Algo entró en el radio"));

	ACelestiaCharacter* PlayerChar = Cast<ACelestiaCharacter>(OtherActor);
	if (PlayerChar)
	{
		if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, TEXT("Jefe: Es el Jugador"));

		if (PlayerChar->PlayerHUDInstance)
		{
			if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Cyan, TEXT("Jefe: Llamando al HUD..."));
			PlayerChar->PlayerHUDInstance->ShowBossUI(TEXT("Jefe de Celestia"), HealthComponent->Health, HealthComponent->MaxHealth);
		}
		else
		{
			if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("ERROR: PlayerHUDInstance es NULO en el personaje"));
		}
	}
}

void ABossEnemy::OnProximityOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	ACelestiaCharacter* PlayerChar = Cast<ACelestiaCharacter>(OtherActor);
	if (PlayerChar && PlayerChar->PlayerHUDInstance)
	{
		PlayerChar->PlayerHUDInstance->HideBossUI();
	}
}

void ABossEnemy::Die_Implementation()
{
	Super::Die_Implementation();

	ACelestiaCharacter* PlayerChar = Cast<ACelestiaCharacter>(UGameplayStatics::GetPlayerCharacter(GetWorld(), 0));
	if (PlayerChar && PlayerChar->PlayerHUDInstance)
	{
		PlayerChar->PlayerHUDInstance->HideBossUI();
	}
}