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
#include "Quests/QuestComponent.h"
#include "Engine/Engine.h"
#include "Net/UnrealNetwork.h"
#include "Engine/TargetPoint.h"
#include "EngineUtils.h"
#include "../../CelestiaCharacter.h" 

AEnemyBase::AEnemyBase()
{
	PrimaryActorTick.bCanEverTick = false;
	bUseControllerRotationYaw = false;
	bReplicates = true;

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

void AEnemyBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AEnemyBase, bAlreadyDied);
	DOREPLIFETIME(AEnemyBase, bIsStunned);
}

void AEnemyBase::BeginPlay()
{
	Super::BeginPlay();

	if (HasAuthority())
	{
		InitializeEnemyStats();
	}


	if (HealthComponent)
	{
		HealthComponent->OnDeath.AddUniqueDynamic(this, &AEnemyBase::OnHealthComponentDeath);
	}
}

AActor* AEnemyBase::GetAITarget() const
{
	if (AAIController* AIC = Cast<AAIController>(GetController()))
	{
		if (UBlackboardComponent* BB = AIC->GetBlackboardComponent())
		{
			return Cast<AActor>(BB->GetValueAsObject(FName("TargetActor")));
		}
	}
	return nullptr;
}

float AEnemyBase::TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser)
{
	float ActualDamage = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);

	if (!HasAuthority() || bAlreadyDied || ActualDamage <= 0.0f)
	{
		return 0.0f;
	}

	AActor* TrueAttacker = nullptr;
	if (EventInstigator && EventInstigator->GetPawn())
	{
		TrueAttacker = EventInstigator->GetPawn();
	}
	else if (DamageCauser)
	{
		if (DamageCauser->IsA(APawn::StaticClass()))
		{
			TrueAttacker = DamageCauser;
		}
		else if (APawn* InstPawn = DamageCauser->GetInstigator())
		{
			TrueAttacker = InstPawn;
		}
	}

	if (TrueAttacker)
	{
		LastAttacker = TrueAttacker;
		if (AEnemyAIController* AICon = Cast<AEnemyAIController>(GetController()))
		{
			AICon->ReceiveDamageAggro(TrueAttacker);
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
	if (HasAuthority())
	{
		Die_Implementation();
		OnDeath.Broadcast(this);
	}
}

void AEnemyBase::Die_Implementation()
{
	if (bAlreadyDied || !HasAuthority()) return;

	bAlreadyDied = true;
	OnRep_AlreadyDied();
	for (TActorIterator<ACelestiaCharacter> It(GetWorld()); It; ++It)
	{
		ACelestiaCharacter* Player = *It;
		if (Player)
		{
			if (UProgressionComponent* PlayerProg = Player->FindComponentByClass<UProgressionComponent>())
			{
				PlayerProg->AddXP(CalculateXPReward());
			}
			if (UQuestComponent* QuestComp = Player->FindComponentByClass<UQuestComponent>())
			{
				QuestComp->UpdateObjective(EObjectiveType::Kill, EnemyQuestID, nullptr, 1);
			}
		}
	}

	SpawnPotionDrop();
}

void AEnemyBase::OnRep_AlreadyDied()
{
	GetWorldTimerManager().ClearTimer(StunTimerHandle);

	if (UCharacterMovementComponent* MoveComp = GetCharacterMovement())
	{
		MoveComp->DisableMovement();
		MoveComp->StopMovementImmediately();
	}

	if (AController* Con = GetController())
	{
		Con->UnPossess();
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
	if (UCapsuleComponent* Capsule = GetCapsuleComponent())
	{
		Capsule->SetCollisionResponseToAllChannels(ECR_Ignore);
		Capsule->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Block);
		Capsule->SetCollisionResponseToChannel(ECC_WorldDynamic, ECR_Block);
	}

	SetLifeSpan(MontageLength + 1.0f);
}

void AEnemyBase::SpawnPotionDrop()
{
	if (!HasAuthority() || FMath::RandRange(0.0f, 1.0f) > DropChance) return;

	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = this;
	SpawnParams.Instigator = GetInstigator();
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

	FVector SpawnLocation = GetActorLocation() + FVector(0.f, 0.f, -30.f);
	GetWorld()->SpawnActor<AActor>(PotionDropClass, SpawnLocation, FRotator::ZeroRotator, SpawnParams);
}

float AEnemyBase::CalculateXPReward() const
{
	float TypeMultiplier = 1.0f;

	switch (EnemyType)
	{
	case EEnemyClassType::Melee: TypeMultiplier = 1.0f; break;
	case EEnemyClassType::Ranged: TypeMultiplier = 1.2f; break;
	case EEnemyClassType::Boss: TypeMultiplier = 5.0f; break;
	case EEnemyClassType::Tank: TypeMultiplier = 1.5f; break;
	}

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
		BaseStr = 10.0f; BaseEnd = 30.0f; BaseDex = 3.0f; BaseInt = 3.0f; BaseWis = 3.0f;
		StrGrowth = 2.0f; EndGrowth = 5.0f; DexGrowth = 1.0f; IntGrowth = 1.0f; WisGrowth = 1.0f;
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
		HealthComponent->UpdateMaxHealth(NewMaxHealth);
		HealthComponent->Heal(NewMaxHealth);
	}
}

void AEnemyBase::ApplyStun_Implementation(float Duration)
{
	if (bIsStunned || !HasAuthority()) return;
	bIsStunned = true;
	OnRep_IsStunned();

	if (AAIController* AIC = Cast<AAIController>(GetController()))
	{
		if (AIC->GetBrainComponent())
		{
			AIC->GetBrainComponent()->PauseLogic("Stunned");
		}
	}

	GetWorldTimerManager().SetTimer(StunTimerHandle, this, &AEnemyBase::ReleaseStun, Duration, false);
}

void AEnemyBase::OnRep_IsStunned()
{
	if (bIsStunned)
	{
		StopAnimMontage();

		if (GetCharacterMovement())
		{
			GetCharacterMovement()->DisableMovement();
			GetCharacterMovement()->StopMovementImmediately();
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
	}
	else
	{
		if (GetCharacterMovement())
		{
			GetCharacterMovement()->SetMovementMode(MOVE_Walking);
		}

		if (GetMesh())
		{
			GetMesh()->bPauseAnims = false;
		}

		if (IsValid(ActiveStunVFX))
		{
			ActiveStunVFX->DestroyComponent();
		}
		ActiveStunVFX = nullptr;
	}
}

void AEnemyBase::ReleaseStun()
{
	if (bAlreadyDied || !HasAuthority()) return;

	bIsStunned = false;
	OnRep_IsStunned();

	if (AAIController* AIC = Cast<AAIController>(GetController()))
	{
		if (UBrainComponent* BrainComp = AIC->GetBrainComponent())
		{
			BrainComp->ResumeLogic("Stunned");
		}

		if (LastAttacker)
		{
			if (UBlackboardComponent* BB = AIC->GetBlackboardComponent())
			{
				BB->SetValueAsObject(TEXT("TargetActor"), LastAttacker);
			}
		}
	}
}