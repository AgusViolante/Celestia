// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/EnemyBase.h"
#include "Components/HealthComponent.h"
#include "Components/StatsComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "AI/EnemyAIController.h"
#include "Kismet/GameplayStatics.h"
#include "Components/ProgressionComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraComponent.h"
#include "AIController.h"
#include "BrainComponent.h"
#include "Engine/Engine.h"

AEnemyBase::AEnemyBase()
{
	PrimaryActorTick.bCanEverTick = false;
	bUseControllerRotationYaw = false;

	if (UCharacterMovementComponent* MoveComp = GetCharacterMovement())
	{
		MoveComp->bOrientRotationToMovement = true;
		MoveComp->RotationRate = FRotator(0.f, 720.f, 0.f);
	}

	HealthComponent = CreateDefaultSubobject<UHealthComponent>(TEXT("HealthComponent"));
	StatsComponent = CreateDefaultSubobject<UStatsComponent>(TEXT("StatsComponent"));

	AIControllerClass = AEnemyAIController::StaticClass();
	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;

	EnemyType = EEnemyClassType::Melee;
}

void AEnemyBase::BeginPlay()
{
	Super::BeginPlay();

	if (UCapsuleComponent* Capsule = GetCapsuleComponent())
	{
		Capsule->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		Capsule->SetCollisionResponseToChannel(ECC_Pawn, ECR_Block);
		Capsule->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);
		
	}
	if (GetMesh())
	{
		GetMesh()->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);
	}

	if (HealthComponent)
	{
		HealthComponent->OnDeath.AddDynamic(this, &AEnemyBase::OnHealthComponentDeath);
	}

}

float AEnemyBase::TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser)
{
	
	float ActualDamage = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);


	if (bAlreadyDied || ActualDamage <= 0.0f)
	{
		return 0.0f;
	}
	if (AEnemyAIController* AICon = Cast<AEnemyAIController>(GetController()))
	{
		if (EventInstigator && EventInstigator->GetPawn())
		{
			AICon->ReceiveDamageAggro(EventInstigator->GetPawn());
		}

		else if (DamageCauser)
		{
			AICon->ReceiveDamageAggro(DamageCauser);
		}
	}
	if (HealthComponent)
	{
		HealthComponent->TakeDamage(ActualDamage);
	}

	return ActualDamage;
}

void AEnemyBase::OnHealthComponentDeath(AActor* DeadOwner)
{
	Die_Implementation();
	OnDeath.Broadcast(this);
}


void AEnemyBase::Die_Implementation()
{
	if (bAlreadyDied) return;
	bAlreadyDied = true;

	if (UCharacterMovementComponent* MoveComp = GetCharacterMovement())
	{
		MoveComp->DisableMovement();
	}

	if (AController* Con = GetController())
	{
		Con->UnPossess();
	}

	if (ACharacter* PlayerChar = UGameplayStatics::GetPlayerCharacter(GetWorld(), 0))
	{
		if (UProgressionComponent* PlayerProg = PlayerChar->FindComponentByClass<UProgressionComponent>())
		{
			float XPToGive = CalculateXPReward();
			PlayerProg->AddXP(XPToGive);

			//Mostrar en pantalla cuanta xp ganaste
			if (GEngine)
			{
				GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Cyan, FString::Printf(TEXT("+%.0f XP"), XPToGive));
			}
		}
	}

	float MontageLength = 1.0f;
	UAnimInstance* AnimInst = (GetMesh() ? GetMesh()->GetAnimInstance() : nullptr);

	if (Death_A_Montage && AnimInst)
	{
		MontageLength = AnimInst->Montage_Play(Death_A_Montage, 0.8f);
	}

	if (GetMesh())
	{
		GetMesh()->SetCollisionResponseToAllChannels(ECR_Ignore);
	}

	SetLifeSpan(MontageLength);
	SpawnPotionDrop();
}

void AEnemyBase::SpawnPotionDrop()
{
	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = this;
	SpawnParams.Instigator = GetInstigator();
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

	FVector SpawnLocation = GetActorLocation() + FVector(0.f, 0.f, -30.f);
	AActor* SpawnedPotion = GetWorld()->SpawnActor<AActor>(PotionDropClass, SpawnLocation, FRotator::ZeroRotator, SpawnParams);

}

float AEnemyBase::CalculateXPReward() const
{
	float TypeMultiplier = 1.0f;

	switch (EnemyType)
	{
	case EEnemyClassType::Melee:
		TypeMultiplier = 1.0f; // Multiplicador normal
		break;
	case EEnemyClassType::Ranged:
		TypeMultiplier = 1.2f; // Los de rango dan un 20% más por ser molestos
		break;
	case EEnemyClassType::Boss:
		TypeMultiplier = 5.0f; // Los jefes dan muchísima más XP
		break;
	case EEnemyClassType::Tank:
		TypeMultiplier = 1.5f; // Dan un 50% más de XP 
		break;

	}

	// Formula: XP Base * Nivel del Enemigo * Multiplicador de Tipo
	return BaseXPReward * EnemyLevel * TypeMultiplier;
}

