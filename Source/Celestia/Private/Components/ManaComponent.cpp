// Fill out your copyright notice in the Description page of Project Settings.
#include "Components/ManaComponent.h"
#include "Components/ProgressionComponent.h"
#include "Math/UnrealMathUtility.h"

UManaComponent::UManaComponent()
{
	PrimaryComponentTick.bCanEverTick = false;

	MaxMana = 50.0f;
	CurrentMana = MaxMana;
}

void UManaComponent::BeginPlay()
{
	Super::BeginPlay();
	
}

float UManaComponent::CalculateDynamicCost(float BaseCost, float Multiplier) const
{
	float FinalCost = BaseCost;

	
	if (AActor* MyOwner = GetOwner())
	{
		if (UProgressionComponent* ProgComp = MyOwner->FindComponentByClass<UProgressionComponent>())
		{
			
			int32 PlayerLevel = ProgComp->CurrentLevel;
			FinalCost = BaseCost + (PlayerLevel * Multiplier);
		}
	}

	return FinalCost;
}

bool UManaComponent::TryConsumeDynamicMana(float BaseCost, float LevelMultiplier)
{
	
	float ActualCost = CalculateDynamicCost(BaseCost, LevelMultiplier);

	if (CurrentMana >= ActualCost)
	{
		CurrentMana -= ActualCost;
		OnManaChanged.Broadcast(CurrentMana, MaxMana);
		return true;
	}


	return false;
}
void UManaComponent::RestoreMana(float Amount)
{
	CurrentMana = FMath::Clamp(CurrentMana + Amount, 0.0f, MaxMana);
	OnManaChanged.Broadcast(CurrentMana, MaxMana);
}

void UManaComponent::UpdateMaxMana(float NewMaxMana)
{
	
	float ManaDifference = NewMaxMana - MaxMana;

	
	MaxMana = NewMaxMana;

	
	if (ManaDifference > 0.0f)
	{
		CurrentMana += ManaDifference;
	}

	CurrentMana = FMath::Clamp(CurrentMana, 0.0f, MaxMana);

	
	OnManaChanged.Broadcast(CurrentMana, MaxMana);
}

bool UManaComponent::TryRestoreMana(float AmountToRestore)
{
	
	if (FMath::IsNearlyEqual(CurrentMana, MaxMana))
	{
		if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Yellow, TEXT("Mana lleno. Pocion cancelada."));
		return false;
	}

	
	RestoreMana(AmountToRestore);
	return true;
}