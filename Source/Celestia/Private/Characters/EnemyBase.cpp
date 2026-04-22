// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/EnemyBase.h"
#include "Components/HealthComponent.h"
#include "Components/StatsComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "AI/EnemyAIController.h"
#include "Kismet/GameplayStatics.h"
#include "Components/ProgressionComponent.h"
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
	}

	// Formula: XP Base * Nivel del Enemigo * Multiplicador de Tipo
	return BaseXPReward * EnemyLevel * TypeMultiplier;
}
