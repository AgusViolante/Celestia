// Fill out your copyright notice in the Description page of Project Settings.

#include "Characters/EnemyCharacter.h"
#include "Components/SphereComponent.h"
#include "Interfaces/DeathInterface.h"
#include "Animation/AnimInstance.h"
#include "AI/EnemyAIController.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "../../CelestiaCharacter.h"
#include "Components/HealthComponent.h"
#include "Components/CapsuleComponent.h"
#include "TimerManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"

AEnemyCharacter::AEnemyCharacter()
{
    PrimaryActorTick.bCanEverTick = false;

    bUseControllerRotationYaw = false;
    if (UCharacterMovementComponent* MoveComp = GetCharacterMovement())
    {
        MoveComp->bOrientRotationToMovement = true;
        MoveComp->RotationRate = FRotator(0.f, 720.f, 0.f);
    }

    HealthComponent = CreateDefaultSubobject<UHealthComponent>(TEXT("HealthComponent"));
    if (HealthComponent)
    {
        HealthComponent->bAutoRegen = true;
        HealthComponent->RegenDelaySeconds = 60.f;
        HealthComponent->RegenPerSecond = 1.f;
        HealthComponent->RegenTickInterval = 1.f;
    }

    DetectionSphere = CreateDefaultSubobject<USphereComponent>(TEXT("DetectionSphere"));
    DetectionSphere->SetupAttachment(RootComponent);
    DetectionSphere->SetSphereRadius(800.f);
    DetectionSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    DetectionSphere->SetCollisionObjectType(ECC_WorldDynamic);
    DetectionSphere->SetCollisionResponseToAllChannels(ECR_Ignore);
    DetectionSphere->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
    DetectionSphere->SetGenerateOverlapEvents(true);
    DetectionSphere->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);
    DetectionSphere->SetCollisionResponseToChannel(ECC_Visibility, ECR_Ignore);

    DamageSphere = CreateDefaultSubobject<USphereComponent>(TEXT("DamageSphere"));
    DamageSphere->SetupAttachment(RootComponent);
    DamageSphere->SetSphereRadius(75.f); 
    DamageSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    DamageSphere->SetCollisionObjectType(ECC_WorldDynamic);
    DamageSphere->SetCollisionResponseToAllChannels(ECR_Ignore);
    DamageSphere->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
    DamageSphere->SetGenerateOverlapEvents(true);
    DamageSphere->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);
    DamageSphere->SetCollisionResponseToChannel(ECC_Visibility, ECR_Ignore);

    AIControllerClass = AEnemyAIController::StaticClass();

    AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;
}

void AEnemyCharacter::BeginPlay()
{
    Super::BeginPlay();

    CurrentTarget = nullptr;
    if (UCapsuleComponent* Capsule = GetCapsuleComponent())
    {
        Capsule->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
        Capsule->SetCollisionResponseToChannel(ECC_Pawn, ECR_Block);
        Capsule->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Block);
        Capsule->SetCollisionResponseToChannel(ECC_WorldDynamic, ECR_Block);
        Capsule->SetCollisionResponseToChannel(ECC_PhysicsBody, ECR_Block);
        Capsule->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);
        Capsule->SetCollisionResponseToChannel(ECC_Visibility, ECR_Ignore);
    }

    if (DetectionSphere)
    {
        DetectionSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
        DetectionSphere->SetCollisionResponseToAllChannels(ECR_Ignore);
        DetectionSphere->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
     

        DetectionSphere->OnComponentBeginOverlap.AddDynamic(this, &AEnemyCharacter::OnDetectionBeginOverlap);
        DetectionSphere->OnComponentEndOverlap.AddDynamic(this, &AEnemyCharacter::OnDetectionEndOverlap);

        TArray<AActor*> OverlappingActors;
        DetectionSphere->GetOverlappingActors(OverlappingActors, ACelestiaCharacter::StaticClass());

        for (AActor* Actor : OverlappingActors)
        {
            if (Actor && Actor != this)
            {
                OnDetectionBeginOverlap(DetectionSphere, Actor, nullptr, 0, false, FHitResult());
            }
        }
    }

    if (DamageSphere)
    {
        DamageSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
        DamageSphere->SetCollisionResponseToAllChannels(ECR_Ignore);
        DamageSphere->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);

        DamageSphere->OnComponentBeginOverlap.AddDynamic(this, &AEnemyCharacter::OnHitPlayerBeginOverlap);
        DamageSphere->OnComponentEndOverlap.AddDynamic(this, &AEnemyCharacter::OnHitPlayerEndOverlap);
    }

    if (HealthComponent)
    {
        HealthComponent->OnDeath.AddDynamic(this, &AEnemyCharacter::OnHealthComponentDeath);
    }

}

void AEnemyCharacter::OnDetectionBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
    UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    if (!OtherActor || OtherActor == this) return;

    if (ACelestiaCharacter* Player = Cast<ACelestiaCharacter>(OtherActor))
    {
        CurrentTarget = Player;
        if (AEnemyAIController* AICon = Cast<AEnemyAIController>(GetController()))
        {
            AICon->StartChasing(CurrentTarget, 50.f);
        }
        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Orange, TEXT("Enemigo: jugador detectado"));
        }
    }
}