void AEnemyBase::InitializeEnemyStats()
{
	if (!StatsComponent) return;

	float BaseStr = 1.0f, BaseDex = 1.0f, BaseInt = 1.0f, BaseWis = 1.0f, BaseEnd = 1.0f;


	float StrGrowth = 0.5f, DexGrowth = 0.5f, IntGrowth = 0.5f, WisGrowth = 0.5f, EndGrowth = 0.5f;

	switch (EnemyType)
	{
	case EEnemyClassType::Melee:
		
		BaseStr = 2.0f; BaseEnd = 4.0f; BaseDex = 1.0f;
		StrGrowth = 1.0f; EndGrowth = 1.5f; DexGrowth = 0.5f;
		break;
	case EEnemyClassType::Ranged:
		
		BaseDex = 3.0f; BaseInt = 2.0f; BaseEnd = 2.0f;
		DexGrowth = 1.0f; IntGrowth = 1.0f; EndGrowth = 0.5f;
		break;
	case EEnemyClassType::Boss:
		BaseStr = 6.0f; BaseEnd = 10.0f; BaseDex = 3.0f; BaseInt = 3.0f; BaseWis = 3.0f;
		StrGrowth = 2.0f; EndGrowth = 3.0f; DexGrowth = 1.0f; IntGrowth = 1.0f; WisGrowth = 1.0f;
		break;
	case EEnemyClassType::Tank:
	
		BaseStr = 3.0f; BaseEnd = 8.0f; BaseDex = 1.0f; BaseInt = 1.0f; BaseWis = 2.0f;
		StrGrowth = 1.5f; EndGrowth = 2.5f; DexGrowth = 0.2f; IntGrowth = 0.2f; WisGrowth = 0.5f;
		break;
	}

	int32 LevelUps = FMath::Max(0, EnemyLevel - 1);

	StatsComponent->SetPrimaryStats(
		BaseStr + (StrGrowth * LevelUps),
		BaseDex + (DexGrowth * LevelUps),
		BaseInt + (IntGrowth * LevelUps),
		BaseWis + (WisGrowth * LevelUps),
		BaseEnd + (EndGrowth * LevelUps)
	);

	if (HealthComponent)
	{
		float NewMaxHealth = StatsComponent->GetStatValue(ERPGStatType::MaxHealth);
		HealthComponent->MaxHealth = NewMaxHealth;
		HealthComponent->Health = NewMaxHealth;
	}
}
void AEnemyBase::ApplyStun_Implementation(float Duration)
{
	if (bIsStunned) return;
	bIsStunned = true;

	StopAnimMontage();

	if (GetCharacterMovement())
	{
		GetCharacterMovement()->DisableMovement();
		GetCharacterMovement()->StopMovementImmediately();
	}

	if (AAIController* AIC = Cast<AAIController>(GetController()))
	{
		if (AIC->GetBrainComponent())
		{
			AIC->GetBrainComponent()->PauseLogic("Stunned");
		}
	}

	if (GetMesh())
	{
		GetMesh()->bPauseAnims = true;
	}

	if (StunVFX)
	{
		ActiveStunVFX = UNiagaraFunctionLibrary::SpawnSystemAttached(
			StunVFX, GetMesh(), NAME_None,
			FVector(0.f, 0.f, StunVFXHeightOffset), FRotator::ZeroRotator,
			EAttachLocation::KeepRelativeOffset, true
		);
	}

	GetWorldTimerManager().SetTimer(StunTimerHandle, this, &AEnemyBase::ReleaseStun, Duration, false);
}

void AEnemyBase::ReleaseStun()
{
	if (bAlreadyDied) return;

	bIsStunned = false;

	
	if (GetCharacterMovement())
	{
		GetCharacterMovement()->SetMovementMode(MOVE_Walking);
	}

	if (AAIController* AIC = Cast<AAIController>(GetController()))
	{
		if (UBrainComponent* BrainComp = AIC->GetBrainComponent())
		{
			
			BrainComp->ResumeLogic("Stunned");
		}

		ACharacter* PlayerChar = UGameplayStatics::GetPlayerCharacter(GetWorld(), 0);
		if (PlayerChar)
		{
			
			if (UBlackboardComponent* BB = AIC->GetBlackboardComponent())
			{
				
				BB->ClearValue(TEXT("TargetActor"));

				
				BB->SetValueAsObject(TEXT("TargetActor"), PlayerChar);
			}

			
			if (AEnemyAIController* MyAIC = Cast<AEnemyAIController>(AIC))
			{
				MyAIC->ReceiveDamageAggro(PlayerChar);
			}
		}
	}

	if (GetMesh())
	{
		GetMesh()->bPauseAnims = false;
	}

	if (ActiveStunVFX)
	{
		ActiveStunVFX->DestroyComponent();
		ActiveStunVFX = nullptr;
	}
}