void AEnemyCharacter::OnDetectionEndOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
    UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
    if (!OtherActor || OtherActor == this) return;

    if (ACelestiaCharacter* Player = Cast<ACelestiaCharacter>(OtherActor))
    {
        if (CurrentTarget == Player)
        {
            CurrentTarget = nullptr;
            if (AEnemyAIController* AICon = Cast<AEnemyAIController>(GetController()))
            {
                AICon->StopChasing();
            }
            if (GEngine)
            {
                GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Yellow, TEXT("Enemigo: jugador perdido"));
            }
        }
    }
}


void AEnemyCharacter::OnHitPlayerBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
    UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    if (!OtherActor || OtherActor == this) return;

    if (ACelestiaCharacter* Player = Cast <ACelestiaCharacter >(OtherActor))
    {
        StartDamage(Player);
    }
}

void AEnemyCharacter::OnHitPlayerEndOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
    UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
    if (!OtherActor || OtherActor == this) return;

    if (ACelestiaCharacter* Player = Cast<ACelestiaCharacter>(OtherActor))
    {
        StopDamage();
    }
}

void AEnemyCharacter::StartDamage(AActor* TargetActor)
{
    if (!TargetActor) return;

    if (DamageTarget == TargetActor && GetWorld() && GetWorld()->GetTimerManager().IsTimerActive(DamageTimerHandle))
    {
        return;
    }

    DamageTarget = TargetActor;

    if (GetWorld())
    {
        
        GetWorld()->GetTimerManager().ClearTimer(DamageTimerHandle);

        float Interval = FMath::Max(0.01f, DamageInterval);
        GetWorld()->GetTimerManager().SetTimer(DamageTimerHandle, this, &AEnemyCharacter::ApplyDamage, Interval, true, Interval);
    }

}

void AEnemyCharacter::StopDamage()
{
    if (GetWorld())
    {
        GetWorld()->GetTimerManager().ClearTimer(DamageTimerHandle);
    }
    DamageTarget = nullptr;

}

void AEnemyCharacter::ApplyDamage()
{
    if (!DamageTarget)
        return;

    if (UHealthComponent* HealthComp = DamageTarget->FindComponentByClass<UHealthComponent>())
    {
        const float Amount = DamagePerSecond * DamageInterval;

        if (AttackMontage)
        {
            if (UAnimInstance* AnimInst = GetMesh()->GetAnimInstance())
            {
                
                if (!AnimInst->Montage_IsPlaying(AttackMontage))
                {
                    AnimInst->Montage_Play(AttackMontage, 1.0f);
                }
            }
        }

        HealthComp->TakeDamage(Amount);

        if (GEngine)
        {
            FString Msg = FString::Printf(TEXT("Enemigo hizo %.1f de danio -> Vida actual: %.1f / %.1f"),
                Amount, HealthComp->Health, HealthComp->MaxHealth);
            GEngine->AddOnScreenDebugMessage(-1, DamageInterval, FColor::Red, Msg);
        }

        if (HealthComp->IsDead())
        {
            StopDamage();
        }
    }
    else
    {
        StopDamage();
    }
}
void AEnemyCharacter::Die_Implementation()
{
    if (bAlreadyDied) return;      
    bAlreadyDied = true;

    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Red, TEXT("Enemigo murio"));
    }

    StopDamage();

    if (UCharacterMovementComponent* MoveComp = GetCharacterMovement())
    {
        MoveComp->DisableMovement();
    }

    if (AController* Con = GetController())
    {
        if (Con->IsA(AAIController::StaticClass()))
        {
            Con->UnPossess();
        }
    }

    float MontageLength = 0.f;
    UAnimInstance* AnimInst = (GetMesh() ? GetMesh()->GetAnimInstance() : nullptr);

    if (Death_A_Montage && AnimInst)
    {
        MontageLength = AnimInst->Montage_Play(Death_A_Montage, 0.8f);
    }

    if (DamageSphere) DamageSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    if (DetectionSphere) DetectionSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    if (GetMesh())
    {
        GetMesh()->SetCollisionResponseToAllChannels(ECR_Ignore);
    }

    const float FinalLife = (MontageLength > 0.f) ? MontageLength : 1.0f;
    SetLifeSpan(FinalLife);

    SpawnPotionDrop();
}

void AEnemyCharacter::InitializeFromSpawner(const TArray<ATargetPoint*>& InPatrolPoints, float InAcceptanceRadius, float InWaitTime)
{
    PatrolPoints = InPatrolPoints;
    PatrolAcceptanceRadius = InAcceptanceRadius;
    PatrolWaitTime = InWaitTime;
}

void AEnemyCharacter::OnHealthComponentDeath(AActor* DeadOwner)
{
    
    Die_Implementation();
    OnDeath.Broadcast(this);
}

void AEnemyCharacter::SpawnPotionDrop()
{
    if (!PotionDropClass) return;                         
    if (DropChance <= 0.0f) return;                       
    if (FMath::FRand() > DropChance) return;              


   
    FActorSpawnParameters SpawnParams;
    SpawnParams.Owner = this;
    SpawnParams.Instigator = GetInstigator();
    SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

    
    FVector SpawnLocation = GetActorLocation() + FVector(0.f, 0.f, -75.f); 
    FRotator SpawnRotation = FRotator::ZeroRotator;

    GetWorld()->SpawnActor<AActor>(PotionDropClass, SpawnLocation, SpawnRotation, SpawnParams);
}